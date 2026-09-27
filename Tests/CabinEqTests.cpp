/*
  ==============================================================================

    CabinEqTests.cpp

    Headless tests for the DSP and the saved state. Build and run with:
        cmake --build build --target CabinEQ_Tests
        build/CabinEQ_Tests_artefacts/Release/CabinEQ_Tests

  ==============================================================================
*/

#include <JuceHeader.h>
#include "../Source/CabinEqAudioProcessor.h"
#include "../Source/FilterChain.h"
#include "../Source/BandEqCurve.h"
#include "../Source/EqPresetFile.h"
#include "../Source/CabinPeqGraph.h"

namespace
{
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 256;

    /// Runs a sine through the chain until it settles, and returns the gain it measured in dB.
    float measureGainDb (FilterChain& chain, float frequency, int channel = 0)
    {
        juce::AudioBuffer<float> buffer (2, blockSize);
        double phase = 0.0;
        const double increment = juce::MathConstants<double>::twoPi * frequency / sampleRate;
        const int settleBlocks = (int) (sampleRate * 0.5 / blockSize) + 8 + (int) (sampleRate / frequency / blockSize);
        const int measureBlocks = std::max (8, (int) (sampleRate * 0.1 / blockSize));

        double inPower = 0.0, outPower = 0.0;
        for (int block = 0; block < settleBlocks + measureBlocks; ++block)
        {
            for (int i = 0; i < blockSize; ++i)
            {
                auto sample = (float) std::sin (phase);
                phase += increment;
                buffer.setSample (0, i, sample);
                buffer.setSample (1, i, sample);
                if (block >= settleBlocks)
                    inPower += sample * sample;
            }

            juce::dsp::AudioBlock<float> audioBlock (buffer);
            chain.process (audioBlock);

            if (block >= settleBlocks)
                for (int i = 0; i < blockSize; ++i)
                    outPower += buffer.getSample (channel, i) * buffer.getSample (channel, i);
        }
        return (float) (10.0 * std::log10 (outPower / inPower));
    }

    juce::MemoryBlock stateOf (juce::AudioProcessor& processor)
    {
        juce::MemoryBlock block;
        processor.getStateInformation (block);
        return block;
    }

    void pumpMessages()
    {
        // Lets the processor's AsyncUpdater run, which works out auto gain
        juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
    }
}

//==============================================================================
class FilterResponseTests : public juce::UnitTest
{
public:
    FilterResponseTests() : juce::UnitTest ("Filters match the drawn curve", "CabinEQ") {}

    void runTest() override
    {
        const std::vector<Band> cases {
            Band::withQ (0, 1000.0f, 6.0f, 1.41f, Band::Type::both, Band::Shape::peak),
            Band::withQ (0, 120.0f, -9.0f, 4.0f, Band::Type::both, Band::Shape::peak),
            Band::withQ (0, 200.0f, 5.0f, 0.71f, Band::Type::both, Band::Shape::lowShelf),
            Band::withQ (0, 6000.0f, -4.0f, 0.71f, Band::Type::both, Band::Shape::highShelf),
            Band::withQ (0, 80.0f, 0.0f, 0.71f, Band::Type::both, Band::Shape::lowCut),
            Band::withQ (0, 9000.0f, 0.0f, 0.71f, Band::Type::both, Band::Shape::highCut),
        };

        for (const auto& band : cases)
        {
            beginTest (Band::shapeName (band.shape) + " at " + juce::String (band.freq) + " Hz");

            FilterChain chain;
            chain.prepare (sampleRate);
            chain.setBands ({ band });

            BandEqCurve curve;
            curve.setSampleRate (sampleRate);
            curve.updateWithBands ({ band });

            for (float frequency : { 40.0f, 100.0f, band.freq, 1000.0f, 3000.0f, 12000.0f })
            {
                const float expected = curve.dbAtFrequency (frequency);
                const float measured = measureGainDb (chain, frequency);
                expectWithinAbsoluteError (measured, expected, 0.15f,
                                           "at " + juce::String (frequency) + " Hz");
            }
        }

        beginTest ("Left-only and right-only bands");
        {
            FilterChain chain;
            chain.prepare (sampleRate);
            chain.setBands ({ Band::withQ (0, 1000.0f, 6.0f, 1.0f, Band::Type::left),
                              Band::withQ (1, 1000.0f, -6.0f, 1.0f, Band::Type::right) });
            expectWithinAbsoluteError (measureGainDb (chain, 1000.0f, 0), 6.0f, 0.1f);
            expectWithinAbsoluteError (measureGainDb (chain, 1000.0f, 1), -6.0f, 0.1f);
        }

        beginTest ("Disabled bands do nothing");
        {
            FilterChain chain;
            chain.prepare (sampleRate);
            auto band = Band::withQ (0, 1000.0f, 12.0f, 1.0f, Band::Type::both);
            band.enabled = false;
            chain.setBands ({ band });
            expectWithinAbsoluteError (measureGainDb (chain, 1000.0f), 0.0f, 0.01f);
        }
    }
};

