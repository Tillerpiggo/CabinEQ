#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "AppView.h"
#include "Engine.h"
#include "SystemAudioTap.h"
#include "TestSignal.h"

#include <iostream>

// CabinEQ System: routes all system audio through CabinEQ (or another plugin), using a Core Audio process tap.
//
//   (default)   opens a window with the plugin's editor, a status bar that says whether it's
//               working, and a setup checklist; closing the window stops it
//   --test      automated test: plays pink noise from another process (afplay) and checks
//               that it went tap -> plugin -> speakers, then writes report.txt, dry.wav, wet.wav
//   --plugin P  the .vst3 or .component to load (default: CabinEQ.vst3)
//   --out D     where to write the report and recordings

namespace
{
constexpr double stimulusSeconds = 4.0;
constexpr double stimulusSampleRate = 48000.0;
constexpr int fftOrder = 12;

struct Options
{
    juce::File pluginFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory)
                                .getChildFile ("Library/Audio/Plug-Ins/VST3/CabinEQ.vst3");
    juce::File outputDir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("CabinEQ System");
    bool test = false;

    static Options parse (const juce::StringArray& args)
    {
        Options options;

        for (int i = 0; i < args.size(); ++i)
        {
            if (args[i] == "--plugin" && i + 1 < args.size())
                options.pluginFile = juce::File (args[++i]);
            else if (args[i] == "--out" && i + 1 < args.size())
                options.outputDir = juce::File (args[++i]);
            else if (args[i] == "--test")
                options.test = true;
        }

        return options;
    }
};

class Report
{
public:
    bool check (bool ok, const juce::String& what, const juce::String& whyNot = {})
    {
        const auto line = ok || whyNot.isEmpty() ? what : what + ": " + whyNot;
        add (ok ? "PASS" : "FAIL", line);

        if (ok)
            ++passed;
        else
            failures.add (line);

        return ok;
    }

    void warn (const juce::String& line) { add ("WARN", line); }
    void info (const juce::String& line) { add ("INFO", line); }

    void addResult()
    {
        const int failed = failures.size();
        const auto result = failed == 0 ? "RESULT: PASS (" + juce::String (passed) + " checks passed)"
                                        : "RESULT: FAIL (" + juce::String (failed) + " of " + juce::String (passed + failed) + " checks failed)";
        text << result << "\n";
        std::cout << result << std::endl;
    }

    juce::String text;
    juce::StringArray failures;
    int passed = 0;

private:
    void add (const juce::String& tag, const juce::String& line)
    {
        const auto tagged = "[" + tag + "] " + line;
        text << tagged << "\n";
        std::cout << tagged << std::endl;
    }
};

//==============================================================================
juce::String formatDb (float db)
{
    return db <= -150.0f ? juce::String ("silent") : juce::String (db, 1) + " dBFS";
}

float rmsDb (const juce::AudioBuffer<float>& buffer, int start, int numSamples)
{
    if (numSamples <= 0)
        return -200.0f;

    double sum = 0.0;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const auto rms = buffer.getRMSLevel (ch, start, numSamples);
        sum += rms * rms;
    }

    return juce::Decibels::gainToDecibels ((float) std::sqrt (sum / buffer.getNumChannels()), -200.0f);
}

int findOnset (const juce::AudioBuffer<float>& buffer, int from, int to, float threshold)
{
    for (int i = from; i < to; ++i)
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            if (std::abs (buffer.getSample (ch, i)) > threshold)
                return i;

    return -1;
}

bool isAllFinite (const juce::AudioBuffer<float>& buffer, int numSamples)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < numSamples; ++i)
            if (! std::isfinite (buffer.getSample (ch, i)))
                return false;

    return true;
}

// Welch-averaged power spectrum of the mono sum, so dry vs wet shows what the plugin did per band.
std::vector<double> averagePowerSpectrum (const juce::AudioBuffer<float>& buffer, int start, int end)
{
    constexpr int size = 1 << fftOrder;
    juce::dsp::FFT fft (fftOrder);
    juce::dsp::WindowingFunction<float> window (size, juce::dsp::WindowingFunction<float>::hann, false);
    std::vector<float> frame (2 * size);
    std::vector<double> power (size / 2 + 1, 0.0);

    for (int pos = start; pos + size <= end; pos += size / 2)
    {
        for (int i = 0; i < size; ++i)
            frame[(size_t) i] = 0.5f * (buffer.getSample (0, pos + i) + buffer.getSample (1, pos + i));

        std::fill (frame.begin() + size, frame.end(), 0.0f);
        window.multiplyWithWindowingTable (frame.data(), size);
        fft.performFrequencyOnlyForwardTransform (frame.data(), true);

        for (size_t k = 0; k < power.size(); ++k)
            power[k] += (double) frame[k] * frame[k];
    }

    return power;
}

