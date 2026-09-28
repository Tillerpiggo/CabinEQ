#include "Engine.h"
#include "TestSignal.h"

namespace
{
constexpr float silenceDb = -70.0f;              // quieter than this counts as nothing
constexpr double stallSeconds = 1.5;             // no audio callbacks for this long means audio stopped
constexpr double blockedAfterSeconds = 4.0;      // another app playing but we hear silence for this long
constexpr double testSeconds = 1.5;

const juce::String keyOutput { "outputUID" };
const juce::String keyPlugin { "pluginPath" };
const juce::String keySeenSetup { "seenSetup" };
const juce::String keyImported { "importedStandaloneProfiles" };

double now() { return juce::Time::getMillisecondCounterHiRes() / 1000.0; }

float toDb (float gain) { return juce::Decibels::gainToDecibels (gain, -100.0f); }

juce::String deviceName (const std::vector<AudioDevices::OutputDevice>& devices, const juce::String& uid)
{
    for (const auto& device : devices)
        if (juce::String (device.uid) == uid)
            return device.name;
    return {};
}
}

juce::File Engine::getLogFile()
{
    return juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("Library/Logs/CabinEQ System.log");
}

Engine::Engine (bool startAudio) : audioAllowed (startAudio)
{
    if (audioAllowed)
        log = std::make_unique<juce::FileLogger> (getLogFile(), "CabinEQ System", 256 * 1024);

    juce::PropertiesFile::Options options;
    options.applicationName = "CabinEQ System";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";
    settings.setStorageParameters (options);

    auto& stored = *settings.getUserSettings();
    outputChoice = stored.getValue (keyOutput);
    pluginFile = stored.containsKey (keyPlugin) ? juce::File (stored.getValue (keyPlugin)) : getDefaultPluginFile();

    juce::addDefaultFormatsToManager (formats);
    devices = AudioDevices::getOutputDevices();
    conflictingApps = AudioDevices::getConflictingApps();
    deviceListener = std::make_unique<AudioDevices::ChangeListener> ([this] { devicesChanged(); });

    loadPlugin();
    permission = SystemAudioTap::checkPermission();

    // Someone who's been through setup before shouldn't have to click anything to get going
    if (permission == SystemAudioTap::Permission::granted)
        connect();
    else if (permission == SystemAudioTap::Permission::unknown && hasSeenSetup() && audioAllowed)
        askForPermission();

    updateStatus();
    startTimerHz (10);
}

Engine::~Engine()
{
    *alive = false;
    stopTimer();
    savePluginState();
    testPlayer.reset();
    disconnect();
    deviceListener.reset();
}

juce::File Engine::getDefaultPluginFile()
{
    return juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("Library/Audio/Plug-Ins/VST3/CabinEQ.vst3");
}

//==============================================================================
void Engine::loadPlugin()
{
    savePluginState(); // the plugin that's going away
    disconnect();
    if (prepared && plugin != nullptr)
        plugin->releaseResources();
    prepared = false;
    plugin.reset();
    pluginError = {};

    const auto path = pluginFile.getFullPathName();
    if (! pluginFile.exists())
    {
        pluginError = "There's no plugin at " + path;
        return;
    }

    for (auto* format : formats.getFormats())
    {
        juce::OwnedArray<juce::PluginDescription> found;
        if (format->fileMightContainThisPluginType (path))
            format->findAllTypesForFile (found, path);

        if (! found.isEmpty())
        {
            plugin = formats.createPluginInstance (*found[0], 48000.0, 512, pluginError);
            if (plugin != nullptr)
                restorePluginState();
            return;
        }
    }

    pluginError = "That isn't a VST3 or Audio Unit plugin";
}

//==============================================================================
// Hosts are what remember a plugin's settings, so CabinEQ System has to: your profiles live in here
juce::File Engine::getStateFile() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("Application Support/CabinEQ System")
               .getChildFile (pluginFile.getFileNameWithoutExtension() + " state.bin");
}