//==============================================================================
class SmoothingTests : public juce::UnitTest
{
public:
    SmoothingTests() : juce::UnitTest ("Changes don't click", "CabinEQ") {}

    void runTest() override
    {
        beginTest ("A big jump in gain glides");
        {
            // A low sine moves by at most ~0.0013 per sample. A click would be far bigger.
            FilterChain chain;
            chain.prepare (sampleRate);
            chain.setBands ({ Band::withQ (0, 100.0f, 0.0f, 1.0f, Band::Type::both) });

            const float maxStep = runSineAndChange (chain, [] (FilterChain& c) {
                c.setBands ({ Band::withQ (0, 100.0f, 18.0f, 1.0f, Band::Type::both) });
            });
            expectLessThan (maxStep, 0.02f);
        }

        beginTest ("Adding, reshaping and removing bands fades them");
        {
            FilterChain chain;
            chain.prepare (sampleRate);
            chain.setBands ({});

            float maxStep = runSineAndChange (chain, [] (FilterChain& c) {
                c.setBands ({ Band::withQ (5, 90.0f, 15.0f, 2.0f, Band::Type::both) });
            });
            expectLessThan (maxStep, 0.02f);

            maxStep = runSineAndChange (chain, [] (FilterChain& c) {
                c.setBands ({ Band::withQ (5, 90.0f, 15.0f, 0.7f, Band::Type::both, Band::Shape::lowShelf) });
            });
            expectLessThan (maxStep, 0.02f);

            maxStep = runSineAndChange (chain, [] (FilterChain& c) { c.setBands ({}); });
            expectLessThan (maxStep, 0.02f);
        }

        beginTest ("Random edits never blow up");
        {
            FilterChain chain;
            chain.prepare (sampleRate);
            juce::Random random (1234);
            juce::AudioBuffer<float> buffer (2, blockSize);
            float peak = 0.0f;

            for (int block = 0; block < 4000; ++block)
            {
                if (block % 3 == 0)
                {
                    std::vector<Band> bands;
                    const int numBands = random.nextInt (FilterChain::maxBands + 1);
                    for (int i = 0; i < numBands; ++i)
                        bands.push_back (Band::withQ (random.nextInt (40),
                                                      20.0f * std::pow (1000.0f, random.nextFloat()),
                                                      random.nextFloat() * 36.0f - 18.0f,
                                                      0.1f + random.nextFloat() * 20.0f,
                                                      static_cast<Band::Type> (random.nextInt (3)),
                                                      static_cast<Band::Shape> (random.nextInt (5)),
                                                      random.nextInt (4) != 0));
                    // Ids must be unique within a profile
                    std::sort (bands.begin(), bands.end(), [] (auto& a, auto& b) { return a.id < b.id; });
                    bands.erase (std::unique (bands.begin(), bands.end(), [] (auto& a, auto& b) { return a.id == b.id; }), bands.end());
                    chain.setBands (bands);
                }

                for (int channel = 0; channel < 2; ++channel)
                    for (int i = 0; i < blockSize; ++i)
                        buffer.setSample (channel, i, random.nextFloat() * 0.5f - 0.25f);

                juce::dsp::AudioBlock<float> audioBlock (buffer);
                chain.process (audioBlock);

                for (int channel = 0; channel < 2; ++channel)
                    for (int i = 0; i < blockSize; ++i)
                        peak = std::max (peak, std::abs (buffer.getSample (channel, i)));
            }

            expect (std::isfinite (peak), "output stayed finite");

            // Stacked random boosts can be very loud, but the filters must stay stable:
            // once the bands are gone and the input stops, the output has to die away
            chain.setBands ({});
            float tail = 0.0f;
            for (int block = 0; block < (int) sampleRate / blockSize; ++block)
            {
                buffer.clear();
                juce::dsp::AudioBlock<float> audioBlock (buffer);
                chain.process (audioBlock);
                tail = buffer.getMagnitude (0, blockSize);
            }
            expectLessThan (tail, 1.0e-6f);
        }
    }

private:
    /// Plays a 50 Hz sine, calls change halfway through, and returns the biggest sample-to-sample step.
    static float runSineAndChange (FilterChain& chain, std::function<void (FilterChain&)> change)
    {
        juce::AudioBuffer<float> buffer (2, blockSize);
        double phase = 0.0;
        const double increment = juce::MathConstants<double>::twoPi * 50.0 / sampleRate;
        float last = 0.0f, maxStep = 0.0f;
        const int numBlocks = (int) (sampleRate / blockSize);

        for (int block = 0; block < numBlocks; ++block)
        {
            if (block == numBlocks / 2)
                change (chain);

            for (int i = 0; i < blockSize; ++i)
            {
                const auto sample = (float) (0.1 * std::sin (phase));
                phase += increment;
                buffer.setSample (0, i, sample);
                buffer.setSample (1, i, sample);
            }

            juce::dsp::AudioBlock<float> audioBlock (buffer);
            chain.process (audioBlock);

            for (int i = 0; i < blockSize; ++i)
            {
                const float sample = buffer.getSample (0, i);
                if (block > 4) // ignore the filter's own start-up
                    maxStep = std::max (maxStep, std::abs (sample - last));
                last = sample;
            }
        }
        return maxStep;
    }
};