// Plugins expect to be created, prepared and released on the message thread.
void onMessageThread (std::function<void()> fn)
{
    juce::WaitableEvent done;
    juce::MessageManager::callAsync ([&] { fn(); done.signal(); });
    done.wait();
}

//==============================================================================
class Session : private juce::Thread
{
public:
    explicit Session (Options optionsToUse) : juce::Thread ("CabinEQ System"), options (std::move (optionsToUse))
    {
        juce::addDefaultFormatsToManager (formats);
        startThread();
    }

    ~Session() override
    {
        stopThread (10000);
        tap.close();

        if (prepared)
            plugin->releaseResources();
    }

private:
    void run() override
    {
        options.outputDir.createDirectory();
        const bool ready = setUp();

        if (ready)
            measure();

        tap.close();

        if (prepared)
        {
            onMessageThread ([this] { plugin->releaseResources(); });
            prepared = false;
        }

        report.addResult();
        saveReport();

        juce::MessageManager::callAsync ([failures = report.failures, test = options.test]
        {
            juce::JUCEApplication::getInstance()->setApplicationReturnValue (failures.isEmpty() ? 0 : 1);

            if (test || failures.isEmpty())
            {
                juce::JUCEApplication::quit();
                return;
            }

            // Opened by double-click, there's no terminal to read, so say what went wrong.
            juce::Process::makeForegroundProcess();
            juce::NativeMessageBox::showAsync (juce::MessageBoxOptions()
                                                   .withIconType (juce::MessageBoxIconType::WarningIcon)
                                                   .withTitle ("Couldn't start")
                                                   .withMessage (failures.joinIntoString ("\n\n"))
                                                   .withButton ("Quit"),
                                               [] (int) { juce::JUCEApplication::quit(); });
        });
    }

    bool setUp()
    {
        report.info ("Plugin file: " + options.pluginFile.getFullPathName());

        juce::String loadError;
        onMessageThread ([&] { plugin = loadPlugin (loadError); });

        if (! report.check (plugin != nullptr, "Loaded plugin" + (plugin != nullptr ? " \"" + plugin->getName() + "\"" : juce::String()), loadError))
            return false;

        switch (SystemAudioTap::requestPermission())
        {
            case SystemAudioTap::Permission::granted:
                report.check (true, "System audio recording permission granted");
                break;

            case SystemAudioTap::Permission::denied:
                report.check (false, "System audio recording permission",
                              "denied. Turn CabinEQ System on in System Settings > Privacy & Security > Screen & System Audio Recording, then rerun");
                return false;

            case SystemAudioTap::Permission::unknown:
                report.warn ("Couldn't check the system audio recording permission. If the capture below is silent, that's the likely cause");
                break;
        }

        std::string error;

        if (! report.check (tap.open (error), "Created a tap of all other apps' audio and a private aggregate device", error))
            return false;

        for (const auto& line : juce::StringArray::fromLines (tap.getDescription()))
            report.info (line);

        sampleRate = tap.getSampleRate();
        blockSize = juce::jmax (32, tap.getBufferSize());

        if (! report.check (sampleRate > 0, "Aggregate device reports a sample rate"))
            return false;

        onMessageThread ([this]
        {
            plugin->enableAllBuses();
            plugin->prepareToPlay (sampleRate, blockSize);
        });
        prepared = true;
        report.info ("Plugin latency: " + juce::String (plugin->getLatencySamples()) + " samples");

        work.setSize (juce::jmax (2, plugin->getTotalNumInputChannels(), plugin->getTotalNumOutputChannels()), blockSize);
        dry.setSize (2, (int) (sampleRate * 30.0));
        wet.setSize (2, dry.getNumSamples());
        dry.clear();
        wet.clear();

        return report.check (tap.start ([this] (const float* const* in, float* const* out, int n) { process (in, out, n); }, error),
                             "Started audio: tap -> plugin -> speakers", error);
    }

    std::unique_ptr<juce::AudioPluginInstance> loadPlugin (juce::String& error)
    {
        const auto path = options.pluginFile.getFullPathName();

        for (auto* format : formats.getFormats())
        {
            juce::OwnedArray<juce::PluginDescription> found;

            if (format->fileMightContainThisPluginType (path))
                format->findAllTypesForFile (found, path);

            if (! found.isEmpty())
                return formats.createPluginInstance (*found[0], 48000.0, 512, error);
        }

        error = "no VST3 or AU plugin found there";
        return nullptr;
    }

