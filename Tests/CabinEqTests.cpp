/*
  ==============================================================================

    CabinEqTests.cpp

    Headless tests for the DSP and the saved state. Build and run with:
        cmake --build build --target CabinEQ_Tests
        build/CabinEQ_Tests_artefacts/Release/CabinEQ_Tests

  ==============================================================================
*/

#include <JuceHeader.h>
#include <map>
#include "../Source/CabinEqAudioProcessor.h"
#include "../Source/FilterChain.h"
#include "../Source/BandEqCurve.h"
#include "../Source/EqPresetFile.h"
#include "../Source/CabinPeqGraph.h"
#include "../Source/CalibrationPanel.h"
#include "../Source/CalibrationSettings.h"
#include "../Source/CurveFilter.h"

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

        beginTest ("Bands that go and come straight back keep their slots");
        {
            std::vector<Band> bands;
            for (int i = 0; i < 24; ++i)
                bands.push_back (Band::withQ (i, 30.0f * std::pow (1.3f, (float) i), 3.0f, 3.0f, Band::Type::both));

            FilterChain disturbed, untouched;
            for (auto* chain : { &disturbed, &untouched })
            {
                chain->prepare (sampleRate);
                chain->setBands (bands);
            }

            juce::Random random (9);
            float maxDifference = 0.0f;
            for (int block = 0; block < 40; ++block)
            {
                if (block == 10)
                {
                    disturbed.setBands ({});
                    disturbed.setBands (bands);
                }

                juce::AudioBuffer<float> a (2, blockSize), b (2, blockSize);
                for (int i = 0; i < blockSize; ++i)
                    a.setSample (0, i, random.nextFloat() - 0.5f);
                a.copyFrom (1, 0, a, 0, 0, blockSize);
                b.makeCopyOf (a);
                juce::dsp::AudioBlock<float> blockA (a), blockB (b);
                disturbed.process (blockA);
                untouched.process (blockB);
                for (int i = 0; i < blockSize; ++i)
                    maxDifference = std::max (maxDifference, std::abs (a.getSample (0, i) - b.getSample (0, i)));
            }
            expectLessThan (maxDifference, 1.0e-6f);
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

        beginTest ("A new profile starts as a curve, and one made from bands (an import) stays bands");
        {
            CabinEqAudioProcessor processor;
            auto& profiles = processor.getProfiles();
            expect (profiles.getSelectedProfile().getMode() == BandProfile::Mode::curve);
            expect (profiles.addProfile ("Empty").getMode() == BandProfile::Mode::curve);
            expect (profiles.addProfile ("Imported", BandProfile ({ Band::withQ (0, 500.0f, 3.0f, 1.0f, Band::Type::both) }, 0.0f)).getMode()
                    == BandProfile::Mode::bands);
            juce::ValueTree saved (CabinEqProfile::idProfile);
            saved.setProperty (CabinEqProfile::idProfileName, "Saved before curves", nullptr);
            expect (CabinEqProfile (saved, nullptr).getMode() == BandProfile::Mode::bands, "a profile saved before curves is bands");
        }

        beginTest ("Undo and redo, and undo selects the profile it changed");
        {
            CabinEqAudioProcessor processor;
            auto& profiles = processor.getProfiles();
            auto& undo = processor.getUndoManager();

            auto first = profiles.getSelectedProfile();
            first.setMode (BandProfile::Mode::bands);
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

        beginTest ("Renaming the selected profile doesn't disturb the audio");
        {
            // Before, renaming briefly sent no bands, and 20 bands came back in each other's slots
            auto makeProcessor = [] (std::unique_ptr<CabinEqAudioProcessor>& processor)
            {
                processor = std::make_unique<CabinEqAudioProcessor>();
                std::vector<Band> bands;
                for (int i = 0; i < 20; ++i)
                    bands.push_back (Band::withQ (i, 40.0f * std::pow (1.35f, (float) i), (i % 2 == 0 ? 4.0f : -4.0f), 2.0f, Band::Type::both));
                processor->getSelectedProfile().setBands (bands);
                processor->setPlayConfigDetails (2, 2, sampleRate, blockSize);
                processor->prepareToPlay (sampleRate, blockSize);
                processor->parameters.getParameter (ParamIDs::autoGain)->setValueNotifyingHost (0.0f);
            };

            std::unique_ptr<CabinEqAudioProcessor> renamed, untouched;
            makeProcessor (renamed);
            makeProcessor (untouched);

            juce::Random random (3);
            juce::MidiBuffer midi;
            float maxDifference = 0.0f;
            for (int block = 0; block < 60; ++block)
            {
                if (block == 20)
                    renamed->getProfiles().renameProfile (CabinEqProfileManager::defaultProfileName, "Renamed");

                juce::AudioBuffer<float> a (2, blockSize), b (2, blockSize);
                for (int channel = 0; channel < 2; ++channel)
                    for (int i = 0; i < blockSize; ++i)
                        a.setSample (channel, i, random.nextFloat() * 0.5f - 0.25f);
                b.makeCopyOf (a);
                renamed->processBlock (a, midi);
                untouched->processBlock (b, midi);

                for (int channel = 0; channel < 2; ++channel)
                    for (int i = 0; i < blockSize; ++i)
                        maxDifference = std::max (maxDifference, std::abs (a.getSample (channel, i) - b.getSample (channel, i)));
            }
            expectEquals (renamed->getProfiles().getSelectedProfileName(), juce::String ("Renamed"));
            expectLessThan (maxDifference, 1.0e-6f);
        }

        beginTest ("Undoing an import goes back to the profile you were on");
        {
            CabinEqAudioProcessor processor;
            auto& profiles = processor.getProfiles();
            profiles.addProfile ("A");
            profiles.addProfile ("B");
            processor.selectProfile ("B");
            pumpMessages();

            processor.getUndoManager().beginNewTransaction();
            auto imported = profiles.addProfile ("Imported", BandProfile ({ Band::withQ (0, 1000.0f, 3.0f, 1.0f, Band::Type::both) }, 0.0f));
            processor.selectProfile (imported.getName());
            pumpMessages();

            processor.undo();
            pumpMessages();
            expectEquals (profiles.getSelectedProfileName(), juce::String ("B"));
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

        beginTest ("Master volume boosts, even bypassed, and the limiter stops it clipping");
        {
            CabinEqAudioProcessor processor;
            processor.setPlayConfigDetails (2, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);
            auto* volume = processor.parameters.getParameter (ParamIDs::volume);
            volume->setValueNotifyingHost (volume->convertTo0to1 (12.0f));

            auto run = [&processor] (float amplitude)
            {
                juce::MidiBuffer midi;
                juce::AudioBuffer<float> buffer (2, blockSize);
                double phase = 0.0;
                float peak = 0.0f;
                for (int block = 0; block < 100; ++block)
                {
                    for (int i = 0; i < blockSize; ++i)
                    {
                        const auto sample = (float) (amplitude * std::sin (phase));
                        phase += juce::MathConstants<double>::twoPi * 1000.0 / sampleRate;
                        buffer.setSample (0, i, sample);
                        buffer.setSample (1, i, sample);
                    }
                    processor.processBlock (buffer, midi);
                    if (block > 20)
                        peak = std::max (peak, buffer.getMagnitude (0, blockSize));
                }
                return peak;
            };

            expectWithinAbsoluteError (juce::Decibels::gainToDecibels (run (0.01f) / 0.01f), 12.0f, 0.2f, "a quiet signal gets 12 dB louder");
            expectLessThan (run (0.8f), 0.98f, "a loud one is held under full scale");

            processor.parameters.getParameter (ParamIDs::bypass)->setValueNotifyingHost (1.0f);
            expectWithinAbsoluteError (juce::Decibels::gainToDecibels (run (0.01f) / 0.01f), 12.0f, 0.2f, "and it still applies with the EQ off");
        }

        beginTest ("Auto gain cancels a boost");
        {
            CabinEqAudioProcessor processor;
            processor.getSelectedProfile().setMode (BandProfile::Mode::bands);
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
        profile.getSelectedProfile().setMode (BandProfile::Mode::bands); // a new profile starts as a curve
        graph.refresh();

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

        beginTest ("Double-clicking the line adds one band, and leaves it on");
        {
            profile.getSelectedProfile().setBands ({});
            graph.refresh();
            const juce::Point<float> onLine { 300.0f, (500.0f - 22.0f) * 0.5f };
            press (graph, onLine);
            release (graph, onLine);
            graph.mouseDown (event (graph, onLine, onLine, juce::ModifierKeys::leftButtonModifier, 2));
            graph.mouseUp (event (graph, onLine, onLine, {}, 2));
            graph.mouseDoubleClick (event (graph, onLine, onLine, {}, 2));

            auto bands = profile.getSelectedProfile().getBandProfile().getBands();
            expectEquals ((int) bands.size(), 1);
            expect (! bands.empty() && bands[0].enabled, "the band is on");
            profile.getSelectedProfile().setBands ({});
            graph.refresh();
        }

        beginTest ("Shift-drag up widens a band, Shift-click selects, right-click deletes");
        {
            profile.getSelectedProfile().setBands ({ Band::withQ (0, 1000.0f, 6.0f, 2.0f, Band::Type::both) });
            graph.refresh();
            const auto bandId = profile.getSelectedProfile().getBandProfile().getBands()[0].id;

            // Where the handle is: 1 kHz across 20 Hz to 20 kHz, +6 dB in a +/-30 dB plot inset by 14 px
            const float xFor1k = std::log (1000.0f / 20.0f) / std::log (1000.0f) * 1000.0f;
            const float yFor6dB = 14.0f + (30.0f - 6.0f) / 60.0f * (478.0f - 28.0f);
            const juce::Point<float> handle { xFor1k, yFor6dB };

            const auto shift = juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier | juce::ModifierKeys::shiftModifier);
            graph.mouseDown (event (graph, handle, handle, shift));
            for (int step = 1; step <= 10; ++step)
                graph.mouseDrag (event (graph, handle.translated (0.0f, -6.0f * (float) step), handle, shift));
            graph.mouseUp (event (graph, handle.translated (0.0f, -60.0f), handle, {}));

            auto widened = profile.getSelectedProfile().getBand (bandId);
            expect (widened.has_value() && widened->qFactor < 1.4f && widened->qFactor > 1.0f, "dragging up 60 px lowered the Q from 2 to about 1.2 (wider)");
            expectWithinAbsoluteError (widened.has_value() ? widened->ampl : 0.0f, 6.0f, 0.01f, "the gain didn't move");

            graph.refresh();
            graph.mouseDown (event (graph, handle, handle, shift));
            graph.mouseUp (event (graph, handle, handle, {}));
            expectEquals (graph.getNumSelected(), 1, "shift-click without dragging selects");

            // Drag to move it, then hold Shift partway: it stops moving and gets wider instead
            const auto left = juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier);
            const auto before = *profile.getSelectedProfile().getBand (bandId);
            graph.mouseDown (event (graph, handle, handle, left));
            graph.mouseDrag (event (graph, handle.translated (0.0f, -20.0f), handle, left));
            const auto moved = *profile.getSelectedProfile().getBand (bandId);
            for (int step = 1; step <= 5; ++step)
                graph.mouseDrag (event (graph, handle.translated (0.0f, -20.0f - 10.0f * (float) step), handle, shift));
            graph.mouseUp (event (graph, handle.translated (0.0f, -70.0f), handle, {}));
            const auto after = *profile.getSelectedProfile().getBand (bandId);
            expectGreaterThan (moved.ampl, before.ampl, "the plain drag moved it up");
            expectWithinAbsoluteError (after.ampl, moved.ampl, 0.01f, "holding Shift stopped it moving");
            expectLessThan (after.qFactor, moved.qFactor, "and made it wider");

            graph.refresh();
            graph.mouseDown (event (graph, handle, handle, shift));
            graph.mouseUp (event (graph, handle, handle, {}));

            // The band moved up 20 px in that drag, so find it again
            const auto now = *profile.getSelectedProfile().getBand (bandId);
            const juce::Point<float> movedHandle { handle.x, 14.0f + (30.0f - now.ampl) / 60.0f * (478.0f - 28.0f) };
            graph.mouseDown (event (graph, movedHandle, movedHandle, juce::ModifierKeys::rightButtonModifier));
            expectEquals (profile.getSelectedProfile().getNumBands(), 0, "right-click deletes");
            processor.undo();
            expectEquals (profile.getSelectedProfile().getNumBands(), 1, "and undo brings it back");
            profile.getSelectedProfile().setBands ({});
            graph.refresh();
        }

        beginTest ("The view's dB range zooms from the -/+ and the axis, without limiting bands");
        {
            auto range = [&processor] { return (float) processor.parameters.state.getProperty ("graphRange", 30.0f); };
            processor.parameters.state.setProperty ("graphRange", 30.0f, nullptr);

            // The control sits in the top right: - on its left, + on its right
            const juce::Point<float> minus { 1000.0f - 8.0f - 120.0f + 13.0f, 17.0f }, plus { 1000.0f - 8.0f - 13.0f, 17.0f };
            press (graph, minus);
            release (graph, minus);
            expectEquals (range(), 24.0f, "- zooms in a step");
            press (graph, plus);
            release (graph, plus);
            press (graph, plus);
            release (graph, plus);
            expectEquals (range(), 36.0f, "+ zooms out, past the old +/-30 dB");

            // Drag up on the axis to zoom in
            const juce::Point<float> onAxis { 20.0f, 240.0f };
            press (graph, onAxis);
            drag (graph, onAxis, onAxis.translated (0.0f, -150.0f));
            release (graph, onAxis.translated (0.0f, -150.0f));
            expectWithinAbsoluteError (range(), 36.0f / std::exp (1.0f), 0.5f);

            // A band can still be bigger than what's shown
            processor.parameters.state.setProperty ("graphRange", 6.0f, nullptr);
            profile.getSelectedProfile().setBands ({ Band::withQ (0, 1000.0f, 20.0f, 1.0f, Band::Type::both) });
            graph.refresh();
            expectWithinAbsoluteError (profile.getSelectedProfile().getBandProfile().getBands()[0].ampl, 20.0f, 0.01f);
            profile.getSelectedProfile().setBands ({});
            processor.parameters.state.setProperty ("graphRange", 30.0f, nullptr);
            graph.refresh();
        }

        beginTest ("Scrolling over a band zooms, and leaves its width alone");
        {
            profile.getSelectedProfile().setBands ({ Band::withQ (0, 632.0f, 0.0f, 2.0f, Band::Type::both) });
            graph.refresh();
            const juce::Point<float> onBand { 500.0f, (500.0f - 22.0f) * 0.5f };
            juce::MouseWheelDetails wheel {};
            wheel.deltaY = 0.5f;
            wheel.deltaX = 0.05f; // a little sideways, as trackpads do
            graph.mouseWheelMove (event (graph, onBand, onBand, {}), wheel);
            expectWithinAbsoluteError (profile.getSelectedProfile().getBandProfile().getBands()[0].qFactor, 2.0f, 0.001f);

            auto& state = processor.parameters.state;
            const float low = state.getProperty ("graphLowFrequency"), high = state.getProperty ("graphHighFrequency");
            expectWithinAbsoluteError (low * std::pow (high / low, 0.5f), 632.5f, 3.0f, "zoomed in on the frequency under the mouse");
            profile.getSelectedProfile().setBands ({});
            graph.mouseDoubleClick (event (graph, { 500.0f, 490.0f }, { 500.0f, 490.0f }, {}, 2));
        }

        beginTest ("Scrolling zooms in on frequencies around the mouse, and double-clicking the axis resets");
        {
            auto& state = processor.parameters.state;
            auto frequencyAt = [] (float x, float low, float high) { return low * std::pow (high / low, x / 1000.0f); };

            const juce::Point<float> mouse { 700.0f, 300.0f };
            const float before = frequencyAt (mouse.x, 20.0f, 20000.0f);
            juce::MouseWheelDetails wheel {};
            wheel.deltaY = 0.5f;
            graph.mouseWheelMove (event (graph, mouse, mouse, {}), wheel);

            const float low = state.getProperty ("graphLowFrequency"), high = state.getProperty ("graphHighFrequency");
            expectLessThan (high / low, 1000.0f * 0.75f, "the view got narrower");
            expectWithinAbsoluteError (frequencyAt (mouse.x, low, high) / before, 1.0f, 0.01f, "the frequency under the mouse stayed put");

            // Double-click the frequency axis along the bottom
            const juce::Point<float> axis { 500.0f, 490.0f };
            graph.mouseDown (event (graph, axis, axis, juce::ModifierKeys::leftButtonModifier));
            graph.mouseUp (event (graph, axis, axis, {}));
            graph.mouseDoubleClick (event (graph, axis, axis, {}, 2));
            expectWithinAbsoluteError ((float) state.getProperty ("graphLowFrequency"), 20.0f, 0.01f);
            expectWithinAbsoluteError ((float) state.getProperty ("graphHighFrequency"), 20000.0f, 0.1f);
        }

        beginTest ("The end spots resize the group around its middle, the middle one moves it, and two spots have a grip");
        {
            auto& state = processor.parameters.state;
            auto& player = processor.getCalibration();
            auto xFor = [] (float hz) { return std::log (hz / 20.0f) / std::log (1000.0f) * 1000.0f; };
            const float chipY = 478.0f - 16.0f, lineY = 100.0f;
            auto dragFromTo = [&] (juce::Point<float> from, juce::Point<float> to)
            {
                graph.mouseMove (event (graph, from, from, {}));
                graph.mouseDown (event (graph, from, from, juce::ModifierKeys::leftButtonModifier));
                graph.mouseDrag (event (graph, to, from, juce::ModifierKeys::leftButtonModifier));
                graph.mouseUp (event (graph, to, from, {}));
            };
            auto frequency = [&state] (int spot) { return CalibrationSettings::getSpotFrequency (state, spot); };
            auto place = [&] (std::vector<float> frequencies)
            {
                for (size_t i = 0; i < frequencies.size(); ++i)
                    CalibrationSettings::setSpotFrequency (state, player, (int) i, frequencies[i]);
            };

            expectEquals (CalibrationSettings::getSpotCount (state), 3, "three spots to start with");
            expect (CalibrationSettings::getMode (state) == CalibrationPlayer::Mode::grid, "the grid to start with");
            CalibrationSettings::setMode (state, player, CalibrationPlayer::Mode::spots);
            place ({ 200.0f, 1000.0f, 5000.0f });
            graph.setCalibrationSpotsVisible (true);

            // The top spot's chip, 5 kHz to 10 kHz: the middle spot stays, and the bottom spreads out to match
            dragFromTo ({ xFor (5000.0f), chipY }, { xFor (10000.0f), chipY });
            expectWithinAbsoluteError (frequency (2), 10000.0f, 150.0f);
            expectWithinAbsoluteError (frequency (1), 1000.0f, 0.5f, "the middle stayed put");
            expectWithinAbsoluteError (frequency (0), 100.0f, 2.0f, "the bottom went out as far (in octaves)");

            // The middle spot's line moves them all by the same ratio
            dragFromTo ({ xFor (1000.0f), lineY }, { xFor (1500.0f), lineY });
            expectWithinAbsoluteError (frequency (0), 150.0f, 3.0f);
            expectWithinAbsoluteError (frequency (2), 15000.0f, 300.0f);

            // The bottom spot's chip, dragged in: the top comes in to match, the middle stays
            dragFromTo ({ xFor (150.0f), chipY }, { xFor (300.0f), chipY });
            expectWithinAbsoluteError (frequency (0), 300.0f, 6.0f);
            expectWithinAbsoluteError (frequency (1), 1500.0f, 1.0f, "the middle stayed put");
            expectWithinAbsoluteError (frequency (2), 7500.0f, 150.0f);
            expectEquals (profile.getSelectedProfile().getNumBands(), 0, "and none of that added a band");

            // Four spots spread out between the ends; the inner ones move them
            CalibrationSettings::setSpotCount (state, player, 4);
            expectWithinAbsoluteError (frequency (0), 300.0f, 6.0f);
            expectWithinAbsoluteError (frequency (3), 7500.0f, 150.0f);
            const float inner = frequency (1);
            dragFromTo ({ xFor (inner), lineY }, { xFor (inner * 0.5f), lineY });
            expectWithinAbsoluteError (frequency (0), 150.0f, 4.0f, "an inner spot moved them all");

            // Two spots: the grip between them moves both
            CalibrationSettings::setSpotCount (state, player, 2);
            place ({ 200.0f, 800.0f });
            dragFromTo ({ xFor (400.0f), chipY }, { xFor (800.0f), chipY });
            expectWithinAbsoluteError (frequency (0), 400.0f, 8.0f);
            expectWithinAbsoluteError (frequency (1), 1600.0f, 32.0f, "both moved by the same ratio");

            CalibrationSettings::setSpotCount (state, player, 3);
            graph.setCalibrationSpotsVisible (false);
        }

        beginTest ("Dragging empty space selects, and Delete removes");
        {
            profile.getSelectedProfile().addBand (Band::withQ (0, 100.0f, 3.0f, 1.0f, Band::Type::both));
            profile.getSelectedProfile().addBand (Band::withQ (0, 3000.0f, -3.0f, 1.0f, Band::Type::both));
            graph.refresh();

            press (graph, { 50.0f, 40.0f }); // just right of the dB axis, which zooms
            drag (graph, { 50.0f, 40.0f }, { 995.0f, 470.0f });
            release (graph, { 995.0f, 470.0f });
            expectEquals (graph.getNumSelected(), 2);

            graph.keyPressed (juce::KeyPress (juce::KeyPress::deleteKey));
            expectEquals (profile.getSelectedProfile().getNumBands(), 0);
        }

        beginTest ("In curve mode, clicking anywhere adds a point, dragging moves it, and right-click deletes it");
        {
            auto p = profile.getSelectedProfile();
            p.setMode (BandProfile::Mode::curve);
            graph.refresh();

            // Where the plot puts 1 kHz and +6 dB, as in the band tests
            const float xFor1k = std::log (1000.0f / 20.0f) / std::log (1000.0f) * 1000.0f;
            const float yFor6dB = 14.0f + (30.0f - 6.0f) / 60.0f * (478.0f - 28.0f);
            const float yFor0dB = 14.0f + 0.5f * (478.0f - 28.0f);
            const juce::Point<float> at { xFor1k, yFor6dB };
            press (graph, at);
            release (graph, at);
            expectEquals (p.getNumPoints(), 1);
            expectEquals (p.getNumBands(), 0, "no band was added");
            auto points = p.getBandProfile().getPoints();
            if (! points.empty())
            {
                expectWithinAbsoluteError (points[0].freq, 1000.0f, 15.0f);
                expectWithinAbsoluteError (points[0].gain, 6.0f, 0.2f);
            }

            // Drag it down to 0 dB
            press (graph, at);
            drag (graph, at, { xFor1k, yFor0dB });
            release (graph, { xFor1k, yFor0dB });
            points = p.getBandProfile().getPoints();
            expect (! points.empty() && std::abs (points[0].gain) < 0.3f, "dragged down to 0 dB");
            expectEquals (p.getNumPoints(), 1, "dragging a point doesn't add one");

            processor.undo();
            graph.refresh();
            points = p.getBandProfile().getPoints();
            expect (! points.empty() && std::abs (points[0].gain - 6.0f) < 0.3f, "undo puts it back");

            // Right-click deletes it
            const juce::Point<float> backAt { xFor1k, yFor6dB };
            graph.mouseMove (event (graph, backAt, backAt, {}));
            graph.mouseDown (event (graph, backAt, backAt, juce::ModifierKeys::rightButtonModifier));
            graph.mouseUp (event (graph, backAt, backAt, {}));
            expectEquals (p.getNumPoints(), 0);

            // Split: R picks the right ear's tweak, and a click puts the right ear's curve where you click
            p.setPoints ({ { 0, 1000.0f, 2.0f } }); // both ears: +2 dB
            p.setCurveSplit (true);
            graph.refresh();
            graph.keyPressed (juce::KeyPress ('r'));
            press (graph, at);
            release (graph, at);
            expectEquals (p.getNumPoints (BandProfile::rightTweak), 1, "the right ear got the point");
            expectEquals (p.getNumPoints (BandProfile::leftTweak), 0, "the left didn't");
            expectEquals (p.getNumPoints(), 1, "nor did both");
            auto tweak = p.getBandProfile().getPoints (BandProfile::rightTweak);
            expect (! tweak.empty() && std::abs (tweak[0].gain - 4.0f) < 0.2f, "clicking at +6 dB over a +2 dB curve tweaks by +4");
            expectWithinAbsoluteError (p.getBandProfile().curveDbAt (1000.0f, 1), 6.0f, 0.2f);

            // Dragging it down to 0 dB on screen takes the right ear there
            press (graph, at);
            drag (graph, at, { xFor1k, yFor0dB });
            release (graph, { xFor1k, yFor0dB });
            expectWithinAbsoluteError (p.getBandProfile().curveDbAt (1000.0f, 1), 0.0f, 0.3f);

            graph.keyPressed (juce::KeyPress ('b'));
            expectEquals (graph.getNumSelected(), 0, "switching layers starts with nothing selected");
            p.setCurveSplit (false);

            p.setMode (BandProfile::Mode::bands);
            graph.refresh();
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

//==============================================================================
class CalibrationTests : public juce::UnitTest
{
public:
    CalibrationTests() : juce::UnitTest ("Calibration sounds", "CabinEQ") {}

    void runTest() override
    {
        beginTest ("Plays each position in reading order, left to right");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::grid);
            player.setGrid (3, 5);
            player.setLevelDb (-20.0f);
            player.setRate (1.0f / 0.3f);
            player.setPlaying (true);

            // The first burst is the top-left position: all in the left ear
            juce::AudioBuffer<float> buffer (2, (int) (0.25 * sampleRate));
            buffer.clear();
            player.process (buffer);
            expectEquals (player.getCurrentPosition(), 0);
            expectGreaterThan (buffer.getRMSLevel (0, 0, buffer.getNumSamples()), 0.01f);
            expectLessThan (buffer.getRMSLevel (1, 0, buffer.getNumSamples()), 1.0e-4f);

            // A burst every 0.3 s, at that rate
            juce::AudioBuffer<float> more (2, (int) (0.3 * sampleRate));
            more.clear();
            player.process (more);
            expectEquals (player.getCurrentPosition(), 1);

            // Stopping lets the tails die away, then it's silent
            player.setPlaying (false);
            juce::AudioBuffer<float> tail (2, (int) (2.0 * sampleRate));
            tail.clear();
            player.process (tail);
            expectLessThan (tail.getMagnitude (tail.getNumSamples() - 4800, 4800), 1.0e-4f);
        }

        beginTest ("Only selected positions play, in reading order");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::grid);
            player.setGrid (3, 5);
            player.setSelection ({ 2, 7, 12 });
            player.setRate (1.0f / 0.3f);
            player.setPlaying (true);

            std::vector<int> heard;
            juce::AudioBuffer<float> buffer (2, (int) (0.3 * sampleRate));
            for (int burst = 0; burst < 6; ++burst)
            {
                buffer.clear();
                player.process (buffer);
                heard.push_back (player.getCurrentPosition());
            }
            expect (heard == std::vector<int> { 2, 7, 12, 2, 7, 12 });
        }

        beginTest ("Three rows cut 20 Hz to 20 kHz into four sections, one low cut per row");
        {
            expectWithinAbsoluteError (CalibrationPlayer::cutoffForRow (2, 3), 20.0 * std::pow (1000.0, 0.25), 0.01); // ~112 Hz
            expectWithinAbsoluteError (CalibrationPlayer::cutoffForRow (1, 3), 20.0 * std::pow (1000.0, 0.5), 0.01);  // ~632 Hz
            expectWithinAbsoluteError (CalibrationPlayer::cutoffForRow (0, 3), 20.0 * std::pow (1000.0, 0.75), 0.01); // ~3.6 kHz
            expectWithinAbsoluteError (CalibrationPlayer::cutoffForRow (0, 1), 20.0 * std::pow (1000.0, 0.5), 0.01, "one row cuts in the middle");

            // The top row of three has almost nothing below its 3.6 kHz cut
            auto lowEnergyOfRow = [] (int row)
            {
                CalibrationPlayer player;
                player.prepare (sampleRate);
                player.setMode (CalibrationPlayer::Mode::grid);
            player.setGrid (3, 1);
                player.setSelection ({ row });
                player.setPlaying (true);
                juce::AudioBuffer<float> buffer (2, (int) sampleRate);
                buffer.clear();
                player.process (buffer);

                // Steeply low-pass what came out at 200 Hz (48 dB/octave) and see how much is left
                FilterChain lowPass;
                lowPass.prepare (sampleRate);
                std::vector<Band> cuts;
                for (int i = 0; i < 4; ++i)
                    cuts.push_back (Band::withQ (i, 200.0f, 0.0f, 0.7071f, Band::Type::both, Band::Shape::highCut));
                lowPass.setBands (cuts);
                lowPass.reset();
                juce::dsp::AudioBlock<float> block (buffer);
                lowPass.process (block);
                return buffer.getRMSLevel (0, 0, buffer.getNumSamples());
            };
            expectLessThan (lowEnergyOfRow (0) * 50.0f, lowEnergyOfRow (2), "the top row has far less bass than the bottom one");
        }

        beginTest ("Depth plays each position several times, 10 dB louder each time");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::grid);
            player.setGrid (1, 2);
            player.setDepth (3);
            player.setRate (1.0f / 0.3f);
            player.setPlaying (true);

            std::vector<int> positions;
            std::vector<float> levels;
            for (int burst = 0; burst < 4; ++burst)
            {
                juce::AudioBuffer<float> buffer (2, (int) (0.3 * sampleRate));
                buffer.clear();
                player.process (buffer);
                positions.push_back (player.getCurrentPosition());
                levels.push_back (juce::Decibels::gainToDecibels (buffer.getMagnitude (0, (int) (0.02 * sampleRate))));
            }
            expect (positions == std::vector<int> { 0, 0, 0, 1 });
            expectWithinAbsoluteError (levels[1] - levels[0], 10.0f, 3.0f, "the second is about 10 dB louder");
            expectWithinAbsoluteError (levels[2] - levels[1], 10.0f, 3.0f, "and the third another 10 dB");
        }

        beginTest ("Speed sets how often bursts come");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::grid);
            player.setGrid (1, 12);
            player.setRate (4.0f);
            player.setPlaying (true);
            juce::AudioBuffer<float> buffer (2, (int) (2.0 * sampleRate) - 10);
            buffer.clear();
            player.process (buffer);
            expectEquals (player.getCurrentPosition(), 7, "8 bursts in 2 s at 4 per second");
        }

        beginTest ("Spots mode plays each spot in turn, across the pan range");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::spots);
            player.setSpotCount (2);
            player.setSpot (0, 500.0f);
            player.setSpot (1, 3000.0f);
            player.setPanRange (-1.0f, -1.0f); // hard left
            player.setRate (1.0f / 0.3f);
            player.setPlaying (true);

            juce::AudioBuffer<float> buffer (2, (int) (0.25 * sampleRate));
            buffer.clear();
            player.process (buffer);
            expectEquals (player.getCurrentPosition(), 0);
            expectLessThan (buffer.getRMSLevel (1, 0, buffer.getNumSamples()), 1.0e-4f, "spot A is all in the left ear");

            std::vector<int> heard;
            for (int burst = 0; burst < 3; ++burst)
            {
                juce::AudioBuffer<float> more (2, (int) (0.3 * sampleRate));
                more.clear();
                player.process (more);
                heard.push_back (player.getCurrentPosition());
            }
            expect (heard == std::vector<int> { 1, 0, 1 }, "it goes back and forth between the two");
        }

        beginTest ("Pan steps sweep each spot across the range, each with its depth run");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::spots);
            player.setSpotCount (2);
            player.setPanRange (-1.0f, 1.0f);
            player.setPanSteps (3);
            player.setDepth (2);
            player.setRate (1.0f / 0.3f);
            player.setPlaying (true);

            std::vector<int> positions;
            std::vector<float> lefts, rights, levels;
            for (int burst = 0; burst < 7; ++burst)
            {
                juce::AudioBuffer<float> buffer (2, (int) (0.3 * sampleRate));
                buffer.clear();
                player.process (buffer);
                positions.push_back (player.getCurrentPosition());
                const int start = (int) (0.005 * sampleRate), length = (int) (0.02 * sampleRate);
                lefts.push_back (buffer.getRMSLevel (0, start, length));
                rights.push_back (buffer.getRMSLevel (1, start, length));
                levels.push_back (juce::Decibels::gainToDecibels (std::max (lefts.back(), rights.back())));
            }

            expect (positions == std::vector<int> { 0, 0, 0, 0, 0, 0, 1 }, "six bursts for A (3 pans x 2 depths), then B");
            // Look at the loud burst of each depth run (the quiet one is under the last one's tail)
            expectGreaterThan (lefts[1], rights[1] * 10.0f, "it starts on the left");
            expectWithinAbsoluteError (lefts[3] / rights[3], 1.0f, 0.5f, "the middle step is in the centre");
            expectGreaterThan (rights[5], lefts[5] * 3.0f, "and ends on the right");
            expectWithinAbsoluteError (levels[1] - levels[0], 10.0f, 3.0f, "each pan position does its depth run, quiet then loud");
        }

        beginTest ("Each spot plays from its frequency all the way up");
        {
            // How much of the first burst (spot A's) gets through a steep filter at `cut`
            auto measure = [] (float spotA, float spotB, Band::Shape shape, float cut)
            {
                CalibrationPlayer player;
                player.prepare (sampleRate);
                player.setMode (CalibrationPlayer::Mode::spots);
                player.setSpotCount (2);
                player.setSpot (0, spotA);
                player.setSpot (1, spotB);
                player.setRate (0.5f); // just the one burst in the second we listen to
                player.setPlaying (true);
                juce::AudioBuffer<float> buffer (2, (int) sampleRate);
                buffer.clear();
                player.process (buffer);

                FilterChain filter;
                filter.prepare (sampleRate);
                std::vector<Band> cuts;
                for (int i = 0; i < 4; ++i)
                    cuts.push_back (Band::withQ (i, cut, 0.0f, 0.7071f, Band::Type::both, shape));
                filter.setBands (cuts);
                filter.reset();
                juce::dsp::AudioBlock<float> block (buffer);
                filter.process (block);
                return buffer.getRMSLevel (0, 0, buffer.getNumSamples());
            };

            // A at 500 Hz, with B at 3 kHz above it, still has plenty above 6 kHz...
            const float above = measure (500.0f, 3000.0f, Band::Shape::lowCut, 6000.0f);
            expectGreaterThan (above, 1.0e-3f, "it goes past the spot above it");
            // ...and next to nothing below 150 Hz
            expectLessThan (measure (500.0f, 3000.0f, Band::Shape::highCut, 150.0f) * 50.0f, above, "and its low cut still works");
        }

        beginTest ("Clicking a position repeats just it");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::grid);
            player.setGrid (3, 5);
            player.setSelection ({ 14 }); // bottom right
            player.setPlaying (true);
            juce::AudioBuffer<float> buffer (2, (int) (1.0 * sampleRate));
            buffer.clear();
            player.process (buffer);
            expectEquals (player.getCurrentPosition(), 14);
            expectLessThan (buffer.getRMSLevel (0, 0, buffer.getNumSamples()), 1.0e-4f, "the right column is all in the right ear");
        }

        beginTest ("Nothing plays until it's started");
        {
            CabinEqAudioProcessor processor;
            processor.setPlayConfigDetails (2, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);
            juce::AudioBuffer<float> buffer (2, blockSize);
            juce::MidiBuffer midi;
            float peak = 0.0f;
            for (int block = 0; block < 20; ++block)
            {
                buffer.clear();
                processor.processBlock (buffer, midi);
                peak = std::max (peak, buffer.getMagnitude (0, blockSize));
            }
            expectEquals (peak, 0.0f);

            processor.getCalibration().setPlaying (true);
            buffer.clear();
            processor.processBlock (buffer, midi);
            processor.processBlock (buffer, midi);
            expectGreaterThan (buffer.getMagnitude (0, blockSize), 0.0f, "and then it plays, through the processor");
        }
    }
};