//==============================================================================
class ProcessorStateTests : public juce::UnitTest
{
public:
    ProcessorStateTests() : juce::UnitTest ("Profiles and saved state", "CabinEQ") {}

    void runTest() override
    {
        beginTest ("A new instance has one profile, selected");
        {
            CabinEqAudioProcessor processor;
            auto names = processor.getProfiles().getProfileNames();
            expectEquals ((int) names.size(), 1);
            expectEquals (processor.getProfiles().getSelectedProfileName(), CabinEqProfileManager::defaultProfileName);
            expect (processor.isAutoGainOn(), "auto gain is on for new users");
        }

        beginTest ("State round trips");
        {
            CabinEqAudioProcessor processor;
            auto& profiles = processor.getProfiles();
            auto second = profiles.addProfile ("HD 600");
            second.addBand (Band::withQ (0, 3000.0f, -2.5f, 2.0f, Band::Type::left, Band::Shape::peak));
            second.addBand (Band::withQ (0, 105.0f, 6.0f, 0.7f, Band::Type::both, Band::Shape::lowShelf));
            second.setVolume (-6.5f);
            processor.selectProfile ("HD 600");

            CabinEqAudioProcessor restored;
            auto state = stateOf (processor);
            restored.setStateInformation (state.getData(), (int) state.getSize());

            expectEquals (restored.getProfiles().getSelectedProfileName(), juce::String ("HD 600"));
            auto bandProfile = restored.getSelectedBandProfile();
            expectEquals ((int) bandProfile.getBands().size(), 2);
            expectWithinAbsoluteError (bandProfile.getVolume(), -6.5f, 0.001f);
            expectEquals ((int) bandProfile.getBands()[0].type, (int) Band::Type::left);
            expectEquals ((int) bandProfile.getBands()[1].shape, (int) Band::Shape::lowShelf);
            expectWithinAbsoluteError (bandProfile.getBands()[1].qFactor, 0.7f, 0.001f);
        }

        beginTest ("Old state migrates");
        {
            // What a95b4b7 saved: bands under AmplTree/MultiBandStep, and a master volume
            const char* oldXml = R"(
                <Params lastSelectedProfileId="Old" masterVolumeId="-2" hasLicenseId="0">
                  <PARAM id="dummyParam" value="0"/>
                  <Profile ProfileName="Old" Locked="1" ProfileVolume="1.5" MelodyVolume="0" NoiseVolume="0">
                    <AmplTree>
                      <MultiBandStep id="0" Enabled="1">
                        <Band id="0" freq="1000" ampl="4" bandwidth="1" bandtype="0"/>
                        <Band id="1" freq="60" ampl="-3" bandwidth="2" bandtype="2"/>
                      </MultiBandStep>
                      <MultiBandStep id="1" Enabled="0">
                        <Band id="0" freq="8000" ampl="-6" bandwidth="0.5" bandtype="1"/>
                      </MultiBandStep>
                    </AmplTree>
                  </Profile>
                  <Profile ProfileName="Empty" ProfileVolume="0">
                    <AmplTree><MultiBandStep id="0" Enabled="1"/></AmplTree>
                  </Profile>
                </Params>)";

            juce::MemoryBlock state;
            juce::AudioProcessor::copyXmlToBinary (*juce::parseXML (oldXml), state);

            CabinEqAudioProcessor processor;
            processor.setStateInformation (state.getData(), (int) state.getSize());

            expectEquals ((int) processor.getProfiles().getProfileNames().size(), 2);
            expectEquals (processor.getProfiles().getSelectedProfileName(), juce::String ("Old"));
            expect (! processor.isAutoGainOn(), "auto gain stays off for existing users");

            auto bandProfile = processor.getSelectedBandProfile();
            const auto& bands = bandProfile.getBands();
            expectEquals ((int) bands.size(), 3);
            expectWithinAbsoluteError (bandProfile.getVolume(), -0.5f, 0.001f); // 1.5 preamp + -2 master
            expect (bands[0].enabled && bands[1].enabled, "bands from the enabled step are on");
            expect (! bands[2].enabled, "bands from the disabled step are kept, but off");
            expectEquals ((int) bands[1].type, (int) Band::Type::right);
            expectWithinAbsoluteError (bands[1].bandwidth, 2.0f, 0.001f);

            std::set<int> ids;
            for (const auto& band : bands)
                ids.insert (band.id);
            expectEquals ((int) ids.size(), 3, "ids are unique after merging steps");

            auto xml = processor.parameters.copyState().toXmlString();
            expect (! xml.contains ("AmplTree") && ! xml.contains ("Locked") && ! xml.contains ("dummyParam")
                    && ! xml.contains ("masterVolumeId") && ! xml.contains ("hasLicenseId"), "old properties are gone");
        }