    // Runs on Core Audio's real-time thread, so no allocating, logging or waiting in here.
    void process (const float* const* in, float* const* out, int numFrames)
    {
        const auto startTicks = juce::Time::getHighResolutionTicks();

        for (int offset = 0; offset < numFrames;)
        {
            const int n = juce::jmin (blockSize, numFrames - offset);
            juce::AudioBuffer<float> block (work.getArrayOfWritePointers(), work.getNumChannels(), n);

            for (int ch = 0; ch < block.getNumChannels(); ++ch)
            {
                if (ch < 2)
                    block.copyFrom (ch, 0, in[ch] + offset, n);
                else
                    block.clear (ch, 0, n);
            }

            const int pos = recordPos.load();
            const bool record = recording.load() && pos + n <= dry.getNumSamples();

            if (record)
                for (int ch = 0; ch < 2; ++ch)
                    dry.copyFrom (ch, pos, block, ch, 0, n);

            {
                const juce::ScopedLock lock (plugin->getCallbackLock());

                if (plugin->isSuspended())
                    block.clear();
                else
                    plugin->processBlock (block, midi);
            }

            midi.clear();

            for (int ch = 0; ch < 2; ++ch)
            {
                juce::FloatVectorOperations::copy (out[ch] + offset, block.getReadPointer (ch), n);

                if (record)
                    wet.copyFrom (ch, pos, block, ch, 0, n);
            }

            if (record)
                recordPos.store (pos + n);

            offset += n;
        }

        // Skip the first few callbacks, which include one-off warm-up costs.
        if (callbacks.fetch_add (1) >= 10)
        {
            const auto ticks = juce::Time::getHighResolutionTicks() - startTicks;
            auto worst = worstTicks.load();

            while (ticks > worst && ! worstTicks.compare_exchange_weak (worst, ticks)) {}
        }
    }

    void measure()
    {
        const auto stimulus = options.outputDir.getChildFile ("stimulus.wav");

        if (! report.check (TestSignal::writePinkNoise (stimulus, stimulusSeconds, -20.0f, stimulusSampleRate), "Wrote the test signal (4 s of pink noise at -20 dBFS)"))
            return;

        // Record a little silence, the noise played by another process, then a tail.
        recording = true;
        juce::Thread::sleep (700);
        const int launchedAt = recordPos;

        juce::ChildProcess afplay;

        if (! report.check (afplay.start (juce::StringArray { "/usr/bin/afplay", stimulus.getFullPathName() }, 0), "Started afplay"))
            return;

        afplay.waitForProcessToFinish (20000);
        const int stoppedAt = recordPos;
        juce::Thread::sleep (1200);
        recording = false;
        const int recorded = recordPos;
        tap.stop();

        TestSignal::writeWav (options.outputDir.getChildFile ("dry.wav"), dry, sampleRate, recorded);
        TestSignal::writeWav (options.outputDir.getChildFile ("wet.wav"), wet, sampleRate, recorded);
        report.info ("Recordings: dry.wav (what the tap heard) and wet.wav (what the plugin sent to the speakers)");

        analyse (launchedAt, stoppedAt, recorded);
    }