//==============================================================================
class CalibrationPanelTests : public juce::UnitTest
{
public:
    CalibrationPanelTests() : juce::UnitTest ("Calibration grid", "CabinEQ") {}

    void runTest() override
    {
        beginTest ("Arrow keys move the selection, Shift+arrow grows it, and the edges stop it");
        CabinEqAudioProcessor processor;
        processor.parameters.state.setProperty (CalibrationSettings::idMode, (int) CalibrationPlayer::Mode::grid, nullptr);
        CalibrationPanel panel (processor);
        panel.setBounds (0, 0, 900, CalibrationPanel::preferredHeight);
        auto& player = processor.getCalibration();
        player.setGrid (3, 5);

        auto press = [&panel] (int key, juce::ModifierKeys mods = {}) { panel.keyPressed (juce::KeyPress (key, mods, 0)); };

        press (juce::KeyPress::rightKey); // nothing selected: starts at the top left
        expect (player.getSelection() == std::set<int> { 0 });
        press (juce::KeyPress::rightKey);
        press (juce::KeyPress::downKey);
        expect (player.getSelection() == std::set<int> { 6 }, "moved right then down");
        press (juce::KeyPress::rightKey, juce::ModifierKeys::shiftModifier);
        expect (player.getSelection() == std::set<int> { 6, 7 }, "shift+right added the next one");
        for (int i = 0; i < 5; ++i)
            press (juce::KeyPress::rightKey);
        expect (player.getSelection() == std::set<int> { 8, 9 }, "the pair moved right until it hit the edge");
        expect (player.isPlaying(), "and it started playing");
        panel.stop();
    }
};