void Engine::restorePluginState()
{
    const auto file = getStateFile();
    juce::MemoryBlock state;

    auto& stored = *settings.getUserSettings();

    if (file.existsAsFile() && file.loadFileAsData (state) && state.getSize() > 0)
    {
        plugin->setStateInformation (state.getData(), (int) state.getSize());
        stored.setValue (keyImported, true); // it has settings of its own now
        if (log != nullptr)
            log->logMessage ("Restored " + plugin->getName() + "'s settings from " + file.getFullPathName());
    }
    else if (audioAllowed && ! stored.getBoolValue (keyImported, false) && importFromStandaloneApp())
    {
        // Only ever import once, so clearing the settings later doesn't bring the old profiles back
        stored.setValue (keyImported, true);
        savePluginState();
    }

    lastSavedState.reset();
    plugin->getStateInformation (lastSavedState);
}

bool Engine::importFromStandaloneApp()
{
    // The first time, bring over the profiles from the CabinEQ standalone app, which keeps its
    // plugin state in ~/Library/Application Support/CabinEQ.settings. CabinEQ migrates old ones.
    if (pluginFile.getFileNameWithoutExtension() != "CabinEQ" || plugin->getPluginDescription().pluginFormatName != "VST3")
        return false;

    const auto settingsFile = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                                  .getChildFile ("Application Support/CabinEQ.settings");
    if (! settingsFile.existsAsFile())
        return false;

    juce::PropertiesFile standaloneSettings (settingsFile, {});
    juce::MemoryBlock pluginState;
    if (! pluginState.fromBase64Encoding (standaloneSettings.getValue ("filterState")) || pluginState.getSize() == 0)
        return false;

    // A VST3 host hands the plugin's own state over as the component's state
    juce::XmlElement wrapper ("VST3PluginState");
    wrapper.createNewChildElement ("IComponent")->addTextElement (pluginState.toBase64Encoding());
    juce::MemoryBlock wrapped;
    juce::AudioProcessor::copyXmlToBinary (wrapper, wrapped);
    plugin->setStateInformation (wrapped.getData(), (int) wrapped.getSize());

    if (log != nullptr)
        log->logMessage ("Imported the CabinEQ app's profiles from " + settingsFile.getFullPathName());
    return true;
}

void Engine::savePluginState()
{
    if (plugin == nullptr || ! audioAllowed) // --snapshot only looks
        return;

    juce::MemoryBlock state;
    plugin->getStateInformation (state);
    if (state.getSize() == 0 || state == lastSavedState)
        return;

    const auto file = getStateFile();
    file.getParentDirectory().createDirectory();
    if (file.replaceWithData (state.getData(), state.getSize())) // writes a temporary file, then swaps it in
        lastSavedState = std::move (state);
}

void Engine::setPluginFile (const juce::File& file)
{
    pluginFile = file;
    settings.getUserSettings()->setValue (keyPlugin, file.getFullPathName());
    loadPlugin();
    connect();
    updateStatus();
}

//==============================================================================
void Engine::askForPermission()
{
    if (askingForPermission)
        return;

    askingForPermission = true;
    updateStatus();

    // The prompt blocks until it's answered, so wait for it off the main thread
    juce::Thread::launch ([this, alive = alive]
    {
        const auto result = SystemAudioTap::requestPermission();
        juce::MessageManager::callAsync ([this, alive, result]
        {
            if (! *alive)
                return;
            askingForPermission = false;
            permissionAsked = true;
            permission = result;
            connect();
            updateStatus();
        });
    });
}

//==============================================================================
bool Engine::canConnect() const
{
    // Without the permission the tap is silent while apps are muted, so you'd hear nothing at all.
    // "Unknown" after asking means macOS won't say, so try anyway and watch what comes in.
    return audioAllowed && plugin != nullptr && ! askingForPermission
        && (permission == SystemAudioTap::Permission::granted
            || (permission == SystemAudioTap::Permission::unknown && permissionAsked));
}