    void analyse (int launchedAt, int stoppedAt, int recorded)
    {
        report.check (callbacks > 0, "Audio callbacks ran (" + juce::String (callbacks.load()) + ")");

        const int onset = findOnset (dry, launchedAt, stoppedAt, juce::Decibels::decibelsToGain (-50.0f));

        if (! report.check (onset >= 0, "Tap captured audio from another process (afplay)",
                            "the tap only delivered silence, which usually means the permission wasn't granted"))
            return;

        const int margin = (int) (0.25 * sampleRate);
        const int start = onset + margin;
        const int end = juce::jmin (stoppedAt, onset + (int) (stimulusSeconds * sampleRate)) - margin;

        if (! report.check (end - start >= (int) sampleRate, "Captured at least a second of the test signal"))
            return;

        const auto dryDb = rmsDb (dry, start, end - start);
        const auto wetDb = rmsDb (wet, start, end - start);
        report.info ("Level into plugin: " + formatDb (dryDb) + ", out of plugin: " + formatDb (wetDb));

        const bool finite = isAllFinite (wet, recorded);
        report.check (finite && wetDb > -80.0f, "Plugin produced audio",
                      finite ? "its output was silent" : "its output contains NaN/inf");

        const auto dryPower = averagePowerSpectrum (dry, start, end);
        const auto wetPower = averagePowerSpectrum (wet, start, end);
        const double binWidth = sampleRate / (1 << fftOrder);
        double biggestChange = 0.0;
        report.info ("Plugin's effect on the noise, by octave band:");

        for (int centre : { 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000 })
        {
            double dryBand = 0.0, wetBand = 0.0;

            for (size_t k = 1; k < dryPower.size(); ++k)
            {
                const double frequency = (double) k * binWidth;

                if (frequency >= centre / std::sqrt (2.0) && frequency < centre * std::sqrt (2.0))
                {
                    dryBand += dryPower[k];
                    wetBand += wetPower[k];
                }
            }

            const double gain = 10.0 * std::log10 ((wetBand + 1e-20) / (dryBand + 1e-20));
            biggestChange = juce::jmax (biggestChange, std::abs (gain));
            report.info (juce::String::formatted ("  %5d Hz  %+6.1f dB", centre, gain));
        }

        if (biggestChange < 0.5)
            report.info ("The plugin passed the audio through unchanged (flat curve, or its EQ is off)");
        else
            report.info ("The plugin changed the audio, by up to " + juce::String (biggestChange, 1) + " dB in a band");

        // If our own output were being tapped too, the tap would keep hearing it after afplay stopped.
        const int tailStart = stoppedAt + (int) (0.4 * sampleRate);
        const auto tailDb = rmsDb (dry, tailStart, recorded - tailStart);
        const auto baselineDb = rmsDb (dry, 0, launchedAt);

        if (tailDb < -70.0f)
            report.check (true, "No feedback: the tap went quiet when afplay stopped, so it isn't re-capturing our own output");
        else if (baselineDb > -70.0f)
            report.warn ("Another app was playing during the test (" + formatDb (baselineDb) + " before the noise), so the feedback check is inconclusive");
        else
            report.check (false, "No feedback", "the tap still heard " + formatDb (tailDb) + " after afplay stopped");

        const double budgetMs = 1000.0 * tap.getBufferSize() / sampleRate;
        const double worstMs = 1000.0 * juce::Time::highResolutionTicksToSeconds (worstTicks.load());
        report.check (worstMs < 0.8 * budgetMs,
                      juce::String::formatted ("Real-time headroom: slowest callback took %.2f ms of its %.2f ms budget", worstMs, budgetMs),
                      "too close to the budget, expect dropouts");

        report.info ("Muting of the original audio can't be measured from in here. By ear: you should have heard the noise once, not doubled.");
    }

    void saveReport()
    {
        options.outputDir.getChildFile ("report.txt").replaceWithText (report.text);
    }

    Options options;
    Report report;
    SystemAudioTap tap;

    juce::AudioPluginFormatManager formats;   // declared before `plugin` so it outlives it
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    bool prepared = false;

    double sampleRate = 0.0;
    int blockSize = 512;
    juce::AudioBuffer<float> work, dry, wet;
    juce::MidiBuffer midi;

    std::atomic<bool> recording { false };
    std::atomic<int> recordPos { 0 };
    std::atomic<int> callbacks { 0 };
    std::atomic<juce::int64> worstTicks { 0 };
};
}

//==============================================================================
class CabinEQSystemApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "CabinEQ System"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override           { return getCommandLineParameters().contains ("--snapshot"); }

    void initialise (const juce::String&) override
    {
        const auto args = getCommandLineParameterArray();
        if (const int index = args.indexOf ("--snapshot"); index >= 0 && index + 1 < args.size())
        {
            // Renders the window to a PNG without touching the audio: --snapshot out.png [setup]
            Engine looker (false);
            AppView view (looker);
            view.showSetup (args.contains ("setup"));
            juce::MessageManager::getInstance()->runDispatchLoopUntil (200);
            auto image = view.createComponentSnapshot (view.getLocalBounds(), true, 2.0f);
            juce::File file (args[index + 1]);
            file.deleteFile();
            juce::FileOutputStream stream (file);
            juce::PNGImageFormat().writeImageToStream (image, stream);
            setApplicationReturnValue (0);
            quit();
            return;
        }

        const auto options = Options::parse (args);
        if (options.test)
        {
            session = std::make_unique<Session> (options);
            return;
        }

        engine = std::make_unique<Engine>();
        window = std::make_unique<MainWindow> (*engine);
    }

    void shutdown() override
    {
        window = nullptr;
        engine = nullptr;
        session = nullptr;
    }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (Engine& engine)
            : DocumentWindow ("CabinEQ System", juce::Colour (0xff0e0f12), closeButton)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new AppView (engine), true);
            setResizable (true, false);
            setResizeLimits (720, 460 + AppView::statusBarHeight, 4000, 3000);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
            juce::Process::makeForegroundProcess();
        }

        void closeButtonPressed() override { juce::JUCEApplication::quit(); }
    };

    std::unique_ptr<Session> session;     // the automated test
    std::unique_ptr<Engine> engine;       // normal use
    std::unique_ptr<MainWindow> window;
};

START_JUCE_APPLICATION (CabinEQSystemApplication)