//==============================================================================
class CurveTests : public juce::UnitTest
{
public:
    CurveTests() : juce::UnitTest ("Curve EQ (FIR)", "CabinEQ") {}

    void runTest() override
    {
        beginTest ("The curve goes through every point, never overshoots, and is flat beyond them");
        {
            CurveResponse curve ({ { 0, 100.0f, 6.0f }, { 1, 1000.0f, -3.0f }, { 2, 5000.0f, -3.0f }, { 3, 10000.0f, 4.0f } });
            expectWithinAbsoluteError (curve.dbAtFrequency (100.0f), 6.0f, 1.0e-4f);
            expectWithinAbsoluteError (curve.dbAtFrequency (1000.0f), -3.0f, 1.0e-4f);
            expectWithinAbsoluteError (curve.dbAtFrequency (10000.0f), 4.0f, 1.0e-4f);
            expectWithinAbsoluteError (curve.dbAtFrequency (20.0f), 6.0f, 1.0e-4f, "flat below the first point");
            expectWithinAbsoluteError (curve.dbAtFrequency (20000.0f), 4.0f, 1.0e-4f, "flat above the last");

            float lowest = 100.0f, highest = -100.0f;
            for (float f = 1000.0f; f <= 5000.0f; f *= 1.01f)
            {
                lowest = std::min (lowest, curve.dbAtFrequency (f));
                highest = std::max (highest, curve.dbAtFrequency (f));
            }
            expectWithinAbsoluteError (lowest, -3.0f, 1.0e-3f, "no dip between two equal points");
            expectWithinAbsoluteError (highest, -3.0f, 1.0e-3f, "and no bump");

            for (float f = 100.0f; f <= 1000.0f; f *= 1.01f)
                expect (curve.dbAtFrequency (f) <= 6.0f + 1.0e-3f && curve.dbAtFrequency (f) >= -3.0f - 1.0e-3f, "stays between its neighbours");
        }

        beginTest ("Tracing some bands gives a curve within 0.3 dB of them, with few points");
        {
            BandEqCurve bands;
            bands.updateWithBands ({ Band::withQ (0, 120.0f, 5.0f, 0.7f, Band::Type::both, Band::Shape::lowShelf),
                                     Band::withQ (1, 3000.0f, -4.0f, 2.0f, Band::Type::both) });
            auto points = CurveResponse::tracing ([&bands] (float f) { return bands.dbAtFrequency (f); });
            CurveResponse curve (points);
            float worst = 0.0f;
            for (float f = 20.0f; f <= 20000.0f; f *= 1.02f)
                worst = std::max (worst, std::abs (curve.dbAtFrequency (f) - bands.dbAtFrequency (f)));
            expectLessThan (worst, 0.35f);
            expectLessThan ((int) points.size(), 25, "it didn't need many points");
            expect (CurveResponse::tracing ([] (float) { return 0.0f; }).empty(), "a flat response needs none");
        }

        beginTest ("The FIR filter's response matches the curve");
        {
            CurveResponse curve ({ { 0, 60.0f, 8.0f }, { 1, 300.0f, 0.0f }, { 2, 2500.0f, -6.0f }, { 3, 8000.0f, 3.0f } });
            const int length = CurveFilter::lengthFor (sampleRate);
            auto impulse = CurveFilter::design ([&curve] (float f) { return curve.dbAtFrequency (f); }, sampleRate, length);
            expectEquals (impulse.getNumSamples(), 16384);

            // Its spectrum, from a zero-padded FFT
            const int order = 16, size = 1 << order;
            juce::dsp::FFT fft (order);
            std::vector<float> data ((size_t) size * 2, 0.0f);
            std::copy_n (impulse.getReadPointer (0), length, data.begin());
            fft.performFrequencyOnlyForwardTransform (data.data());

            float worst = 0.0f;
            for (float f = 30.0f; f <= 16000.0f; f *= 1.05f)
            {
                const int bin = juce::roundToInt (f * size / sampleRate);
                const float measured = juce::Decibels::gainToDecibels (data[(size_t) bin]);
                worst = std::max (worst, std::abs (measured - curve.dbAtFrequency ((float) bin * (float) sampleRate / (float) size)));
            }
            expectLessThan (worst, 0.5f);

            // Minimum phase: nearly all its energy comes right at the start, so it adds hardly any delay
            double total = 0.0, early = 0.0;
            for (int n = 0; n < length; ++n)
            {
                const double energy = impulse.getSample (0, n) * impulse.getSample (0, n);
                total += energy;
                if (n < (int) (0.005 * sampleRate))
                    early += energy;
            }
            expectGreaterThan (early / total, 0.95, "95% of it is in the first 5 ms");
        }

        beginTest ("Split ears: each ear gets its own curve, with the same timing in both");
        {
            CurveResponse left ({ { 0, 100.0f, 0.0f }, { 1, 1000.0f, 6.0f }, { 2, 10000.0f, 0.0f } });
            CurveResponse right ({ { 0, 100.0f, 0.0f }, { 1, 1500.0f, -4.0f }, { 2, 10000.0f, 2.0f } });
            auto impulse = CurveFilter::designSplit ([&] (float f) { return left.dbAtFrequency (f); },
                                                     [&] (float f) { return right.dbAtFrequency (f); }, sampleRate);
            expectEquals (impulse.getNumChannels(), 2);

            const int order = 16, size = 1 << order;
            juce::dsp::FFT fft (order);
            auto spectrum = [&] (int channel)
            {
                std::vector<std::complex<float>> time ((size_t) size), frequency ((size_t) size);
                for (int n = 0; n < impulse.getNumSamples(); ++n)
                    time[(size_t) n] = impulse.getSample (channel, n);
                fft.perform (time.data(), frequency.data(), false);
                return frequency;
            };
            const auto l = spectrum (0), r = spectrum (1);

            float worstLevel = 0.0f, worstPhase = 0.0f;
            for (float f = 300.0f; f <= 16000.0f; f *= 1.05f)
            {
                const int bin = juce::roundToInt (f * size / sampleRate);
                const float at = (float) bin * (float) sampleRate / (float) size;
                worstLevel = std::max (worstLevel, std::abs (juce::Decibels::gainToDecibels (std::abs (l[(size_t) bin])) - left.dbAtFrequency (at)));
                worstLevel = std::max (worstLevel, std::abs (juce::Decibels::gainToDecibels (std::abs (r[(size_t) bin])) - right.dbAtFrequency (at)));
                worstPhase = std::max (worstPhase, std::abs (std::arg (l[(size_t) bin] * std::conj (r[(size_t) bin]))));
            }
            expectLessThan (worstLevel, 0.5f, "each ear matches its curve");
            expectLessThan (worstPhase, 0.02f, "and the two ears' phase is the same, so the timing between them is untouched");

            // A delay of about 11 ms, the same for both ears
            const int expectedDelay = CurveFilter::splitLengthFor (sampleRate) / 2;
            expectWithinAbsoluteError (expectedDelay, 512, 0);
        }

        beginTest ("Splitting a curve adds a tweak for each ear on top of the shared curve; joining keeps the shared curve");
        {
            CabinEqAudioProcessor processor;
            auto profile = processor.getSelectedProfile();
            profile.setPoints ({ { 0, 200.0f, 3.0f }, { 1, 4000.0f, -2.0f } });
            profile.setCurveSplit (true);
            auto split = profile.getBandProfile();
            expect (split.isSplit());
            expectEquals (profile.getNumPoints (BandProfile::leftTweak), 0, "the ears start with no tweaks");
            expectWithinAbsoluteError (split.curveDbAt (200.0f, 0), 3.0f, 0.01f, "so they both hear the shared curve");

            profile.addPoint ({ 0, 4000.0f, 5.0f }, BandProfile::rightTweak);
            split = profile.getBandProfile();
            expectWithinAbsoluteError (split.curveDbAt (4000.0f, 1), 3.0f, 0.01f, "the right ear hears both plus its tweak");
            expectWithinAbsoluteError (split.curveDbAt (4000.0f, 0), -2.0f, 0.01f, "the left doesn't");

            // Editing the shared curve moves both ears, keeping the tweak
            profile.updatePoint ({ 1, 4000.0f, 0.0f });
            split = profile.getBandProfile();
            expectWithinAbsoluteError (split.curveDbAt (4000.0f, 1), 5.0f, 0.01f);
            expectWithinAbsoluteError (split.curveDbAt (4000.0f, 0), 0.0f, 0.01f);

            // It round-trips through saved state
            auto copy = CabinEqProfile::createTree ("Copy", profile.getBandProfile());
            expectEquals (CabinEqProfile (copy, nullptr).getNumPoints (BandProfile::rightTweak), 1);

            processor.getUndoManager().beginNewTransaction();
            profile.setCurveSplit (false);
            expect (! profile.getBandProfile().isSplit());
            expectEquals (profile.getNumPoints(), 2, "joined: the shared curve stays");
            expectWithinAbsoluteError (profile.getBandProfile().curveDbAt (4000.0f, 1), 0.0f, 0.01f, "and the tweaks go");
            processor.undo();
            expectEquals (profile.getNumPoints (BandProfile::rightTweak), 1, "undo brings the tweak back");
        }

        beginTest ("A split curve plays each ear its own gain through the processor");
        {
            CabinEqAudioProcessor processor;
            processor.parameters.getParameter (ParamIDs::autoGain)->setValueNotifyingHost (0.0f);
            processor.setPlayConfigDetails (2, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);

            auto profile = processor.getSelectedProfile();
            profile.setMode (BandProfile::Mode::curve);
            profile.setPoints ({ { 0, 1000.0f, 1.0f } });
            profile.setCurveSplit (true);
            profile.setPoints ({ { 0, 1000.0f, 5.0f } }, BandProfile::leftTweak);
            profile.setPoints ({ { 0, 1000.0f, -5.0f } }, BandProfile::rightTweak);

            auto gains = [&processor] (int blocks)
            {
                juce::AudioBuffer<float> buffer (2, blockSize);
                juce::MidiBuffer midi;
                double phase = 0.0, in = 0.0, outL = 0.0, outR = 0.0;
                for (int block = 0; block < blocks; ++block)
                {
                    for (int i = 0; i < blockSize; ++i)
                    {
                        const auto sample = (float) (0.1 * std::sin (phase));
                        phase += juce::MathConstants<double>::twoPi * 1000.0 / sampleRate;
                        buffer.setSample (0, i, sample);
                        buffer.setSample (1, i, sample);
                        if (block >= blocks / 2) in += sample * sample;
                    }
                    processor.processBlock (buffer, midi);
                    if (block >= blocks / 2)
                        for (int i = 0; i < blockSize; ++i)
                        {
                            outL += buffer.getSample (0, i) * buffer.getSample (0, i);
                            outR += buffer.getSample (1, i) * buffer.getSample (1, i);
                        }
                }
                return std::pair { (float) (10.0 * std::log10 (outL / in)), (float) (10.0 * std::log10 (outR / in)) };
            };

            juce::Thread::sleep (300);
            gains (40);
            juce::Thread::sleep (300);
            auto [leftGain, rightGain] = gains (200);
            expectWithinAbsoluteError (leftGain, 6.0f, 0.3f);
            expectWithinAbsoluteError (rightGain, -4.0f, 0.3f);
        }

        beginTest ("A curve profile plays through the processor, and switching back to bands undoes it");
        {
            CabinEqAudioProcessor processor;
            processor.parameters.getParameter (ParamIDs::autoGain)->setValueNotifyingHost (0.0f);
            processor.setPlayConfigDetails (2, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);

            auto profile = processor.getSelectedProfile();
            profile.setPoints ({ { 0, 1000.0f, 6.0f } }); // one point: +6 dB everywhere
            profile.setMode (BandProfile::Mode::curve);

            auto gainAt1k = [&processor] (int blocks)
            {
                juce::AudioBuffer<float> buffer (2, blockSize);
                juce::MidiBuffer midi;
                double phase = 0.0, in = 0.0, out = 0.0;
                for (int block = 0; block < blocks; ++block)
                {
                    for (int i = 0; i < blockSize; ++i)
                    {
                        const auto sample = (float) (0.1 * std::sin (phase));
                        phase += juce::MathConstants<double>::twoPi * 1000.0 / sampleRate;
                        buffer.setSample (0, i, sample);
                        buffer.setSample (1, i, sample);
                        if (block >= blocks / 2) in += sample * sample;
                    }
                    processor.processBlock (buffer, midi);
                    if (block >= blocks / 2)
                        for (int i = 0; i < blockSize; ++i)
                            out += buffer.getSample (0, i) * buffer.getSample (0, i);
                }
                return (float) (10.0 * std::log10 (out / in));
            };

            // The design happens in the background; give it a moment
            juce::Thread::sleep (300);
            gainAt1k (40);
            juce::Thread::sleep (300);
            expectWithinAbsoluteError (gainAt1k (200), 6.0f, 0.3f);

            profile.setMode (BandProfile::Mode::bands);
            juce::Thread::sleep (300);
            gainAt1k (40);
            juce::Thread::sleep (300);
            expectWithinAbsoluteError (gainAt1k (200), 0.0f, 0.1f, "back to bands (there are none)");
        }

        beginTest ("Dragging a point changes the sound smoothly, without clicks");
        {
            CurveFilter filter;
            filter.setCurve (std::vector<CurvePoint> { { 0, 100.0f, 0.0f } });
            filter.prepare ({ sampleRate, (juce::uint32) blockSize, 2 });

            juce::AudioBuffer<float> buffer (2, blockSize);
            double phase = 0.0;
            float last = 0.0f, maxStep = 0.0f;
            for (int block = 0; block < 400; ++block)
            {
                // Drag a point from 0 to +12 dB over about a second, a new position every other block
                if (block % 2 == 0 && block < 200)
                    filter.setCurve (std::vector<CurvePoint> { { 0, 100.0f, 12.0f * (float) block / 200.0f } });

                for (int i = 0; i < blockSize; ++i)
                {
                    const auto sample = (float) (0.05 * std::sin (phase));
                    phase += juce::MathConstants<double>::twoPi * 60.0 / sampleRate;
                    buffer.setSample (0, i, sample);
                    buffer.setSample (1, i, sample);
                }
                juce::dsp::AudioBlock<float> block2 (buffer);
                filter.process (block2);
                juce::Thread::sleep (1); // let the designer keep up, as it would in real time

                for (int i = 0; i < blockSize; ++i)
                {
                    const float sample = buffer.getSample (0, i);
                    if (block > 4)
                        maxStep = std::max (maxStep, std::abs (sample - last));
                    last = sample;
                }
            }
            // A 60 Hz sine at up to about 0.2 moves at most ~0.0016 a sample; a click would be far more
            expectLessThan (maxStep, 0.01f);
        }
    }
};