juce::String Engine::chooseOutput() const
{
    // A device you picked, if it's still there
    if (outputChoice.isNotEmpty() && deviceName (devices, outputChoice).isNotEmpty())
        return outputChoice;

    // Otherwise the Mac's output, unless it's one nobody can hear
    if (auto mac = getMacOutput(); mac.has_value() && ! mac->isVirtual)
        return mac->uid;

    return AudioDevices::pickRealOutput (devices);
}

void Engine::connect()
{
    disconnect();
    connectError = {};

    if (plugin == nullptr)
        return;

    if (! canConnect())
        return;

    const auto target = chooseOutput();
    if (target.isEmpty())
    {
        connectError = "There are no speakers or headphones to play to";
        return;
    }

    if (log != nullptr)
    {
        auto mac = getMacOutput();
        log->logMessage ("Connecting to " + deviceName (devices, target) + " (Mac output: "
                         + (mac.has_value() ? juce::String (mac->name) : juce::String ("none")) + ")");
    }

    std::string error;
    if (! tap.open (error, target.toStdString()))
    {
        connectError = error;
        tap.close();
        return;
    }

    const double sampleRate = tap.getSampleRate();
    const int newBlockSize = juce::jmax (32, tap.getBufferSize());

    if (! prepared || sampleRate != preparedSampleRate || newBlockSize != blockSize)
    {
        if (prepared)
            plugin->releaseResources();
        blockSize = newBlockSize;
        plugin->enableAllBuses();
        plugin->prepareToPlay (sampleRate, blockSize);
        prepared = true;
        preparedSampleRate = sampleRate;
    }

    work.setSize (juce::jmax (2, plugin->getTotalNumInputChannels(), plugin->getTotalNumOutputChannels()), blockSize);
    callbackBudgetSeconds = (double) tap.getBufferSize() / sampleRate;

    if (! tap.start ([this] (const float* const* in, float* const* out, int n) { process (in, out, n); }, error))
    {
        connectError = error;
        tap.close();
        return;
    }

    lastCallbackTime = lastReconnectTime = now();
    if (log != nullptr)
        log->logMessage (juce::String (tap.getDescription()).replace ("\n", "; "));
}

void Engine::disconnect()
{
    tap.close();
}

// Core Audio's real-time thread: no allocating, logging or waiting
void Engine::process (const float* const* in, float* const* out, int numFrames)
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

        {
            const juce::ScopedLock lock (plugin->getCallbackLock());
            if (plugin->isSuspended())
                block.clear();
            else
                plugin->processBlock (block, midi);
        }
        midi.clear();

        for (int ch = 0; ch < 2; ++ch)
            juce::FloatVectorOperations::copy (out[ch] + offset, block.getReadPointer (ch), n);

        offset += n;
    }

    if (juce::Time::highResolutionTicksToSeconds (juce::Time::getHighResolutionTicks() - startTicks) > 0.8 * callbackBudgetSeconds)
        slowCallbacks.fetch_add (1);
}

//==============================================================================
void Engine::setOutputChoice (const juce::String& uid)
{
    outputChoice = uid;
    settings.getUserSettings()->setValue (keyOutput, uid);
    connect();
    updateStatus();
}

juce::String Engine::getPlayingToName() const
{
    return tap.isRunning() ? juce::String (tap.getOutputName()) : juce::String();
}

juce::String Engine::getPlayingToUID() const
{
    return tap.isRunning() ? juce::String (tap.getOutputUID()) : juce::String();
}

std::optional<AudioDevices::OutputDevice> Engine::getMacOutput() const
{
    const auto uid = AudioDevices::getDefaultOutputUID();
    for (const auto& device : devices)
        if (device.uid == uid)
            return device;
    return std::nullopt;
}