        beginTest ("Undo and redo, and undo selects the profile it changed");
        {
            CabinEqAudioProcessor processor;
            auto& profiles = processor.getProfiles();
            auto& undo = processor.getUndoManager();

            auto first = profiles.getSelectedProfile();
            undo.beginNewTransaction();
            first.addBand (Band::withQ (0, 500.0f, 3.0f, 1.0f, Band::Type::both));

            undo.beginNewTransaction();
            auto other = profiles.addProfile ("Other");
            processor.selectProfile ("Other");

            processor.undo(); // removes "Other"
            pumpMessages();
            expectEquals ((int) profiles.getProfileNames().size(), 1);
            expectEquals (profiles.getSelectedProfileName(), CabinEqProfileManager::defaultProfileName);

            processor.undo(); // removes the band
            expectEquals (profiles.getSelectedProfile().getNumBands(), 0);

            processor.redo();
            expectEquals (profiles.getSelectedProfile().getNumBands(), 1);
        }

        beginTest ("Profile names stay unique");
        {
            CabinEqAudioProcessor processor;
            auto& profiles = processor.getProfiles();
            auto copy = profiles.duplicateProfile (CabinEqProfileManager::defaultProfileName);
            expectEquals (copy.getName(), CabinEqProfileManager::defaultProfileName + " copy");
            profiles.addProfile ("A");
            profiles.addProfile ("A");
            expect (profiles.getProfileNamed ("A 2").has_value());
            profiles.renameProfile ("A 2", "A");
            expect (profiles.getProfileNamed ("A 2").has_value(), "renaming onto a taken name keeps it unique");
        }
    }
};