static FilterResponseTests filterResponseTests;
static CurveTests curveTests;
static CalibrationPanelTests calibrationPanelTests;
static CalibrationTests calibrationTests;
static GraphInteractionTests graphInteractionTests;
static SmoothingTests smoothingTests;
static ProcessorStateTests processorStateTests;
static ProcessorAudioTests processorAudioTests;
static PresetFileTests presetFileTests;

//==============================================================================
/// Renders the editor with a demo profile to a PNG, to check the UI without clicking around.
static int writeSnapshot (const juce::File& file, int width, int height, bool channelSpecific, bool showCalibration, bool spotsMode, bool zoomed, bool curveMode, bool split)
{
    CabinEqAudioProcessor processor;
    auto profile = processor.getSelectedProfile();
    profile.setBands ({ Band::withQ (0, 32.0f, 0.0f, 0.71f, Band::Type::both, Band::Shape::lowCut),
                        Band::withQ (0, 105.0f, 5.5f, 0.7f, Band::Type::both, Band::Shape::lowShelf),
                        Band::withQ (0, 2300.0f, -3.2f, 1.9f, channelSpecific ? Band::Type::left : Band::Type::both),
                        Band::withQ (0, 5400.0f, 4.0f, 3.0f, Band::Type::both),
                        Band::withQ (0, 9800.0f, -2.5f, 0.7f, Band::Type::both, Band::Shape::highShelf) });
    profile.setVolume (-6.0f);
    if (curveMode)
    {
        BandEqCurve bands;
        bands.updateWithBands (profile.getBandProfile().getBands());
        profile.setPoints (CurveResponse::tracing ([&bands] (float f) { return bands.dbAtFrequency (f); }));
        profile.setMode (BandProfile::Mode::curve);
        if (split)
        {
            profile.setCurveSplit (true);
            profile.setPoints ({ { 0, 1500.0f, 0.0f }, { 1, 3000.0f, -2.5f }, { 2, 12000.0f, -2.0f } }, BandProfile::rightTweak);
            profile.setPoints ({ { 0, 4000.0f, 0.0f }, { 1, 6500.0f, 1.5f }, { 2, 9000.0f, 0.0f } }, BandProfile::leftTweak);
            processor.parameters.state.setProperty (CabinPeqGraph::idCurveLayer, (int) BandProfile::rightTweak, nullptr);
        }
    }
    processor.getProfiles().addProfile ("HD 600 (AutoEQ)");
    processor.parameters.state.setProperty ("showCalibration", showCalibration, nullptr);
    if (zoomed)
    {
        processor.parameters.state.setProperty ("graphLowFrequency", 800.0f, nullptr);
        processor.parameters.state.setProperty ("graphHighFrequency", 1500.0f, nullptr);
    }
    if (spotsMode)
    {
        processor.parameters.state.setProperty ("calibrationMode", 1, nullptr);
        processor.parameters.state.setProperty ("calibrationSpots", 3, nullptr);
        processor.parameters.state.setProperty ("calibrationSpot0Pan", -0.6f, nullptr);
    }
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

/// Loads a real CabinEQ.settings (from the standalone app) and reports what came through.
static int checkImport (const juce::File& settingsFile)
{
    CabinEqProfileManager::shouldBackUpOldState = false;
    juce::PropertiesFile settings (settingsFile, {});
    juce::MemoryBlock state;
    if (! state.fromBase64Encoding (settings.getValue ("filterState")))
    {
        std::cout << "No filterState in " << settingsFile.getFullPathName() << std::endl;
        return 1;
    }

    // How many bands each old profile had, counting every step
    auto oldXml = juce::AudioProcessor::getXmlFromBinary (state.getData(), (int) state.getSize());
    std::map<juce::String, int> oldCounts;
    for (auto* profile : oldXml->getChildWithTagNameIterator ("Profile"))
    {
        int count = 0;
        for (auto* amplTree : profile->getChildWithTagNameIterator ("AmplTree"))
            for (auto* step : amplTree->getChildWithTagNameIterator ("MultiBandStep"))
                count += step->getNumChildElements();
        oldCounts[profile->getStringAttribute ("ProfileName")] = count;
    }

    CabinEqAudioProcessor processor;
    const auto start = juce::Time::getMillisecondCounterHiRes();
    processor.setStateInformation (state.getData(), (int) state.getSize());
    const auto loadMs = juce::Time::getMillisecondCounterHiRes() - start;

    auto& profiles = processor.getProfiles();
    int mismatches = 0;
    for (const auto& [name, count] : oldCounts)
    {
        auto profile = profiles.getProfileNamed (name);
        if (! profile.has_value() || profile->getNumBands() != count)
        {
            std::cout << "  MISMATCH " << name << ": had " << count << ", now "
                      << (profile.has_value() ? juce::String (profile->getNumBands()) : juce::String ("missing")) << std::endl;
            ++mismatches;
        }
    }

    juce::MemoryBlock saved;
    const auto saveStart = juce::Time::getMillisecondCounterHiRes();
    processor.getStateInformation (saved);
    const auto saveMs = juce::Time::getMillisecondCounterHiRes() - saveStart;

    std::cout << oldCounts.size() << " old profiles, " << profiles.getProfileNames().size() << " after loading, "
              << mismatches << " with a different number of bands. Selected: " << profiles.getSelectedProfileName() << "\n"
              << "Loading took " << juce::String (loadMs, 1) << " ms, saving " << juce::String (saveMs, 1) << " ms ("
              << (int) saved.getSize() / 1024 << " KB)" << std::endl;
    return mismatches == 0 ? 0 : 1;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juce;

    if (argc >= 3 && juce::String (argv[1]) == "--check-import")
        return checkImport (juce::File (juce::String (argv[2])));

    if (argc >= 3 && juce::String (argv[1]) == "--snapshot")
        return writeSnapshot (juce::File (argv[2]), argc >= 5 ? juce::String (argv[3]).getIntValue() : 1080,
                              argc >= 5 ? juce::String (argv[4]).getIntValue() : 680,
                              argc >= 6 && juce::String (argv[5]).contains ("lr"), argc >= 6 && juce::String (argv[5]).contains ("calibration"),
                              argc >= 6 && juce::String (argv[5]).contains ("spots"),
                              argc >= 6 && juce::String (argv[5]).contains ("zoom"),
                              argc >= 6 && juce::String (argv[5]).contains ("curve"),
                              argc >= 6 && juce::String (argv[5]).contains ("split"));

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