void Engine::makeMacOutput (const juce::String& uid)
{
    AudioDevices::setDefaultOutput (uid.toStdString());
    devicesChanged();
}

void Engine::quitConflictingApp (int pid)
{
    AudioDevices::quitApp (pid);
}

void Engine::devicesChanged()
{
    devices = AudioDevices::getOutputDevices();

    // Follow the Mac's output (or fall back) when it changes, or if our device went away
    if (canConnect() && (! tap.isRunning() || chooseOutput() != getPlayingToUID()))
        connect();

    updateStatus();
}

//==============================================================================
void Engine::timerCallback()
{
    const double t = now();
    const auto activity = tap.takeActivity();

    if (activity.callbacks > 0)
        lastCallbackTime = t;
    if (toDb (activity.inPeak) > silenceDb)
        lastInputTime = t;
    if (toDb (activity.outPeak) > silenceDb)
        lastOutputTime = t;

    // Meters that jump up and fall back gently
    inputLevel = std::max (toDb (activity.inPeak), inputLevel - 3.0f);
    outputLevel = std::max (toDb (activity.outPeak), outputLevel - 3.0f);

    if (testState == TestState::playing)
    {
        testInputPeak = std::max (testInputPeak, activity.inPeak);
        testOutputPeak = std::max (testOutputPeak, activity.outPeak);
        if ((testPlayer == nullptr || ! testPlayer->isRunning()) && t - testStartedAt > testSeconds + 0.3)
            finishTest();
    }

    if (slowCallbacks.exchange (0) > 0)
        lastDropoutTime = t;

    // Save any changes to the plugin's settings every couple of seconds
    if (t - lastStateCheck > 2.0)
    {
        lastStateCheck = t;
        savePluginState();
    }

    // Slower checks, twice a second
    if (t - lastPoll > 0.5)
    {
        lastPoll = t;

        const bool playing = AudioDevices::isAnotherProcessPlaying();
        if (playing && ! otherAppPlaying)
            otherAppPlayingSince = t;
        otherAppPlaying = playing;

        conflictingApps = AudioDevices::getConflictingApps();

        // Notice when the permission is turned on in System Settings
        if (permission != SystemAudioTap::Permission::granted && ! askingForPermission)
        {
            const auto current = SystemAudioTap::checkPermission();
            if (current != permission)
            {
                permission = current;
                if (permission == SystemAudioTap::Permission::granted)
                    connect();
            }
        }

        // Audio stopped (a device went away, or Core Audio restarted): start again
        if (tap.isRunning() && t - lastCallbackTime > stallSeconds && t - lastReconnectTime > 3.0)
            connect();
        else if (! tap.isRunning() && canConnect() && t - lastReconnectTime > 3.0)
        {
            lastReconnectTime = t;
            connect();
        }
    }

    updateStatus();
}