//==============================================================================
class ProcessorAudioTests : public juce::UnitTest
{
public:
    ProcessorAudioTests() : juce::UnitTest ("Processor audio", "CabinEQ") {}

    void runTest() override
    {
        beginTest ("Bypass passes audio through untouched");
        {
            CabinEqAudioProcessor processor;
            processor.getSelectedProfile().addBand (Band::withQ (0, 1000.0f, 12.0f, 1.0f, Band::Type::both));
            processor.getSelectedProfile().setVolume (-6.0f);
            processor.setPlayConfigDetails (2, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);
            processor.parameters.getParameter (ParamIDs::bypass)->setValueNotifyingHost (1.0f);

            juce::AudioBuffer<float> buffer (2, blockSize);
            juce::MidiBuffer midi;
            juce::Random random (7);
            float maxDifference = 0.0f;

            for (int block = 0; block < 50; ++block)
            {
                juce::AudioBuffer<float> input (2, blockSize);
                for (int channel = 0; channel < 2; ++channel)
                    for (int i = 0; i < blockSize; ++i)
                        input.setSample (channel, i, random.nextFloat() - 0.5f);
                buffer.makeCopyOf (input);
                processor.processBlock (buffer, midi);

                if (block > 10)
                    for (int channel = 0; channel < 2; ++channel)
                        for (int i = 0; i < blockSize; ++i)
                            maxDifference = std::max (maxDifference, std::abs (buffer.getSample (channel, i) - input.getSample (channel, i)));
            }
            expectEquals (maxDifference, 0.0f);
        }

        beginTest ("Blocks bigger than promised are fine");
        {
            CabinEqAudioProcessor processor;
            processor.getSelectedProfile().addBand (Band::withQ (0, 1000.0f, 6.0f, 1.0f, Band::Type::both));
            processor.setPlayConfigDetails (2, 2, sampleRate, 64);
            processor.prepareToPlay (sampleRate, 64);

            juce::AudioBuffer<float> buffer (2, 4096);
            buffer.clear();
            buffer.setSample (0, 0, 1.0f);
            juce::MidiBuffer midi;
            processor.processBlock (buffer, midi);
            expect (std::isfinite (buffer.getMagnitude (0, 4096)));
        }

        beginTest ("The editor opens at its remembered size");
        {
            CabinEqAudioProcessor processor;
            {
                std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
                expectEquals (editor->getWidth(), 1080);
                expectEquals (editor->getHeight(), 680);
                editor->setSize (900, 600);
            }
            std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
            expectEquals (editor->getWidth(), 900);
        }

        beginTest ("Auto gain cancels a boost");
        {
            CabinEqAudioProcessor processor;
            processor.getSelectedProfile().addBand (Band::withQ (0, 1000.0f, 6.0f, 0.3f, Band::Type::both));
            pumpMessages();
            expectLessThan (processor.getAutoGainDb(), -3.0f);
            expectGreaterThan (processor.getAutoGainDb(), -6.5f);
        }
    }
};

//==============================================================================
class PresetFileTests : public juce::UnitTest
{
public:
    PresetFileTests() : juce::UnitTest ("Equalizer APO / AutoEQ files", "CabinEQ") {}

    void runTest() override
    {
        beginTest ("Reads an AutoEQ ParametricEQ.txt");
        {
            const juce::String text = "Preamp: -6.2 dB\n"
                                      "Filter 1: ON LSC Fc 105 Hz Gain 5.8 dB Q 0.70\n"
                                      "Filter 2: ON PK Fc 2310 Hz Gain -2.3 dB Q 1.93\n"
                                      "Filter 3: OFF PK Fc 4000 Hz Gain 3 dB Q 2\n"
                                      "Filter 4: ON HSC Fc 10000 Hz Gain -4.1 dB Q 0.70\n"
                                      "Filter 5: ON HP Fc 20 Hz\n"
                                      "Filter 6: ON PK Fc 300 Hz Gain 1 dB BW Oct 1\n";
            auto result = EqPresetFile::parse (text);
            expect (result.has_value());
            if (! result.has_value())
                return;

            const auto& bands = result->getBands();
            expectEquals ((int) bands.size(), 6);
            expectWithinAbsoluteError (result->getVolume(), -6.2f, 0.001f);
            expectEquals ((int) bands[0].shape, (int) Band::Shape::lowShelf);
            expectWithinAbsoluteError (bands[1].freq, 2310.0f, 0.01f);
            expectWithinAbsoluteError (bands[1].ampl, -2.3f, 0.001f);
            expectWithinAbsoluteError (bands[1].qFactor, 1.93f, 0.001f);
            expect (! bands[2].enabled);
            expectEquals ((int) bands[4].shape, (int) Band::Shape::lowCut);
            expectWithinAbsoluteError (bands[5].bandwidth, 1.0f, 0.001f);
        }

        beginTest ("Writes what it reads");
        {
            BandProfile original ({ Band::withQ (0, 105.0f, 5.8f, 0.7f, Band::Type::both, Band::Shape::lowShelf),
                                    Band::withQ (1, 2310.0f, -2.3f, 1.93f, Band::Type::both),
                                    Band::withQ (2, 9000.0f, 0.0f, 0.71f, Band::Type::both, Band::Shape::highCut) },
                                  -6.2f);
            auto parsed = EqPresetFile::parse (EqPresetFile::write (original));
            expect (parsed.has_value());
            if (parsed.has_value())
            {
                expectEquals ((int) parsed->getBands().size(), 3);
                expectWithinAbsoluteError (parsed->getBands()[1].qFactor, 1.93f, 0.01f);
                expectEquals ((int) parsed->getBands()[2].shape, (int) Band::Shape::highCut);
            }
        }

        beginTest ("Rejects text with no filters");
        expect (! EqPresetFile::parse ("hello world").has_value());
    }
};

//==============================================================================
class GraphInteractionTests : public juce::UnitTest
{
public:
    GraphInteractionTests() : juce::UnitTest ("Graph interaction", "CabinEQ") {}

    void runTest() override
    {
        CabinEqAudioProcessor processor;
        CabinPeqGraph graph (processor);
        graph.setBounds (0, 0, 1000, 500);
        auto& profile = processor.getProfiles();

        beginTest ("Clicking the line adds a band, and dragging shapes it");
        {
            // With no bands the line is at 0 dB, halfway down the plot
            const juce::Point<float> onLine { 500.0f, (500.0f - 22.0f) * 0.5f };
            press (graph, onLine);
            drag (graph, onLine, onLine.translated (0.0f, -60.0f));
            release (graph, onLine.translated (0.0f, -60.0f));

            auto bands = profile.getSelectedProfile().getBandProfile().getBands();
            expectEquals ((int) bands.size(), 1);
            if (! bands.empty())
            {
                expectGreaterThan (bands[0].ampl, 2.0f);
                expectWithinAbsoluteError (bands[0].freq, 632.0f, 40.0f); // the middle of 20 Hz to 20 kHz
            }
            expectEquals (graph.getFocusedBandId(), bands.empty() ? -2 : bands[0].id);

            processor.undo();
            expectEquals (profile.getSelectedProfile().getNumBands(), 0, "adding and dragging undo as one step");
            processor.redo();
            graph.refresh();
        }

        beginTest ("Dragging empty space selects, and Delete removes");
        {
            profile.getSelectedProfile().addBand (Band::withQ (0, 100.0f, 3.0f, 1.0f, Band::Type::both));
            graph.refresh();

            press (graph, { 5.0f, 5.0f });
            drag (graph, { 5.0f, 5.0f }, { 995.0f, 470.0f });
            release (graph, { 995.0f, 470.0f });
            expectEquals (graph.getNumSelected(), 2);

            graph.keyPressed (juce::KeyPress (juce::KeyPress::deleteKey));
            expectEquals (profile.getSelectedProfile().getNumBands(), 0);
        }
    }

private:
    static juce::MouseEvent event (juce::Component& c, juce::Point<float> position, juce::Point<float> downPosition, juce::ModifierKeys mods, int clicks = 1)
    {
        return juce::MouseEvent (juce::Desktop::getInstance().getMainMouseSource(), position, mods, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                 &c, &c, juce::Time::getCurrentTime(), downPosition, juce::Time::getCurrentTime(), clicks, false);
    }