void Engine::updateStatus()
{
    using Level = Status::Level;
    const double t = now();
    Status s;

    const bool hearing = t - lastInputTime < 1.0;
    const bool sending = t - lastOutputTime < 1.0;
    const auto playingTo = getPlayingToName();
    const auto mac = getMacOutput();

    if (plugin == nullptr)
        s = { Level::problem, "CabinEQ isn't loaded", pluginError.isNotEmpty() ? pluginError : "Choose the plugin in Setup" };
    else if (askingForPermission)
        s = { Level::busy, "Waiting for permission", "Allow CabinEQ System to record system audio in the macOS prompt" };
    else if (permission == SystemAudioTap::Permission::denied)
        s = { Level::problem, "macOS isn't letting CabinEQ hear your audio",
              "Turn on CabinEQ System in System Settings > Privacy & Security > Screen & System Audio Recording" };
    else if (! canConnect())
        s = { Level::warning, "One more step", "CabinEQ System needs your permission to hear your Mac's audio. Open Setup to give it" };
    else if (! tap.isRunning())
        s = { Level::problem, "Not running", connectError.isNotEmpty() ? connectError : "Starting..." };
    else if (t - lastCallbackTime > stallSeconds)
        s = { Level::busy, "Reconnecting", "The audio stopped, so CabinEQ is starting it again" };
    else if (otherAppPlaying && ! hearing && t - otherAppPlayingSince > blockedAfterSeconds
             && permission != SystemAudioTap::Permission::granted)
        s = { Level::problem, "Something's playing, but CabinEQ hears silence",
              "macOS is probably blocking it. Check Screen & System Audio Recording in Privacy & Security" };
    else if (hearing && ! sending)
        s = { Level::problem, "Audio reaches CabinEQ, but nothing comes out", "Check the EQ's preamp, and that " + playingTo + " is connected" };
    else if (! conflictingApps.empty())
        s = { Level::warning, juce::String (conflictingApps.front().name) + " is also processing your audio",
              "You'd hear both at once. Quit it in Setup" };
    else if (t - lastDropoutTime < 5.0)
        s = { Level::warning, "Your Mac is struggling to keep up", "You may hear dropouts. Closing other audio apps can help" };
    else if (mac.has_value() && mac->isVirtual)
        s = { hearing ? Level::working : Level::idle, hearing ? "On" : "Ready",
              "Playing to " + playingTo + ". Your Mac's output is " + juce::String (mac->name) + ", so the volume keys won't work. Fix it in Setup" };
    else if (hearing)
        s = { Level::working, "On", "EQing everything your Mac plays, through " + playingTo };
    else
        s = { Level::idle, "Ready", "Nothing's playing. Anything you play will go through CabinEQ to " + playingTo };

    if (s != status)
    {
        status = s;
        if (log != nullptr)
            log->logMessage (status.headline + ": " + status.detail);
    }

    if (onChange)
        onChange();
}

//==============================================================================
void Engine::playTestSound()
{
    if (! tap.isRunning() || testState == TestState::playing)
        return;

    auto file = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("CabinEQ System test sound.wav");
    if (! TestSignal::writePinkNoise (file, testSeconds, -26.0f))
    {
        testState = TestState::failed;
        testResult = "Couldn't make the test sound";
        return;
    }

    // Played by another app (afplay), just like music would be
    testPlayer = std::make_unique<juce::ChildProcess>();
    if (! testPlayer->start (juce::StringArray { "/usr/bin/afplay", file.getFullPathName() }, 0))
    {
        testState = TestState::failed;
        testResult = "Couldn't play the test sound";
        return;
    }

    testState = TestState::playing;
    testResult = "Playing a test sound...";
    testStartedAt = now();
    testInputPeak = testOutputPeak = 0;
    updateStatus();
}

void Engine::finishTest()
{
    testPlayer.reset();
    const bool heard = toDb (testInputPeak) > -55.0f;
    const bool sent = toDb (testOutputPeak) > -60.0f;

    if (heard && sent)
    {
        testState = TestState::passed;
        testResult = "It went through CabinEQ to " + getPlayingToName() + ". If you heard one burst of noise, not two, everything's working.";
    }
    else if (! heard)
    {
        testState = TestState::failed;
        testResult = permission == SystemAudioTap::Permission::granted
            ? "CabinEQ didn't hear the test sound. Try quitting and reopening CabinEQ System."
            : "CabinEQ didn't hear the test sound, so macOS is probably blocking it. Check the permission above.";
    }
    else
    {
        testState = TestState::failed;
        testResult = "CabinEQ heard the test sound but sent nothing out. Check the EQ isn't turned all the way down.";
    }
    updateStatus();
}

bool Engine::hasSeenSetup() const
{
    return settings.getUserSettings()->getBoolValue (keySeenSetup, false);
}

void Engine::setHasSeenSetup()
{
    settings.getUserSettings()->setValue (keySeenSetup, true);
    settings.saveIfNeeded();
}