    static void press (CabinPeqGraph& graph, juce::Point<float> at)
    {
        graph.mouseMove (event (graph, at, at, {}));
        graph.mouseDown (event (graph, at, at, juce::ModifierKeys::leftButtonModifier));
    }

    static void drag (CabinPeqGraph& graph, juce::Point<float> from, juce::Point<float> to)
    {
        for (int step = 1; step <= 10; ++step)
            graph.mouseDrag (event (graph, from + (to - from) * (float) step / 10.0f, from, juce::ModifierKeys::leftButtonModifier));
    }

    static void release (CabinPeqGraph& graph, juce::Point<float> at)
    {
        graph.mouseUp (event (graph, at, at, {}));
    }
};

static FilterResponseTests filterResponseTests;
static GraphInteractionTests graphInteractionTests;
static SmoothingTests smoothingTests;
static ProcessorStateTests processorStateTests;
static ProcessorAudioTests processorAudioTests;
static PresetFileTests presetFileTests;

//==============================================================================
/// Renders the editor with a demo profile to a PNG, to check the UI without clicking around.
static int writeSnapshot (const juce::File& file, int width, int height, bool channelSpecific)
{
    CabinEqAudioProcessor processor;
    auto profile = processor.getSelectedProfile();
    profile.setBands ({ Band::withQ (0, 32.0f, 0.0f, 0.71f, Band::Type::both, Band::Shape::lowCut),
                        Band::withQ (0, 105.0f, 5.5f, 0.7f, Band::Type::both, Band::Shape::lowShelf),
                        Band::withQ (0, 2300.0f, -3.2f, 1.9f, channelSpecific ? Band::Type::left : Band::Type::both),
                        Band::withQ (0, 5400.0f, 4.0f, 3.0f, Band::Type::both),
                        Band::withQ (0, 9800.0f, -2.5f, 0.7f, Band::Type::both, Band::Shape::highShelf) });
    profile.setVolume (-6.0f);
    processor.getProfiles().addProfile ("HD 600 (AutoEQ)");
    processor.getProfiles().addProfile ("Studio monitors");
    juce::MessageManager::getInstance()->runDispatchLoopUntil (50);

    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
    editor->setSize (width, height);
    juce::MessageManager::getInstance()->runDispatchLoopUntil (50);

    // Select the third band so the inspector has something to show
    std::function<juce::Component* (juce::Component&)> findGraph = [&] (juce::Component& parent) -> juce::Component*
    {
        for (auto* child : parent.getChildren())
        {
            if (child->getComponentID() == "graph")
                return child;
            if (auto* found = findGraph (*child))
                return found;
        }
        return nullptr;
    };
    if (auto* graph = dynamic_cast<CabinPeqGraph*> (findGraph (*editor)))
        graph->focusBand (2);
    juce::MessageManager::getInstance()->runDispatchLoopUntil (50);

    auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 2.0f);
    juce::PNGImageFormat png;
    file.deleteFile();
    juce::FileOutputStream stream (file);
    return png.writeImageToStream (image, stream) ? 0 : 1;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juce;

    if (argc >= 3 && juce::String (argv[1]) == "--snapshot")
        return writeSnapshot (juce::File (argv[2]), argc >= 5 ? juce::String (argv[3]).getIntValue() : 1080,
                              argc >= 5 ? juce::String (argv[4]).getIntValue() : 680, argc >= 6);

    CabinEqProfileManager::shouldBackUpOldState = false;

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runTestsInCategory ("CabinEQ");

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    std::cout << (failures == 0 ? "All tests passed" : juce::String (failures) + " failures") << std::endl;
    return failures == 0 ? 0 : 1;
}
