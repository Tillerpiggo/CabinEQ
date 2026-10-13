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
#include <set>
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
            expect (profiles.getSelectedProfile().getCurveOn() && ! profiles.getSelectedProfile().getBandsOn(), "a new profile is a curve");
            auto empty = profiles.addProfile ("Empty");
            expect (empty.getCurveOn() && ! empty.getBandsOn());
            auto imported = profiles.addProfile ("Imported", BandProfile ({ Band::withQ (0, 500.0f, 3.0f, 1.0f, Band::Type::both) }, 0.0f));
            expect (imported.getBandsOn() && ! imported.getCurveOn(), "an import is bands");
            juce::ValueTree saved (CabinEqProfile::idProfile);
            saved.setProperty (CabinEqProfile::idProfileName, "Saved before curves", nullptr);
            expect (CabinEqProfile (saved, nullptr).getBandsOn() && ! CabinEqProfile (saved, nullptr).getCurveOn(), "a profile saved before curves is bands");
        }

        beginTest ("Undo and redo, and undo selects the profile it changed");
        {
            CabinEqAudioProcessor processor;
            auto& profiles = processor.getProfiles();
            auto& undo = processor.getUndoManager();

            auto first = profiles.getSelectedProfile();
            first.setBandsOn (true), first.setCurveOn (false);
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

        beginTest ("Master volume boosts, even bypassed, and nothing limits or compresses it");
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
            expectWithinAbsoluteError (juce::Decibels::gainToDecibels (run (0.8f) / 0.8f), 12.0f, 0.05f, "a loud one gets exactly the same 12 dB, past full scale");
            expectGreaterThan (run (0.8f), 3.0f, "nothing holds it under full scale");

            processor.parameters.getParameter (ParamIDs::bypass)->setValueNotifyingHost (1.0f);
            expectWithinAbsoluteError (juce::Decibels::gainToDecibels (run (0.01f) / 0.01f), 12.0f, 0.2f, "and it still applies with the EQ off");
        }

        beginTest ("Mono plays the average of left and right in both ears, with the EQ on or off");
        {
            CabinEqAudioProcessor processor;
            processor.setPlayConfigDetails (2, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);
            processor.parameters.getParameter (ParamIDs::autoGain)->setValueNotifyingHost (0.0f);

            // A sine in the left ear only; returns each ear's level relative to it, and the most the ears differ
            auto run = [&processor]
            {
                juce::MidiBuffer midi;
                juce::AudioBuffer<float> buffer (2, blockSize);
                double phase = 0.0, in = 0.0, left = 0.0, right = 0.0;
                float difference = 0.0f;
                for (int block = 0; block < 60; ++block)
                {
                    buffer.clear();
                    for (int i = 0; i < blockSize; ++i)
                    {
                        const auto sample = (float) (0.2 * std::sin (phase));
                        phase += juce::MathConstants<double>::twoPi * 1000.0 / sampleRate;
                        buffer.setSample (0, i, sample);
                        if (block >= 30) in += sample * sample;
                    }
                    processor.processBlock (buffer, midi);
                    if (block >= 30)
                        for (int i = 0; i < blockSize; ++i)
                        {
                            left += buffer.getSample (0, i) * buffer.getSample (0, i);
                            right += buffer.getSample (1, i) * buffer.getSample (1, i);
                            difference = std::max (difference, std::abs (buffer.getSample (0, i) - buffer.getSample (1, i)));
                        }
                }
                return std::tuple { (float) std::sqrt (left / in), (float) std::sqrt (right / in), difference };
            };

            auto [stereoLeft, stereoRight, stereoDifference] = run();
            expectWithinAbsoluteError (stereoLeft, 1.0f, 0.01f, "stereo: the left is as it came in");
            expectWithinAbsoluteError (stereoRight, 0.0f, 0.001f, "and the right is silent");

            processor.parameters.getParameter (ParamIDs::mono)->setValueNotifyingHost (1.0f);
            auto [monoLeft, monoRight, monoDifference] = run();
            expectWithinAbsoluteError (monoLeft, 0.5f, 0.01f, "mono: half of it in the left");
            expectWithinAbsoluteError (monoRight, 0.5f, 0.01f, "and half in the right");
            expectLessThan (monoDifference, 1.0e-5f, "sample for sample the same");

            processor.parameters.getParameter (ParamIDs::bypass)->setValueNotifyingHost (1.0f);
            auto [bypassedLeft, bypassedRight, bypassedDifference] = run();
            expectWithinAbsoluteError (bypassedRight, 0.5f, 0.01f, "still mono with the EQ off");
            expectLessThan (bypassedDifference, 1.0e-5f);
            juce::ignoreUnused (stereoDifference, bypassedLeft);
        }

        beginTest ("Auto gain cancels a boost");
        {
            CabinEqAudioProcessor processor;
            processor.getSelectedProfile().setBandsOn (true), processor.getSelectedProfile().setCurveOn (false);
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
        profile.getSelectedProfile().setBandsOn (true), profile.getSelectedProfile().setCurveOn (false); // a new profile starts as a curve
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

        beginTest ("In curve mode, points come out of the 0 dB line; elsewhere, dragging selects, and selected points drag together");
        {
            auto p = profile.getSelectedProfile();
            p.setBandsOn (false), p.setCurveOn (true);
            graph.refresh();

            // Where the plot puts things, as in the band tests
            auto xFor = [] (float hz) { return std::log (hz / 20.0f) / std::log (1000.0f) * 1000.0f; };
            auto yFor = [] (float db) { return 14.0f + (30.0f - db) / 60.0f * (478.0f - 28.0f); };
            const float xFor1k = xFor (1000.0f);
            const juce::Point<float> onLine { xFor1k, yFor (0.0f) + 3.0f }, at { xFor1k, yFor (6.0f) };

            // Pull one out of the line
            press (graph, onLine);
            drag (graph, onLine, at.translated (0.0f, 3.0f));
            release (graph, at);
            expectEquals (p.getNumPoints(), 1);
            expectEquals (p.getNumBands(), 0, "no band was added");
            auto points = p.getBandProfile().getPoints();
            if (! points.empty())
            {
                expectWithinAbsoluteError (points[0].freq, 1000.0f, 15.0f);
                expectWithinAbsoluteError (points[0].gain, 6.0f, 0.2f);
            }
            processor.undo();
            graph.refresh();
            expectEquals (p.getNumPoints(), 0, "adding and dragging undo as one step");
            processor.redo();
            graph.refresh();

            // Clicking away from the line doesn't add one; it clears the selection
            press (graph, { xFor (200.0f), yFor (15.0f) });
            release (graph, { xFor (200.0f), yFor (15.0f) });
            expectEquals (p.getNumPoints(), 1, "clicking empty space adds nothing");
            expectEquals (graph.getNumSelected(), 0);

            // Right-click deletes it
            graph.mouseMove (event (graph, at, at, {}));
            graph.mouseDown (event (graph, at, at, juce::ModifierKeys::rightButtonModifier));
            graph.mouseUp (event (graph, at, at, {}));
            expectEquals (p.getNumPoints(), 0);

            // Drag a box around two points, then drag one: both move
            p.setPoints ({ { 0, 300.0f, 6.0f }, { 1, 3000.0f, 9.0f }, { 2, 10000.0f, -3.0f } });
            graph.refresh();
            const juce::Point<float> boxFrom { xFor (200.0f), yFor (15.0f) }, boxTo { xFor (5000.0f), yFor (3.0f) };
            press (graph, boxFrom);
            drag (graph, boxFrom, boxTo);
            release (graph, boxTo);
            expectEquals (graph.getNumSelected(), 2, "the box selected the two inside it");

            const juce::Point<float> grab { xFor (300.0f), yFor (6.0f) }, dropAt { xFor (300.0f), yFor (0.0f) };
            press (graph, grab);
            drag (graph, grab, dropAt);
            release (graph, dropAt);
            points = p.getBandProfile().getPoints();
            expectEquals ((int) points.size(), 3, "dragging points doesn't add any");
            expectWithinAbsoluteError (points[0].gain, 0.0f, 0.3f, "the one dragged moved down 6 dB");
            expectWithinAbsoluteError (points[1].gain, 3.0f, 0.3f, "and so did the other selected one");
            expectWithinAbsoluteError (points[2].gain, -3.0f, 0.01f, "the unselected one stayed");

            // Experimental: Shift-drag a point to swing the points on its side about it
            p.setPoints ({ { 0, 50.0f, 0.0f }, { 1, 100.0f, 0.0f }, { 2, 200.0f, 2.0f }, { 3, 2000.0f, 1.0f }, { 4, 8000.0f, 1.0f } });
            graph.refresh();
            const auto shift = juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier | juce::ModifierKeys::shiftModifier);
            auto shiftDrag = [&] (juce::Point<float> from, float pixelsUp)
            {
                graph.mouseMove (event (graph, from, from, {}));
                graph.mouseDown (event (graph, from, from, shift));
                for (int step = 1; step <= 10; ++step)
                    graph.mouseDrag (event (graph, from.translated (0.0f, -pixelsUp * (float) step / 10.0f), from, shift));
                graph.mouseUp (event (graph, from.translated (0.0f, -pixelsUp), from, {}));
            };

            // 200 Hz is left of the middle of the graph, so its left side swings: 75 px up is 10 dB at the edge
            shiftDrag ({ xFor (200.0f), yFor (2.0f) }, 75.0f);
            points = p.getBandProfile().getPoints();
            const float arm = std::log2 (200.0f / 20.0f);
            expectWithinAbsoluteError (points[0].gain, 10.0f * 2.0f / arm, 0.25f, "two octaves away: most of the way");
            expectWithinAbsoluteError (points[1].gain, 10.0f * 1.0f / arm, 0.25f, "one octave away: half as far");
            expectWithinAbsoluteError (points[2].gain, 2.0f, 0.001f, "the point itself stays where it is");
            expectWithinAbsoluteError (points[3].gain, 1.0f, 0.001f, "and so does the other side");
            processor.undo();
            graph.refresh();
            expectWithinAbsoluteError (p.getBandProfile().getPoints()[0].gain, 0.0f, 0.001f, "one undo puts it all back");

            // 2 kHz is right of the middle, so its right side swings (down, here)
            shiftDrag ({ xFor (2000.0f), yFor (1.0f) }, -75.0f);
            points = p.getBandProfile().getPoints();
            expectWithinAbsoluteError (points[4].gain, 1.0f - 10.0f * 2.0f / std::log2 (20000.0f / 2000.0f), 0.25f);
            expectWithinAbsoluteError (points[1].gain, 0.0f, 0.001f, "the left side stays");

            // A Shift-click without a drag still just selects
            graph.keyPressed (juce::KeyPress (juce::KeyPress::escapeKey));
            const int before = graph.getNumSelected();
            shiftDrag ({ xFor (200.0f), yFor (2.0f) }, 0.0f);
            expectEquals (graph.getNumSelected(), before + 1, "Shift-click adds it to the selection");
            expectWithinAbsoluteError (p.getBandProfile().getPoints()[2].gain, 2.0f, 0.001f);

            // Split: R picks the right ear's tweak, whose centre line is the shared curve
            p.setPoints ({ { 0, 1000.0f, 2.0f } }); // both ears: +2 dB
            p.setCurveSplit (true);
            graph.refresh();
            graph.keyPressed (juce::KeyPress ('r'));
            const juce::Point<float> onShared { xFor1k, yFor (2.0f) + 3.0f };
            press (graph, onShared);
            drag (graph, onShared, at.translated (0.0f, 3.0f));
            release (graph, at);
            expectEquals (p.getNumPoints (BandProfile::rightTweak), 1, "the right ear got the point");
            expectEquals (p.getNumPoints (BandProfile::leftTweak), 0, "the left didn't");
            expectEquals (p.getNumPoints(), 1, "nor did both");
            auto tweak = p.getBandProfile().getPoints (BandProfile::rightTweak);
            expect (! tweak.empty() && std::abs (tweak[0].gain - 4.0f) < 0.2f, "pulled up to +6 dB over a +2 dB curve: a +4 dB tweak");
            expectWithinAbsoluteError (p.getBandProfile().curveDbAt (1000.0f, 1), 6.0f, 0.2f);
            expectWithinAbsoluteError (p.getBandProfile().curveDbAt (1000.0f, 0), 2.0f, 0.01f);

            graph.keyPressed (juce::KeyPress ('b'));
            expectEquals (graph.getNumSelected(), 0, "switching layers starts with nothing selected");
            p.setCurveSplit (false);

            // Bands underneath: the curve sits on them, so you drag what you hear
            p.setPoints ({});
            p.setBands ({ Band::withQ (0, 1000.0f, 6.0f, 0.3f, Band::Type::both) }); // about +6 dB around 1 kHz
            p.setBandsOn (true);
            processor.setEditingCurve (true);
            graph.refresh();
            const float bandsAt1k = graph.layerBaseDb (1000.0f);
            expectWithinAbsoluteError (bandsAt1k, 6.0f, 0.2f, "the curve's centre line is the bands");

            press (graph, { xFor1k, yFor (0.0f) });
            release (graph, { xFor1k, yFor (0.0f) });
            expectEquals (p.getNumPoints(), 0, "the 0 dB line isn't where points come from any more");

            const juce::Point<float> onBands { xFor1k, yFor (bandsAt1k) + 2.0f }, upTo { xFor1k, yFor (9.0f) };
            press (graph, onBands);
            drag (graph, onBands, upTo.translated (0.0f, 2.0f));
            release (graph, upTo);
            auto onTop = p.getBandProfile().getPoints();
            expectEquals ((int) onTop.size(), 1);
            expect (! onTop.empty() && std::abs (onTop[0].gain - 3.0f) < 0.3f, "dragged to +9 dB on +6 dB of bands: the curve adds +3");

            // Tab swaps to the bands, whose handles are then the ones you can grab
            graph.keyPressed (juce::KeyPress (juce::KeyPress::tabKey));
            expect (! processor.isEditingCurve(), "Tab swaps to the bands");
            graph.keyPressed (juce::KeyPress (juce::KeyPress::tabKey));
            expect (processor.isEditingCurve(), "and back");
            p.setPoints ({});
            p.setBandsOn (false);
            p.setBands ({});

            p.setBandsOn (true), p.setCurveOn (false);
            graph.refresh();
        }

        beginTest ("With three points selected, two pads shape the middle one, and how it differs between the ears");
        {
            auto p = profile.getSelectedProfile();
            p.setBands ({});
            p.setBandsOn (false), p.setCurveOn (true);
            p.setPoints ({ { 0, 500.0f, 0.0f }, { 1, 1000.0f, 3.0f }, { 2, 4000.0f, 0.0f }, { 3, 12000.0f, -2.0f } });
            graph.refresh();

            CurveShapePanel* pads = nullptr;
            for (auto* child : graph.getChildren())
                if (auto* found = dynamic_cast<CurveShapePanel*> (child))
                    pads = found;
            expect (pads != nullptr);
            if (pads == nullptr)
                return;

            // Select the first three by dragging a box round them
            auto xFor = [] (float hz) { return std::log (hz / 20.0f) / std::log (1000.0f) * 1000.0f; };
            auto yFor = [] (float db) { return 14.0f + (30.0f - db) / 60.0f * (478.0f - 28.0f); };
            expect (! pads->isVisible(), "no pads with nothing selected");
            press (graph, { xFor (400.0f), yFor (12.0f) });
            drag (graph, { xFor (400.0f), yFor (12.0f) }, { xFor (5000.0f), yFor (-12.0f) });
            release (graph, { xFor (5000.0f), yFor (-12.0f) });
            expectEquals (graph.getNumSelected(), 3);
            expect (pads->isVisible(), "three selected: the pads are there");

            auto middle = [&p] { return *p.getPoint (1); };

            // Frequency pad: across is where it is between the outer two (in octaves); up makes it higher in the left ear
            pads->frequencyPad.onGestureStart();
            pads->frequencyPad.onChange ({ 0.5f, 0.5f });
            expectWithinAbsoluteError (middle().freq, std::sqrt (500.0f * 4000.0f), 5.0f, "halfway across is halfway between them, in octaves");
            expectWithinAbsoluteError (middle().freqSplit, 0.0f, 0.001f, "and the middle of the pad is the same in both ears");
            pads->frequencyPad.onChange ({ 0.5f, 0.75f });
            expectWithinAbsoluteError (middle().freqSplit, 0.5f, 0.01f, "three quarters up: half an octave higher in the left ear");
            pads->frequencyPad.onChange ({ 0.0f, 1.0f });
            expectLessThan (middle().freq, 540.0f, "all the way left is just above the lower point");
            expectGreaterThan (middle().forEar (1).freq, 500.0f, "and neither ear's version goes past it");
            expectLessThan (middle().forEar (0).freq, 4000.0f);
            expectWithinAbsoluteError (p.getPoint (0)->freq, 500.0f, 0.001f, "the outer two don't move");
            expectWithinAbsoluteError (p.getPoint (2)->freq, 4000.0f, 0.001f);

            // Gain pad: up is its gain; left makes it louder in the left ear
            pads->gainPad.onGestureStart();
            pads->gainPad.onChange ({ 0.5f, 0.75f });
            expectWithinAbsoluteError (middle().gain, 12.0f, 0.05f, "three quarters up is +12 dB");
            expectWithinAbsoluteError (middle().gainSplit, 0.0f, 0.001f);
            pads->gainPad.onChange ({ 0.25f, 0.5f });
            expectWithinAbsoluteError (middle().gain, 0.0f, 0.05f, "the middle is 0 dB");
            expectWithinAbsoluteError (middle().gainSplit, 6.0f, 0.05f, "a quarter across: 6 dB louder in the left ear");
            expectWithinAbsoluteError (middle().forEar (0).gain, 3.0f, 0.05f, "half of it each way");
            expectWithinAbsoluteError (middle().forEar (1).gain, -3.0f, 0.05f);

            // Double-click makes the ears the same again; undo brings the difference back
            pads->gainPad.onReset();
            expectWithinAbsoluteError (middle().gainSplit, 0.0f, 0.001f);
            processor.undo();
            graph.refresh();
            expectWithinAbsoluteError (middle().gainSplit, 6.0f, 0.05f);

            // Dragging the point on the graph keeps its difference
            graph.keyPressed (juce::KeyPress (juce::KeyPress::upKey));
            expectWithinAbsoluteError (middle().gainSplit, 6.0f, 0.05f, "nudging it keeps how the ears differ");

            // A fourth point selected, or one fewer: no pads
            graph.keyPressed (juce::KeyPress ('a', juce::ModifierKeys::commandModifier, 0));
            expect (! pads->isVisible(), "four selected: no pads");

            p.setPoints ({});
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

        beginTest ("The heights can be spaced in octaves, mels or Hz, between the same 20 Hz and 20 kHz");
        {
            using Spacing = CalibrationPlayer::HeightSpacing;
            for (auto spacing : { Spacing::octaves, Spacing::mels, Spacing::hertz })
            {
                expectWithinAbsoluteError (CalibrationPlayer::frequencyAtHeight (0.0, spacing), 20.0, 0.01, "the bottom's 20 Hz whichever it is");
                expectWithinAbsoluteError (CalibrationPlayer::frequencyAtHeight (1.0, spacing), 20000.0, 0.5, "and the top's 20 kHz");
                for (double fraction : { 0.1, 0.25, 0.5, 0.8 })
                    expectWithinAbsoluteError (CalibrationPlayer::heightOfFrequency (CalibrationPlayer::frequencyAtHeight (fraction, spacing), spacing), fraction, 1.0e-6,
                                               "and back again");
            }
            expectWithinAbsoluteError (CalibrationPlayer::cutoffForRow (1, 3), 632.46, 0.01, "octaves unless you say otherwise");

            // In Hz, three rows are each the same number of Hz from the next
            expectWithinAbsoluteError (CalibrationPlayer::cutoffForRow (2, 3, Spacing::hertz), 5015.0, 0.01);
            expectWithinAbsoluteError (CalibrationPlayer::cutoffForRow (1, 3, Spacing::hertz), 10010.0, 0.01);
            expectWithinAbsoluteError (CalibrationPlayer::cutoffForRow (0, 3, Spacing::hertz), 15005.0, 0.01);

            // In mels they're between: each the same number of mels from the next
            const double low = CalibrationPlayer::cutoffForRow (2, 3, Spacing::mels), middle = CalibrationPlayer::cutoffForRow (1, 3, Spacing::mels),
                         high = CalibrationPlayer::cutoffForRow (0, 3, Spacing::mels);
            expectWithinAbsoluteError (CalibrationPlayer::toMels (middle) - CalibrationPlayer::toMels (low),
                                       CalibrationPlayer::toMels (high) - CalibrationPlayer::toMels (middle), 0.001);
            expectWithinAbsoluteError (CalibrationPlayer::toMels (1000.0), 1000.0, 0.1, "1 kHz is 1000 mels");
            expect (low > CalibrationPlayer::cutoffForRow (2, 3) && low < CalibrationPlayer::cutoffForRow (2, 3, Spacing::hertz));
            expect (middle > 632.46 && middle < 10010.0);

            // Drag mode's field and the guided heights go by it too
            expectWithinAbsoluteError (CalibrationPlayer::Square::cutoffAt (0.5f, Spacing::hertz), 10010.0, 0.01);
            expectWithinAbsoluteError (CalibrationPlayer::cutoffForHeight (-1.0f, Spacing::hertz), 5015.0, 0.01);
            expectWithinAbsoluteError (CalibrationPlayer::cutoffForHeight (1.0f), CalibrationPlayer::cutoffForRow (0, 3), 0.01);

            // What plays: the bottom row of three, in Hz, has lost its bass (it's cut at 5 kHz, not 112 Hz)
            auto lowEnergyOfBottomRow = [] (Spacing spacing)
            {
                CalibrationPlayer player;
                player.prepare (sampleRate);
                player.setMode (CalibrationPlayer::Mode::grid);
                player.setHeightSpacing (spacing);
                player.setGrid (3, 1);
                player.setSelection ({ 2 });
                player.setPlaying (true);
                juce::AudioBuffer<float> buffer (2, (int) sampleRate);
                buffer.clear();
                player.process (buffer);

                FilterChain lowPass;
                lowPass.prepare (sampleRate);
                std::vector<Band> cuts;
                for (int i = 0; i < 4; ++i)
                    cuts.push_back (Band::withQ (i, 1000.0f, 0.0f, 0.7071f, Band::Type::both, Band::Shape::highCut));
                lowPass.setBands (cuts);
                lowPass.reset();
                juce::dsp::AudioBlock<float> block (buffer);
                lowPass.process (block);
                return buffer.getRMSLevel (0, 0, buffer.getNumSamples());
            };
            expectLessThan (lowEnergyOfBottomRow (Spacing::hertz) * 50.0f, lowEnergyOfBottomRow (Spacing::octaves));
            expectLessThan (lowEnergyOfBottomRow (Spacing::hertz), lowEnergyOfBottomRow (Spacing::mels), "mels are between");

            // It's kept with the other calibration settings, and handed to the player with them
            CabinEqAudioProcessor processor;
            auto& state = processor.parameters.state;
            expect (CalibrationSettings::getHeightSpacing (state) == Spacing::octaves);
            CalibrationSettings::setHeightSpacing (state, processor.getCalibration(), Spacing::mels);
            expect (processor.getCalibration().getHeightSpacing() == Spacing::mels);
            processor.getCalibration().setHeightSpacing (Spacing::octaves);
            CalibrationSettings::apply (state, processor.getCalibration());
            expect (processor.getCalibration().getHeightSpacing() == Spacing::mels);
        }

        beginTest ("Depth goes round all the positions several times, a level louder each time round");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::grid);
            player.setGrid (1, 2);
            player.setDepth (3);
            player.setReleaseMs (100.0f); // each burst is gone before the next, so they can be measured apart
            player.setRate (1.0f / 0.3f);
            player.setPlaying (true);

            std::vector<int> positions;
            std::vector<float> levels;
            for (int burst = 0; burst < 7; ++burst)
            {
                juce::AudioBuffer<float> buffer (2, (int) (0.3 * sampleRate));
                buffer.clear();
                player.process (buffer);
                positions.push_back (player.getCurrentPosition());
                const int length = (int) (0.05 * sampleRate);
                levels.push_back (juce::Decibels::gainToDecibels (std::max (buffer.getRMSLevel (0, 0, length), buffer.getRMSLevel (1, 0, length))));
            }
            expect (positions == std::vector<int> { 0, 1, 0, 1, 0, 1, 0 }, "every position, then every position again");
            expectWithinAbsoluteError (levels[1] - levels[0], 0.0f, 3.0f, "the first time round, both are at the floor");
            expectWithinAbsoluteError (levels[2] - levels[0], 10.0f, 3.0f, "the second time round is 10 dB louder");
            expectWithinAbsoluteError (levels[4] - levels[2], 10.0f, 3.0f, "and the third another 10 dB");
            expectWithinAbsoluteError (levels[6] - levels[0], 0.0f, 3.0f, "then it starts again from the floor");
        }

        beginTest ("Depth goes a row at a time, and repeats play the row again at each level before it goes up");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::grid);
            player.setGrid (2, 2);
            player.setDepth (2);
            player.setDepthRepeats (2);
            player.setFloorDb (-12.0f);
            player.setSlope (3.0f);    // pink, on pink's line: both rows are made as loud as each other
            player.setTopLine (3.0f);
            player.setReleaseMs (100.0f); // each burst is gone before the next
            player.setRate (1.0f / 0.3f);
            player.setPlaying (true);

            std::vector<int> positions;
            std::vector<float> levels;
            for (int burst = 0; burst < 17; ++burst)
            {
                juce::AudioBuffer<float> buffer (2, (int) (0.3 * sampleRate));
                buffer.clear();
                player.process (buffer);
                positions.push_back (player.getCurrentPosition());
                const int length = (int) (0.05 * sampleRate);
                levels.push_back (juce::Decibels::gainToDecibels (std::max (buffer.getRMSLevel (0, 0, length), buffer.getRMSLevel (1, 0, length))));
            }
            expect (positions == std::vector<int> { 0, 1, 0, 1, 0, 1, 0, 1, 2, 3, 2, 3, 2, 3, 2, 3, 0 },
                    "the top row four times (two levels, twice each), then the bottom row four times, then the top again");
            expectWithinAbsoluteError (levels[2] - levels[0], 0.0f, 3.0f, "its repeat is at the same level");
            expectWithinAbsoluteError (levels[4] - levels[0], 12.0f, 3.0f, "then it's a level louder");
            expectWithinAbsoluteError (levels[6] - levels[4], 0.0f, 3.0f, "twice");
            expectWithinAbsoluteError (levels[10] - levels[8], 0.0f, 3.0f, "the next row starts again from the floor, twice");
            expectWithinAbsoluteError (levels[12] - levels[8], 12.0f, 3.0f, "then louder");

            // With just one dot of the bottom row picked out, that's the row
            player.setSelection ({ 3 });
            juce::AudioBuffer<float> buffer (2, (int) (0.3 * sampleRate));
            for (int burst = 0; burst < 3; ++burst)
            {
                buffer.clear();
                player.process (buffer);
                expectEquals (player.getCurrentPosition(), 3);
            }

            CalibrationPlayer fresh;
            expectWithinAbsoluteError (fresh.getSlope(), 4.5f, 0.001f, "left alone, the noise falls 4.5 dB an octave");
            expectWithinAbsoluteError (fresh.getTopLine(), 4.5f, 0.001f, "and so do the bursts' bottom edges");
        }

        beginTest ("The grid goes up to 12 rows of 18, and any of them can be picked out");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::grid);
            player.setGrid (99, 99);
            expectEquals (CalibrationPlayer::maxRows, 12);
            expectEquals (CalibrationPlayer::maxColumns, 18);

            // The very last dot, and one from each stretch of the selection's bits
            const std::set<int> picked { 5, 70, 130, 12 * 18 - 1 };
            player.setSelection (picked);
            expect (player.getSelection() == picked);
            expect (player.isSelected (215) && ! player.isSelected (214));

            player.setRate (CalibrationPlayer::maxRate);
            player.setPlaying (true);
            std::set<int> played;
            juce::AudioBuffer<float> buffer (2, (int) (sampleRate / CalibrationPlayer::maxRate));
            for (int burst = 0; burst < 8; ++burst)
            {
                buffer.clear();
                player.process (buffer);
                played.insert (player.getCurrentPosition());
            }
            expect (played == picked, "only the picked ones play, all the way to the last");
        }

        beginTest ("The depth can work on each dot, each row, or the whole cycle");
        {
            using Unit = CalibrationPlayer::DepthUnit;
            // A 2 x 2 grid, two levels. Returns the order, and which bursts were the loud ones.
            auto play = [this] (Unit unit, int repeats, int bursts)
            {
                CalibrationPlayer player;
                player.prepare (sampleRate);
                player.setMode (CalibrationPlayer::Mode::grid);
                player.setGrid (2, 2);
                player.setDepth (2);
                player.setDepthRepeats (repeats);
                player.setDepthUnit (unit);
                player.setFloorDb (-12.0f);
                player.setSlope (3.0f); // pink, on pink's line: every dot is made as loud as the others
                player.setTopLine (3.0f);
                player.setReleaseMs (100.0f);
                player.setRate (1.0f / 0.3f);
                player.setPlaying (true);

                std::vector<int> positions;
                std::vector<float> levels;
                for (int burst = 0; burst < bursts; ++burst)
                {
                    juce::AudioBuffer<float> buffer (2, (int) (0.3 * sampleRate));
                    buffer.clear();
                    player.process (buffer);
                    positions.push_back (player.getCurrentPosition());
                    const int length = (int) (0.05 * sampleRate);
                    levels.push_back (juce::Decibels::gainToDecibels (std::max (buffer.getRMSLevel (0, 0, length), buffer.getRMSLevel (1, 0, length))));
                }
                const float loudest = *std::max_element (levels.begin(), levels.end());
                std::vector<bool> loud;
                for (float level : levels)
                    loud.push_back (level > loudest - 6.0f);
                return std::pair { positions, loud };
            };

            auto [dots, dotsLoud] = play (Unit::dot, 1, 9);
            expect (dots == std::vector<int> { 0, 0, 1, 1, 2, 2, 3, 3, 0 }, "dot: each dot at both levels before the next dot");
            expect (dotsLoud == std::vector<bool> { false, true, false, true, false, true, false, true, false });

            auto [rows, rowsLoud] = play (Unit::row, 1, 9);
            expect (rows == std::vector<int> { 0, 1, 0, 1, 2, 3, 2, 3, 0 }, "row: each row at both levels before the next row");
            expect (rowsLoud == std::vector<bool> { false, false, true, true, false, false, true, true, false });

            auto [cycle, cycleLoud] = play (Unit::cycle, 1, 9);
            expect (cycle == std::vector<int> { 0, 1, 2, 3, 0, 1, 2, 3, 0 }, "cycle: every dot at one level, then every dot at the next");
            expect (cycleLoud == std::vector<bool> { false, false, false, false, true, true, true, true, false });

            auto [dotsTwice, dotsTwiceLoud] = play (Unit::dot, 2, 6);
            expect (dotsTwice == std::vector<int> { 0, 0, 0, 0, 1, 1 }, "dot, with two repeats: twice at each level");
            expect (dotsTwiceLoud == std::vector<bool> { false, false, true, true, false, false });

            auto [cycleTwice, cycleTwiceLoud] = play (Unit::cycle, 2, 12);
            expect (cycleTwice == std::vector<int> { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3 }, "cycle, with two repeats: twice round at each level");
            expect (! cycleTwiceLoud[7] && cycleTwiceLoud[8], "and only then louder");
        }

        beginTest ("The floor sets the quietest time round; the rest climb evenly from there to full level");
        {
            auto levelsWith = [this] (int depth, float floorDb)
            {
                CalibrationPlayer player;
                player.prepare (sampleRate);
                player.setMode (CalibrationPlayer::Mode::grid);
                player.setGrid (1, 1);
                player.setDepth (depth);
                player.setFloorDb (floorDb);
                player.setReleaseMs (100.0f); // each burst is gone before the next, so they can be measured apart
                player.setRate (1.0f / 0.3f);
                player.setPlaying (true);

                std::vector<float> levels;
                for (int burst = 0; burst < depth; ++burst)
                {
                    juce::AudioBuffer<float> buffer (2, (int) (0.3 * sampleRate));
                    buffer.clear();
                    player.process (buffer);
                    levels.push_back (juce::Decibels::gainToDecibels (buffer.getRMSLevel (0, 0, (int) (0.05 * sampleRate))));
                }
                return levels;
            };

            auto levels = levelsWith (3, -30.0f);
            expectWithinAbsoluteError (levels[2] - levels[0], 30.0f, 3.0f, "the first is the floor: 30 dB under the last");
            expectWithinAbsoluteError (levels[1] - levels[0], 15.0f, 3.0f, "the middle one is halfway");

            levels = levelsWith (2, -40.0f);
            expectWithinAbsoluteError (levels[1] - levels[0], 40.0f, 3.0f, "two plays: the floor, then full level");

            const float alone = levelsWith (1, -40.0f)[0];
            expectWithinAbsoluteError (alone, levelsWith (1, 0.0f)[0], 3.0f, "with a depth of one there's no run, so the floor does nothing");
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

        beginTest ("Release sets how long a burst rings, and 0 ms leaves just the attack");
        {
            // How long after a single burst starts the output is last above -60 dB of its peak
            auto ringSeconds = [this] (float releaseMs)
            {
                CalibrationPlayer player;
                player.prepare (sampleRate);
                player.setMode (CalibrationPlayer::Mode::grid);
                player.setGrid (1, 1);
                player.setRate (CalibrationPlayer::minRate); // one burst, then 2 s of nothing
                player.setReleaseMs (releaseMs);
                player.setPlaying (true);
                juce::AudioBuffer<float> buffer (2, (int) (1.9 * sampleRate));
                buffer.clear();
                player.process (buffer);

                // Noise is jumpy, so compare 2 ms RMS windows against the loudest one
                const int window = (int) (0.002 * sampleRate);
                std::vector<float> rms;
                for (int start = 0; start + window <= buffer.getNumSamples(); start += window)
                    rms.push_back (buffer.getRMSLevel (0, start, window));
                const float peak = *std::max_element (rms.begin(), rms.end());
                expectGreaterThan (peak, 0.0f);
                int last = 0;
                for (int i = 0; i < (int) rms.size(); ++i)
                    if (rms[(size_t) i] > peak * 0.001f)
                        last = i;
                return (float) ((last + 1) * window / sampleRate);
            };

            expectLessThan (ringSeconds (0.0f), 0.012f, "0 ms: the 4 ms attack, then gone");
            expectWithinAbsoluteError (ringSeconds (300.0f), 0.3f, 0.08f);
            expectWithinAbsoluteError (ringSeconds (1500.0f), 1.5f, 0.3f);
        }

        beginTest ("Attack sets how long a burst takes to reach full level");
        {
            // When a single burst's 2 ms RMS first gets within 3 dB of its loudest
            auto riseSeconds = [this] (float attackMs)
            {
                CalibrationPlayer player;
                player.prepare (sampleRate);
                player.setMode (CalibrationPlayer::Mode::grid);
                player.setGrid (1, 1);
                player.setRate (CalibrationPlayer::minRate);
                player.setAttackMs (attackMs);
                player.setReleaseMs (CalibrationPlayer::maxReleaseMs); // a slow fade, so the peak is where the attack ends
                player.setPlaying (true);
                juce::AudioBuffer<float> buffer (2, (int) (0.6 * sampleRate));
                buffer.clear();
                player.process (buffer);

                const int window = (int) (0.002 * sampleRate);
                std::vector<float> rms;
                for (int start = 0; start + window <= buffer.getNumSamples(); start += window)
                    rms.push_back (buffer.getRMSLevel (0, start, window));
                // Smooth over 5 windows: noise is jumpy
                std::vector<float> smooth (rms.size());
                for (size_t i = 0; i < rms.size(); ++i)
                {
                    float sum = 0.0f; int n = 0;
                    for (size_t j = i >= 2 ? i - 2 : 0; j < std::min (rms.size(), i + 3); ++j) { sum += rms[j]; ++n; }
                    smooth[i] = sum / (float) n;
                }
                const float peak = *std::max_element (smooth.begin(), smooth.end());
                for (size_t i = 0; i < smooth.size(); ++i)
                    if (smooth[i] > peak * 0.7f)
                        return (float) ((double) (i * (size_t) window) / sampleRate);
                return -1.0f;
            };

            expectLessThan (riseSeconds (0.0f), 0.01f, "0 ms: full level straight away");
            const float soft = riseSeconds (100.0f);
            expectGreaterThan (soft, 0.04f, "100 ms: it takes a while to come up");
            expectLessThan (soft, 0.11f);
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

        beginTest ("Pan steps sweep each spot across the range, and a spot goes through its depth levels before the next spot");
        {
            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::spots);
            player.setSpotCount (2);
            player.setPanRange (-1.0f, 1.0f);
            player.setPanSteps (3);
            player.setDepth (2);
            player.setFloorDb (-10.0f);
            player.setReleaseMs (100.0f); // each burst is gone before the next
            player.setRate (1.0f / 0.3f);
            player.setPlaying (true);

            std::vector<int> positions;
            std::vector<float> lefts, rights, levels;
            for (int burst = 0; burst < 13; ++burst)
            {
                juce::AudioBuffer<float> buffer (2, (int) (0.3 * sampleRate));
                buffer.clear();
                player.process (buffer);
                positions.push_back (player.getCurrentPosition());
                const int start = (int) (0.005 * sampleRate), length = (int) (0.04 * sampleRate);
                lefts.push_back (buffer.getRMSLevel (0, start, length));
                rights.push_back (buffer.getRMSLevel (1, start, length));
                levels.push_back (juce::Decibels::gainToDecibels (std::max (lefts.back(), rights.back())));
            }

            expect (positions == std::vector<int> { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0 }, "A across its three pans, twice (once for each level), then B the same");
            expectGreaterThan (lefts[0], rights[0] * 10.0f, "it starts on the left");
            expectWithinAbsoluteError (lefts[1] / rights[1], 1.0f, 0.5f, "the middle step is in the centre");
            expectGreaterThan (rights[2], lefts[2] * 10.0f, "and ends on the right");
            expectWithinAbsoluteError (levels[2] - levels[0], 0.0f, 3.0f, "the first time across, every pan is at the floor");
            expectWithinAbsoluteError (levels[3] - levels[0], 10.0f, 3.0f, "the second time across is louder");
            expectWithinAbsoluteError (levels[9] - levels[6], 10.0f, 3.0f, "and B does the same");
            expectWithinAbsoluteError (levels[12] - levels[0], 0.0f, 3.0f, "then A starts again from the floor");
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

        beginTest ("The noise's slope and top line are set separately: how fast a burst falls away, and how fast their bottom edges do");
        {
            // The power of a stretch of bursts in the octave around a frequency, in dB. `spot` is the bursts'
            // low cut (0 for none).
            auto octavePower = [this] (float slope, float topLine, float spot, std::vector<float> centres)
            {
                CalibrationPlayer player;
                player.prepare (sampleRate);
                player.setMode (CalibrationPlayer::Mode::spots);
                player.setSpotCount (1);
                player.setSpot (0, spot);
                player.setSlope (slope);
                player.setTopLine (topLine);
                player.setRate (CalibrationPlayer::maxRate); // dense, so there's plenty to measure
                player.setPlaying (true);

                const int order = 17, size = 1 << order;
                juce::AudioBuffer<float> buffer (2, size);
                buffer.clear();
                player.process (buffer);

                juce::dsp::FFT fft (order);
                std::vector<float> data ((size_t) size * 2, 0.0f);
                std::copy_n (buffer.getReadPointer (0), size, data.begin());
                fft.performFrequencyOnlyForwardTransform (data.data());

                std::vector<float> powers;
                for (float centre : centres)
                {
                    double power = 0.0;
                    const int low = (int) (centre / std::sqrt (2.0f) * size / sampleRate), high = (int) (centre * std::sqrt (2.0f) * size / sampleRate);
                    for (int bin = low; bin < high; ++bin)
                        power += (double) data[(size_t) bin] * data[(size_t) bin];
                    powers.push_back ((float) (10.0 * std::log10 (power)));
                }
                return powers;
            };
            auto octaves = [] (float from, float to) { return std::log2 (to / from); };

            // The slope, with no low cut: pink has the same power in each octave; steeper loses the difference, from level at 20 Hz
            const auto pink = octavePower (3.0f, 3.0f, 0.0f, { 250.0f, 4000.0f });
            expectWithinAbsoluteError (pink[0] - pink[1], 0.0f, 1.5f, "a slope of 3 is pink: 250 Hz and 4 kHz octaves are as strong");
            for (float slope : { 4.5f, 6.0f, 7.5f })
            {
                const auto tilted = octavePower (slope, 3.0f, 0.0f, { 250.0f, 4000.0f });
                expectWithinAbsoluteError (tilted[0] - tilted[1], (slope - 3.0f) * 4.0f, 2.0f, "four octaves up, it's down by four times what the slope is over pink's");
                expectWithinAbsoluteError (tilted[0] - pink[0], -(slope - 3.0f) * octaves (20.0f, 250.0f), 1.5f, "and it's level with pink at 20 Hz");
            }

            // The top line: how much further under a pink burst's a burst's bottom edge is, for bursts from 4 kHz
            // against bursts from 500 Hz. Three octaves apart, so three times what the top line is over pink's,
            // whatever the slope: shallower than it, the same, or steeper.
            auto edgeUnderPink = [&] (float slope, float topLine, float spot)
            {
                const std::vector<float> justAbove { spot * 1.75f };
                return octavePower (slope, topLine, spot, justAbove)[0] - octavePower (3.0f, 3.0f, spot, justAbove)[0];
            };
            for (auto [slope, topLine] : { std::pair { 4.5f, 3.0f }, std::pair { 7.5f, 3.0f }, std::pair { 4.5f, 4.5f }, std::pair { 6.0f, 4.5f },
                                           std::pair { 9.0f, 6.0f }, std::pair { 4.5f, 6.0f }, std::pair { 3.0f, 4.5f } })
                expectWithinAbsoluteError (edgeUnderPink (slope, topLine, 4000.0f) - edgeUnderPink (slope, topLine, 500.0f), -(topLine - 3.0f) * 3.0f, 1.5f,
                                           "slope " + juce::String (slope) + ", top line " + juce::String (topLine));

            // A slope of 4.5 on pink's top line: just above its bottom edge, a burst from 2 kHz is about as strong as a pink one
            expectWithinAbsoluteError (edgeUnderPink (4.5f, 3.0f, 2000.0f), -1.5f * octaves (2000.0f, 3500.0f), 1.5f, "on pink's line, it starts as strong as pink");

            CalibrationPlayer limits;
            limits.setSlope (1.0f);
            limits.setTopLine (20.0f);
            expectWithinAbsoluteError (limits.getSlope(), 3.0f, 0.001f, "the slope is no shallower than pink");
            expectWithinAbsoluteError (limits.getTopLine(), 12.0f, 0.001f, "and neither goes past 12 dB an octave");
        }

        beginTest ("Square mode plays a square of dots in reading order: across is pan, up is the low cut");
        {
            using Square = CalibrationPlayer::Square;
            const Square square { 3, 0.5f, 0.5f, 0.4f, 0.6f };
            expectWithinAbsoluteError (square.dot (0).x, 0.3f, 0.001f, "the first is the top left");
            expectWithinAbsoluteError (square.dot (0).y, 0.8f, 0.001f);
            expectWithinAbsoluteError (square.dot (4).x, 0.5f, 0.001f, "the middle one is its centre");
            expectWithinAbsoluteError (square.dot (4).y, 0.5f, 0.001f);
            expectWithinAbsoluteError (square.dot (8).x, 0.7f, 0.001f, "the last is the bottom right");
            expectWithinAbsoluteError (square.dot (8).y, 0.2f, 0.001f);
            expectWithinAbsoluteError ((float) Square::cutoffAt (0.0f), 20.0f, 0.01f, "the bottom of the field is 20 Hz: no cut");
            expectWithinAbsoluteError ((float) Square::cutoffAt (0.5f), 632.5f, 1.0f, "halfway up is halfway in octaves");
            expectWithinAbsoluteError ((float) Square::cutoffAt (1.0f), 20000.0f, 1.0f);

            CalibrationPlayer player;
            player.prepare (sampleRate);
            player.setMode (CalibrationPlayer::Mode::square);
            player.setSquare ({ 2, 0.5f, 0.5f, 0.8f, 0.5f });
            player.setReleaseMs (100.0f);
            player.setRate (1.0f / 0.3f);
            player.setPlaying (true);

            std::vector<int> positions;
            std::vector<float> lefts, rights;
            for (int burst = 0; burst < 5; ++burst)
            {
                juce::AudioBuffer<float> buffer (2, (int) (0.3 * sampleRate));
                buffer.clear();
                player.process (buffer);
                positions.push_back (player.getCurrentPosition());
                lefts.push_back (buffer.getRMSLevel (0, 0, (int) (0.05 * sampleRate)));
                rights.push_back (buffer.getRMSLevel (1, 0, (int) (0.05 * sampleRate)));
            }
            expect (positions == std::vector<int> { 0, 1, 2, 3, 0 }, "its four dots, then round again");
            expectGreaterThan (lefts[0], rights[0] * 3.0f, "the top left dot is on the left");
            expectGreaterThan (rights[1], lefts[1] * 3.0f, "the top right one on the right");
            expectGreaterThan (lefts[2], rights[2] * 3.0f, "then the bottom left");

            // It can't be dragged off the field, or made too small to see
            player.setSquare ({ 7, -1.0f, 2.0f, 0.5f, 0.001f });
            const auto kept = player.getSquare();
            expectEquals (kept.size, 3);
            expectWithinAbsoluteError (kept.x, 0.25f, 0.001f);
            expectWithinAbsoluteError (kept.height, CalibrationPlayer::minSquareSide, 0.001f);
            expectWithinAbsoluteError (kept.y, 1.0f - kept.height * 0.5f, 0.001f);
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

        beginTest ("Drag mode: a square of dots that you move about the field and resize from its corner");
        {
            CabinEqAudioProcessor dragProcessor;
            auto& state = dragProcessor.parameters.state;
            auto& dragPlayer = dragProcessor.getCalibration();
            CalibrationSettings::setMode (state, dragPlayer, CalibrationPlayer::Mode::square);
            CalibrationSettings::setSquare (state, dragPlayer, { 2, 0.5f, 0.5f, 0.4f, 0.4f });
            CalibrationPanel dragPanel (dragProcessor);
            dragPanel.setBounds (0, 0, 900, CalibrationPanel::preferredHeight);
            const auto field = dragPanel.getFieldArea().toFloat();
            auto onField = [field] (float x, float y) { return juce::Point<float> (field.getX() + x * field.getWidth(), field.getBottom() - y * field.getHeight()); };
            auto mouse = [&dragPanel] (juce::Point<float> at, juce::Point<float> down, bool isDown)
            {
                return juce::MouseEvent (juce::Desktop::getInstance().getMainMouseSource(), at, isDown ? juce::ModifierKeys::leftButtonModifier : juce::ModifierKeys(),
                                         0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &dragPanel, &dragPanel, juce::Time::getCurrentTime(), down, juce::Time::getCurrentTime(), 1, false);
            };
            auto dragFromTo = [&] (juce::Point<float> from, juce::Point<float> to)
            {
                dragPanel.mouseDown (mouse (from, from, true));
                dragPanel.mouseDrag (mouse (to, from, true));
                dragPanel.mouseUp (mouse (to, from, false));
            };

            // Grab it by its middle and move it up and to the right
            dragFromTo (onField (0.5f, 0.5f), onField (0.7f, 0.6f));
            auto square = dragPlayer.getSquare();
            expectWithinAbsoluteError (square.x, 0.7f, 0.01f);
            expectWithinAbsoluteError (square.y, 0.6f, 0.01f);
            expectWithinAbsoluteError (square.width, 0.4f, 0.001f, "moving it doesn't resize it");
            expect (dragPlayer.isPlaying(), "and it starts playing");
            expectWithinAbsoluteError ((float) state.getProperty (CalibrationSettings::idSquareX), 0.7f, 0.01f, "where it is is saved");

            // Clicking somewhere else on the field brings it there; the edges keep it wholly on
            dragFromTo (onField (0.05f, 0.95f), onField (0.05f, 0.95f));
            square = dragPlayer.getSquare();
            expectWithinAbsoluteError (square.x, 0.2f, 0.01f, "as far left as it fits");
            expectWithinAbsoluteError (square.y, 0.8f, 0.01f, "and as far up");

            // Its bottom right corner (just outside its dots) resizes it about its centre
            CalibrationSettings::setSquare (state, dragPlayer, { 2, 0.5f, 0.5f, 0.4f, 0.4f });
            const auto corner = onField (0.7f, 0.3f).translated (13.0f, 13.0f);
            dragFromTo (corner, corner.translated (0.1f * field.getWidth(), 0.05f * field.getHeight()));
            square = dragPlayer.getSquare();
            expectWithinAbsoluteError (square.width, 0.6f, 0.005f, "wider by twice as far as the corner went");
            expectWithinAbsoluteError (square.height, 0.5f, 0.005f);
            expectWithinAbsoluteError (square.x, 0.5f, 0.001f, "its centre stays put");

            // Arrows nudge it
            dragPanel.keyPressed (juce::KeyPress (juce::KeyPress::rightKey));
            expectGreaterThan (dragPlayer.getSquare().x, 0.5f);
            dragPanel.stop();
        }
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

        beginTest ("Bands and curve are independent, and both on they stack");
        {
            CabinEqAudioProcessor processor;
            processor.parameters.getParameter (ParamIDs::autoGain)->setValueNotifyingHost (0.0f);
            processor.setPlayConfigDetails (2, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);

            auto profile = processor.getSelectedProfile();
            profile.setBandsOn (true), profile.setCurveOn (false);
            profile.addBand (Band::withQ (0, 1000.0f, 6.0f, 0.3f, Band::Type::both));
            profile.setPoints ({ { 0, 1000.0f, 6.0f } });

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
            auto settle = [&] (int blocks) { juce::Thread::sleep (300); gainAt1k (40); juce::Thread::sleep (300); return gainAt1k (blocks); };

            expectWithinAbsoluteError (settle (200), 6.0f, 0.3f, "bands alone: +6");

            profile.setCurveOn (true);
            expectWithinAbsoluteError (settle (200), 12.0f, 0.4f, "bands and curve: +6 and +6 stack to +12");
            expectEquals (profile.getNumPoints(), 1, "the curve's points are still there");

            profile.setBandsOn (false);
            expectWithinAbsoluteError (settle (200), 6.0f, 0.3f, "curve alone: +6");
            expectEquals (profile.getNumBands(), 1, "the bands are still there");

            profile.setBandsOn (true), profile.setCurveOn (false);
            expectWithinAbsoluteError (settle (200), 6.0f, 0.3f, "and back to bands alone");
        }

        beginTest ("With both layers on, the layer you edit is the one you last picked; with one on, it's that one");
        {
            CabinEqAudioProcessor processor;
            auto profile = processor.getSelectedProfile();
            profile.setBandsOn (true), profile.setCurveOn (true);
            processor.setEditingCurve (false);
            expect (! processor.isEditingCurve(), "bands, when you last picked bands");
            processor.setEditingCurve (true);
            expect (processor.isEditingCurve(), "the curve, once you pick it");
            profile.setCurveOn (false);
            expect (! processor.isEditingCurve(), "with only bands on, you edit the bands");
        }

        beginTest ("A point can differ between the ears: each ear's curve goes through its own version of it");
        {
            const CurvePoint point { 0, 1000.0f, 2.0f, 0.5f, 6.0f }; // half an octave higher, and 6 dB louder, in the left ear
            expectWithinAbsoluteError (point.forEar (0).freq, 1000.0f * std::pow (2.0f, 0.25f), 0.1f);
            expectWithinAbsoluteError (point.forEar (1).freq, 1000.0f / std::pow (2.0f, 0.25f), 0.1f);
            expectWithinAbsoluteError (point.forEar (0).gain, 5.0f, 0.001f);
            expectWithinAbsoluteError (point.forEar (1).gain, -1.0f, 0.001f);

            BandProfile curve (std::vector<CurvePoint> { { 0, 200.0f, 0.0f }, point, { 2, 8000.0f, 0.0f } }, 0.0f);
            expect (curve.earsDiffer());
            expectWithinAbsoluteError (curve.curveDbAt (point.forEar (0).freq, 0), 5.0f, 0.01f, "the left ear's curve goes through its version");
            expectWithinAbsoluteError (curve.curveDbAt (point.forEar (1).freq, 1), -1.0f, 0.01f, "and the right's through its");
            expectWithinAbsoluteError (curve.curveDbAt (1000.0f, -2), 2.0f, 0.01f, "as drawn, it's where the point is");
            expectWithinAbsoluteError (curve.curveDbAt (200.0f, 0), curve.curveDbAt (200.0f, 1), 0.001f, "a point with no difference is the same in both");
            expect (! BandProfile (std::vector<CurvePoint> { { 0, 200.0f, 3.0f } }, 0.0f).earsDiffer());

            // Saved with the profile, and gone from it when it's zero again
            auto tree = CabinEqProfile::createTree ("Ears", curve);
            CabinEqProfile saved (tree, nullptr);
            auto read = *saved.getPoint (1);
            expectWithinAbsoluteError (read.freqSplit, 0.5f, 0.0001f);
            expectWithinAbsoluteError (read.gainSplit, 6.0f, 0.0001f);
            read.freqSplit = read.gainSplit = 0.0f;
            saved.updatePoint (read);
            expect (! saved.getBandProfile().earsDiffer());
            expect (! tree.toXmlString().contains ("Split="), "nothing's kept for a point that's the same in both ears");
        }

        beginTest ("A point that's louder in one ear plays that way through the processor, with no Split L/R needed");
        {
            CabinEqAudioProcessor processor;
            processor.parameters.getParameter (ParamIDs::autoGain)->setValueNotifyingHost (0.0f);
            processor.setPlayConfigDetails (2, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);

            auto profile = processor.getSelectedProfile();
            profile.setBandsOn (false), profile.setCurveOn (true);
            profile.setPoints ({ { 0, 1000.0f, 2.0f, 0.0f, 6.0f } }); // one point: +5 dB in the left ear, -1 in the right, everywhere

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
            expectWithinAbsoluteError (leftGain, 5.0f, 0.3f);
            expectWithinAbsoluteError (rightGain, -1.0f, 0.3f);
        }

        beginTest ("A split curve plays each ear its own gain through the processor");
        {
            CabinEqAudioProcessor processor;
            processor.parameters.getParameter (ParamIDs::autoGain)->setValueNotifyingHost (0.0f);
            processor.setPlayConfigDetails (2, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);

            auto profile = processor.getSelectedProfile();
            profile.setBandsOn (false), profile.setCurveOn (true);
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
            profile.setBandsOn (false), profile.setCurveOn (true);

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

            profile.setBandsOn (true), profile.setCurveOn (false);
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

//==============================================================================
class GuidedSearchTests : public juce::UnitTest
{
public:
    GuidedSearchTests() : juce::UnitTest ("Guided calibration", "CabinEQ") {}

    /// A curve for the search to work on, and a record of what it asked for
    struct Bench
    {
        GuidedSearch::Vector curve, heard;
        std::vector<GuidedSearch::Arrangement> played;
        std::vector<int> refinedTo;
        juce::StringArray events;
        int kept = 0, reverted = 0;
        std::unique_ptr<GuidedSearch> search;

        Bench()
        {
            GuidedSearch::Callbacks callbacks;
            callbacks.readVertices = [this] { return curve; };
            callbacks.preview = [this] (const GuidedSearch::Vector& v) { heard = v; };
            callbacks.commit = [this] (const GuidedSearch::Vector& v) { curve = heard = v; ++kept; };
            callbacks.revert = [this] (const GuidedSearch::Vector& v) { heard = v; ++reverted; };
            callbacks.refine = [this] (int count)
            {
                // Each new vertex between two old ones takes their average, as a curve through them would nearly
                const int before = (int) curve.size() / 2;
                GuidedSearch::Vector finer ((size_t) count * 2, 0.0f);
                for (int ear = 0; ear < 2 && before > 1; ++ear)
                    for (int i = 0; i < count; ++i)
                    {
                        const float at = (float) i * (float) (before - 1) / (float) (count - 1);
                        const int below = std::min ((int) at, before - 2);
                        finer[(size_t) (ear * count + i)] = curve[(size_t) (ear * before + below)] * (1.0f - (at - (float) below))
                                                          + curve[(size_t) (ear * before + below + 1)] * (at - (float) below);
                    }
                curve = finer;
                refinedTo.push_back (count);
                return curve;
            };
            callbacks.playArrangement = [this] (const GuidedSearch::Arrangement& arrangement) { played.push_back (arrangement); };
            callbacks.log = [this] (const juce::var& line) { events.add (line["event"].toString()); };
            search = std::make_unique<GuidedSearch> (GuidedSearch::Config(), std::move (callbacks));
        }

        /// Hears a new arrangement through, if that's what it's waiting for
        void listenFirst()
        {
            if (search->getPhase() == GuidedSearch::Phase::introducing)
                search->framePlayed();
        }

        /// Explores to (x, y), lets go, hears it out, and says whether it's better
        void choose (float x, float y, bool better)
        {
            listenFirst();
            search->padMoved ({ x, y });
            search->padReleased();
            search->framePlayed();
            search->choose (better);
        }

        /// Hears before and after on a check, and answers
        void check (GuidedSearch::Response response)
        {
            search->framePlayed(); // before has played: now after
            search->respond (response);
        }

        void keep (float x, float y)
        {
            choose (x, y, true);
            check (GuidedSearch::Response::better);
            check (GuidedSearch::Response::same);
        }

        void skip() { search->choose (false); }
    };

    void runTest() override
    {
        using Phase = GuidedSearch::Phase;
        using Response = GuidedSearch::Response;
        using Dimension = GuidedSearch::Dimension;
        using Position = GuidedSearch::Position;
        using Arrangement = GuidedSearch::Arrangement;

        // The eight corners, however they're ordered
        auto corners = []
        {
            std::vector<Position> all;
            for (float level : { -1.0f, 1.0f })
                for (float height : { -1.0f, 1.0f })
                    for (float pan : { -1.0f, 1.0f })
                        all.push_back ({ pan, height, level });
            return all;
        };
        auto contains = [] (const std::vector<Position>& positions, const Position& position)
        {
            return std::find (positions.begin(), positions.end(), position) != positions.end();
        };
        auto hasEveryCorner = [&] (const Arrangement& arrangement)
        {
            const auto positions = arrangement.positions();
            for (const auto& corner : corners())
                if (! contains (positions, corner))
                    return false;
            return true;
        };
        auto sameSounds = [&] (const Arrangement& a, const Arrangement& b)
        {
            const auto first = a.positions(), second = b.positions();
            if (first.size() != second.size())
                return false;
            for (const auto& position : first)
                if (! contains (second, position))
                    return false;
            return true;
        };
        auto withLayerBetween = [] (Dimension dimension, float at)
        {
            Arrangement arrangement;
            arrangement.layers[(size_t) dimension] = { -1.0f, at, 1.0f };
            return arrangement;
        };

        beginTest ("It starts with five vertices, the planes in the order given, the widest range, and the corners of a box");
        {
            Bench bench;
            bench.search->begin();
            expect (bench.refinedTo == std::vector<int> { 5 }, "the curve's made five vertices");
            expectEquals (bench.search->getVertexCount(), 5);
            expectWithinAbsoluteError (bench.search->getRangeDb(), 30.0f, 0.001f);
            expectEquals (bench.search->getNumStructuredPlanes(), 13, "4 of (C, C next), 5 of (C, D), 4 of (D, D next)");
            expectEquals (bench.search->getPassSize(), 13, "a pass is every plane once");

            // Left ear's five gains, then the right's. C is both ears; D is left up, right down.
            const auto& planes = bench.search->getStructuredPlanes();
            expect (planes[0].u == GuidedSearch::Vector { 0, 0, 0, 1, 0,  0, 0, 0, 1, 0 }, "first: both ears at the fourth vertex...");
            expect (planes[0].v == GuidedSearch::Vector { 0, 0, 0, 0, 1,  0, 0, 0, 0, 1 }, "...with both ears at the fifth, the highest pair");
            expect (planes[3].u == GuidedSearch::Vector { 1, 0, 0, 0, 0,  1, 0, 0, 0, 0 }, "down to the lowest pair");
            expect (planes[4].u == GuidedSearch::Vector { 0, 0, 0, 0, 1,  0, 0, 0, 0, 1 }, "then each vertex's level...");
            expect (planes[4].v == GuidedSearch::Vector { 0, 0, 0, 0, 1,  0, 0, 0, 0, -1 }, "...with its difference between the ears, from the highest");
            expect (planes[9].u == GuidedSearch::Vector { 0, 0, 0, 1, 0,  0, 0, 0, -1, 0 }, "then neighbouring differences");
            expect (planes[9].v == GuidedSearch::Vector { 0, 0, 0, 0, 1,  0, 0, 0, 0, -1 });

            expectEquals (bench.search->getPlane().structuredIndex, 0);
            const auto& arrangement = bench.search->getArrangement();
            expectEquals (arrangement.size(), 8, "what you judge is all three dimensions at once: two layers of each");
            expect (! arrangement.middleDimension().has_value());
            expect (hasEveryCorner (arrangement), "left and right, low and high, far and near");
            expectEquals ((int) bench.played.size(), 1, "and it's playing");
        }

        beginTest ("An arrangement's read like a book: the near side first, each side along its rows from the top, each row from the left");
        {
            const Arrangement box;
            const std::vector<Position> expected { { -1, 1, 1 }, { 1, 1, 1 }, { -1, -1, 1 }, { 1, -1, 1 },         // pan, height, level: the near side...
                                                   { -1, 1, -1 }, { 1, 1, -1 }, { -1, -1, -1 }, { 1, -1, -1 } };   // ...then the far one
            expect (box.positions() == expected);

            // A layer between low and high is a row between: six to a side
            const std::vector<Position> withRow { { -1, 1, 1 }, { 1, 1, 1 }, { -1, 0, 1 }, { 1, 0, 1 }, { -1, -1, 1 }, { 1, -1, 1 },
                                                  { -1, 1, -1 }, { 1, 1, -1 }, { -1, 0, -1 }, { 1, 0, -1 }, { -1, -1, -1 }, { 1, -1, -1 } };
            expect (withLayerBetween (Dimension::height, 0.0f).positions() == withRow);
            // One between left and right makes each row three long
            const auto withColumn = withLayerBetween (Dimension::width, 0.5f).positions();
            expect (withColumn[0] == Position { -1, 1, 1 } && withColumn[1] == Position { 0.5f, 1, 1 } && withColumn[2] == Position { 1, 1, 1 }
                    && withColumn[3] == Position { -1, -1, 1 });
            // And one between far and near is a whole side between them
            const auto withSide = withLayerBetween (Dimension::depth, 0.0f).positions();
            expect (withSide[3] == Position { 1, -1, 1 } && withSide[4] == Position { -1, 1, 0 } && withSide[8] == Position { -1, 1, -1 });

            // Every arrangement there is: the corners, and a layer between in each dimension, wherever it can be
            std::vector<Arrangement> all { box };
            for (auto dimension : { Dimension::width, Dimension::height, Dimension::depth })
                for (float at : { 0.0f, -0.5f, 0.5f })
                    all.push_back (withLayerBetween (dimension, at));
            for (const auto& arrangement : all)
            {
                const auto positions = arrangement.positions();
                expectEquals ((int) positions.size(), arrangement.middleDimension().has_value() ? 12 : 8);
                expectEquals (arrangement.size(), (int) positions.size());
                expect (hasEveryCorner (arrangement), "the corners are always there");
                for (size_t i = 0; i + 1 < positions.size(); ++i)
                {
                    const auto& from = positions[i];
                    const auto& to = positions[i + 1];
                    expect (to.level < from.level
                            || (to.level == from.level && (to.height < from.height || (to.height == from.height && to.pan > from.pan))),
                            "reading order: on to the right, else down a row, else back a side");
                }

                // Another order is the same sounds, down each side's columns, and still a side at a time
                const auto other = arrangement.reordered();
                expect (sameSounds (arrangement, other) && ! (other.positions() == positions));
                expect (other.reordered() == arrangement);
                const auto columns = other.positions();
                expect (columns[0] == positions[0], "from the same corner");
                expect (columns[1].pan == columns[0].pan && columns[1].height < columns[0].height, "then the one under it");
                for (size_t i = 0; i + 1 < columns.size(); ++i)
                    expect (columns[i + 1].level <= columns[i].level);
            }

            // Each sound as many times as asked before the next
            const auto bursts = box.bursts (4);
            expectEquals ((int) bursts.size(), 32);
            for (int i = 0; i < 32; ++i)
                expect (bursts[(size_t) i] == expected[(size_t) (i / 4)], "burst " + juce::String (i));
            expectEquals ((int) box.bursts (2).size(), 16);
            expectLessOrEqual ((int) withLayerBetween (Dimension::height, 0.0f).bursts (8).size(), CalibrationPlayer::maxSequenceLength,
                               "the longest there is, twelve sounds eight times each, fits in what the player can hold");
        }

        beginTest ("The order's the same every turn, and all through a turn");
        {
            Bench bench;
            bench.search->begin();
            bench.listenFirst();
            const auto first = bench.search->getArrangement();
            expect (first == Arrangement(), "the box, in reading order");

            // Dragging, letting go, hearing where it started: always the same sounds in the same order
            bench.search->padMoved ({ 0.2f, 0.1f });
            bench.search->padReleased();
            bench.search->setHearingStart (true);
            bench.search->setHearingStart (false);
            bench.search->framePlayed();
            for (const auto& played : bench.played)
                expect (played == first);

            for (int turn = 0; turn < 4; ++turn)
            {
                bench.skip();
                expect (bench.search->getArrangement() == first, "so you always know where you are in it");
            }
        }

        beginTest ("The arrangement plays once through before the pad does anything, the first time it comes up");
        {
            Bench bench;
            bench.search->begin();
            expect (bench.search->getPhase() == Phase::introducing);
            expect (! bench.search->canExplore());
            bench.search->padMoved ({ 0.5f, 0.5f });
            expect (bench.heard.empty(), "the pad does nothing yet");
            bench.search->framePlayed();
            expect (bench.search->getPhase() == Phase::exploring && bench.search->canExplore(), "once it's played through, it's yours");
            bench.search->padMoved ({ 0.5f, 0.0f });
            expectWithinAbsoluteError (bench.heard[3], 15.0f, 0.001f);

            bench.skip();
            expect (bench.search->getPhase() == Phase::exploring, "you know these sounds now, in whatever order");
        }

        beginTest ("The pad previews from the turn's starting curve, by the range along each axis");
        {
            Bench bench;
            bench.search->begin();
            bench.curve = { 1, 2, 3, 4, 5,  1, 2, 3, 4, 5 };
            bench.skip(); // so the next turn starts from that curve
            bench.listenFirst();
            // The second plane: both ears at the third vertex across, at the fourth up
            bench.search->padMoved ({ 0.5f, -1.0f });
            expect (bench.heard == GuidedSearch::Vector { 1, 2, 18, -26, 5,  1, 2, 18, -26, 5 }, "half the range up one axis, all of it down the other");
            bench.search->padMoved ({ 0.1f, 0.0f });
            expect (bench.heard == GuidedSearch::Vector { 1, 2, 6, 4, 5,  1, 2, 6, 4, 5 }, "from the start each time, not from where the pad was");
            expect (bench.curve == GuidedSearch::Vector { 1, 2, 3, 4, 5,  1, 2, 3, 4, 5 }, "and nothing's kept yet");
        }

        beginTest ("A choice has to be heard out, and then hold up in another order and with more positions, to be kept");
        {
            Bench bench;
            bench.search->begin();
            bench.listenFirst();
            const auto turn = bench.search->getArrangement();

            bench.search->padMoved ({ 0.5f, 0.0f });
            expect (! bench.search->canChoose(), "not while you're still dragging");
            bench.search->padReleased();
            expect (bench.search->getPhase() == Phase::settling);
            expect (! bench.search->canChoose(), "nor until it's all played once through with it standing still");
            bench.search->framePlayed();
            expect (bench.search->canChoose());

            bench.search->choose (true);
            expect (bench.search->getPhase() == Phase::validatingBefore, "first the starting curve...");
            expect (bench.heard == bench.search->getStart());
            const auto firstCheck = bench.search->getArrangement();
            expect (sameSounds (firstCheck, turn) && firstCheck.readsDownColumns, "...on the same sounds in another order, down the columns: still every dimension at once");
            expect (bench.search->getTurnArrangement() == turn);
            expect (! bench.search->canRespond(), "no answering before the change has been heard against it");
            bench.search->framePlayed();
            expect (bench.search->getPhase() == Phase::validatingAfter, "then the change...");
            expect (bench.search->getArrangement() == firstCheck, "...on exactly the same sounds, in the same order");
            expectWithinAbsoluteError (bench.heard[3], 15.0f, 0.001f);
            expect (bench.search->canRespond());
            bench.search->framePlayed();
            expect (bench.search->getPhase() == Phase::validatingBefore && bench.search->canRespond(), "and round again until you answer");

            bench.search->respond (Response::same);
            expect (bench.search->getPhase() == Phase::validatingBefore, "a second check...");
            const auto secondCheck = bench.search->getArrangement();
            expect (secondCheck.middleDimension().has_value() && secondCheck.size() == 12, "...with positions the choice wasn't made on: a layer between");
            expect (hasEveryCorner (secondCheck), "as well as the corners, not instead of them");
            expectEquals (bench.search->getValidationIndex(), 1);
            expectEquals (bench.kept, 0);
            bench.check (Response::better);
            expectEquals (bench.kept, 1, "worse on neither: kept");
            expectWithinAbsoluteError (bench.curve[3], 15.0f, 0.001f);
        }

        beginTest ("Worse on either check, or no better to begin with, and it's dropped for the next structured plane");
        {
            Bench bench;
            bench.search->begin();
            bench.choose (0.5f, 0.5f, true);
            bench.check (Response::better);
            bench.check (Response::worse);
            expectEquals (bench.kept, 0);
            expectEquals (bench.reverted, 1, "back to the curve it started from");
            expect (bench.curve == GuidedSearch::Vector (10, 0.0f));
            expectEquals (bench.search->getPlane().structuredIndex, 1, "on to the next structured plane");
            expect (bench.search->getPhase() == Phase::exploring);

            bench.choose (0.3f, 0.0f, false);
            expectEquals (bench.search->getPlane().structuredIndex, 2);
            bench.skip(); // skipped without even moving
            expectEquals (bench.search->getPlane().structuredIndex, 3);
            expectEquals (bench.kept, 0);
        }

        beginTest ("A kept choice that touched the pad's edge gets the same plane and the same sounds again, recentred");
        {
            Bench bench;
            bench.search->begin();
            const auto arrangement = bench.search->getArrangement();
            bench.keep (1.0f, 0.2f);
            expectEquals (bench.search->getPlane().structuredIndex, 0, "the same plane");
            expect (! bench.search->getPlane().isAdaptive);
            expect (bench.search->getArrangement() == arrangement, "with the same sounds in the same order");
            expectWithinAbsoluteError (bench.search->getStart()[3], 30.0f, 0.001f, "from where it got to");
            expectWithinAbsoluteError (bench.search->getRangeDb(), 30.0f, 0.001f, "at the same range");
            expect (bench.search->getPad() == juce::Point<float>(), "with the pad back in the middle");
            bench.search->padMoved ({ 1.0f, 0.0f });
            expectWithinAbsoluteError (bench.heard[3], 60.0f, 0.001f, "so it can go on past the range");
        }

        beginTest ("A kept choice inside the pad makes an adaptive plane: the movement, and the least-used direction apart from it");
        {
            Bench bench;
            bench.search->begin();
            bench.keep (0.5f, 0.25f); // 15 dB at the fourth vertex and 7.5 at the fifth, in both ears
            const auto& plane = bench.search->getPlane();
            expect (plane.isAdaptive);
            expect (plane.u == GuidedSearch::Vector { 0, 0, 0, 1, 0.5f,  0, 0, 0, 1, 0.5f }, "across is that movement, its largest entry 1");
            expectWithinAbsoluteError (GuidedSearch::dot (plane.u, plane.v), 0.0f, 1.0e-4f, "up has nothing of it left in");
            expectWithinAbsoluteError (GuidedSearch::largest (plane.v), 1.0f, 1.0e-5f);
            // Used so far: both ears at the fourth and fifth (the first plane). Of the rest, the next in the queue is the third.
            expect (plane.v == GuidedSearch::Vector { 0, 0, 1, 0, 0,  0, 0, 1, 0, 0 }, "the least-used direction, by the queue's order: both ears at the third vertex");
            expectEquals (bench.search->getArrangement().size(), 8, "still judged on the whole arrangement");

            // Continue the movement: left undoes it, right goes further
            bench.search->padMoved ({ -0.5f, 0.0f });
            expectWithinAbsoluteError (bench.heard[3], 0.0f, 0.001f, "left on the pad undoes it");

            bench.keep (0.2f, 0.4f);
            expect (bench.search->getPlane().isAdaptive, "a second adaptive turn");
            bench.keep (0.2f, 0.4f);
            expect (! bench.search->getPlane().isAdaptive, "after two running, a structured one");
            expectEquals (bench.search->getPlane().structuredIndex, 1, "the next in the queue");
        }

        beginTest ("After two adaptive turns running the next is structured, even if the last touched the edge");
        {
            Bench bench;
            bench.search->begin();
            bench.keep (0.5f, 0.25f);
            expect (bench.search->getPlane().isAdaptive);
            bench.keep (1.0f, 0.0f); // at the edge: the same adaptive plane again, which is the second running
            expect (bench.search->getPlane().isAdaptive);
            bench.keep (1.0f, 0.0f); // at the edge again, but that's two
            expect (! bench.search->getPlane().isAdaptive);
            expectEquals (bench.search->getPlane().structuredIndex, 1);
        }

        beginTest ("The other axis: least used first, then the queue's order, and never one nearly parallel to the movement");
        {
            using Direction = GuidedSearch::Direction;
            const GuidedSearch::Vector movement { 1.0f, 0.5f, 0.0f };
            std::vector<Direction> pool { { { 1.0f, 0.55f, 0.0f }, "nearly the movement", 0, 0 },
                                          { { 0.0f, 1.0f, 0.0f }, "second", 1, 1 },
                                          { { 0.0f, 0.0f, 1.0f }, "third", 0, 3 },
                                          { { 0.0f, 1.0f, 1.0f }, "fourth", 0, 2 } };
            expectGreaterThan (GuidedSearch::cosine (movement, pool[0].vector), 0.95f);
            expectEquals (GuidedSearch::leastUsedIndependent (movement, pool, 0.95f), 3, "not the nearly parallel one; of the unused, the earlier in the queue");
            pool[3].uses = 1;
            expectEquals (GuidedSearch::leastUsedIndependent (movement, pool, 0.95f), 2, "fewer uses comes first");
            pool[2].uses = 2;
            expectEquals (GuidedSearch::leastUsedIndependent (movement, pool, 0.95f), 1, "equally used: the queue's order");
            expectEquals (GuidedSearch::leastUsedIndependent (movement, { pool[0] }, 0.95f), -1, "none, if they're all nearly parallel");

            // Its part along the movement is taken out, and what's left has a largest entry of 1
            const auto apart = GuidedSearch::apartFrom (pool[1].vector, movement);
            expectWithinAbsoluteError (GuidedSearch::dot (apart, movement), 0.0f, 1.0e-5f);
            expectWithinAbsoluteError (GuidedSearch::largest (apart), 1.0f, 1.0e-5f);
            expect (apart == GuidedSearch::Vector { -0.5f, 1.0f, 0.0f });
            expect (GuidedSearch::apartFrom (movement, movement).empty(), "nothing's left of the movement itself");
        }

        beginTest ("Hear start plays the curve the turn began with, and the choice then gets its whole time through again");
        {
            Bench bench;
            bench.search->begin();
            bench.search->setHearingStart (true);
            expect (! bench.search->isHearingStart(), "not while the arrangement's being introduced");
            bench.listenFirst();

            bench.search->padMoved ({ 0.5f, 0.0f });
            bench.search->padReleased();
            bench.search->setHearingStart (true);
            expect (bench.search->isHearingStart());
            expect (bench.heard == bench.search->getStart(), "you hear where it started");
            expect (bench.search->getPad() == juce::Point<float> (0.5f, 0.0f), "and the pad's where you left it");
            bench.search->framePlayed();
            expect (! bench.search->canChoose(), "what played through wasn't your choice, so it can't be called better yet");

            bench.search->setHearingStart (false);
            expectWithinAbsoluteError (bench.heard[3], 15.0f, 0.001f, "back to your change");
            expect (bench.search->getPhase() == Phase::settling, "which plays once through again");
            bench.search->framePlayed();
            expect (bench.search->canChoose());

            bench.search->setHearingStart (true);
            bench.search->padMoved ({ 0.2f, 0.0f });
            expect (! bench.search->isHearingStart(), "dragging goes back to hearing the change");
            expectWithinAbsoluteError (bench.heard[3], 6.0f, 0.001f);

            bench.search->padReleased();
            bench.search->framePlayed();
            bench.search->choose (true);
            bench.search->setHearingStart (true);
            expect (! bench.search->isHearingStart(), "the checks play before and after by themselves");
        }

        beginTest ("A pass is every plane once; with nothing kept, a layer between, then nearer each side, then half the range, then more vertices");
        {
            Bench bench;
            bench.search->begin();

            // Say "no better" to everything
            using Stage = std::tuple<int, float, int>; // vertices, range, how many positions the arrangement's up to
            std::vector<std::pair<Stage, int>> stages;
            auto key = [&bench] { return Stage { bench.search->getVertexCount(), bench.search->getRangeDb(), bench.search->getCoverageLevel() }; };
            auto last = key();
            int turns = 0, guard = 0;
            std::set<int> firstPass;
            while (bench.search->getPhase() != Phase::finished && ++guard < 20000)
            {
                if (key() != last)
                {
                    stages.push_back ({ last, turns });
                    last = key();
                    turns = 0;
                }
                ++turns;
                if (stages.empty())
                    firstPass.insert (bench.search->getPlane().structuredIndex);
                bench.skip();
            }
            stages.push_back ({ last, turns });

            expectEquals ((int) firstPass.size(), 13, "the first pass had every plane, once each");
            const std::vector<std::pair<Stage, int>> expected {
                { { 5, 30.0f, 0 }, 13 }, { { 5, 30.0f, 1 }, 13 }, { { 5, 30.0f, 2 }, 13 }, // more positions before a smaller range
                { { 5, 15.0f, 2 }, 13 }, { { 5, 7.5f, 2 }, 13 }, { { 5, 3.75f, 2 }, 13 },
                { { 9, 15.0f, 2 }, 25 }, { { 9, 7.5f, 2 }, 25 }, { { 9, 3.75f, 2 }, 25 },
                { { 17, 15.0f, 2 }, 49 }, { { 17, 7.5f, 2 }, 49 }, { { 17, 3.75f, 2 }, 49 } };
            expectEquals ((int) stages.size(), (int) expected.size());
            for (size_t i = 0; i < std::min (stages.size(), expected.size()); ++i)
                expect (stages[i] == expected[i], "stage " + juce::String ((int) i) + ": " + juce::String (std::get<0> (stages[i].first)) + " vertices, +/-"
                                                    + juce::String (std::get<1> (stages[i].first)) + " dB, coverage " + juce::String (std::get<2> (stages[i].first))
                                                    + ", " + juce::String (stages[i].second) + " turns");
            expect (bench.refinedTo == std::vector<int> { 5, 9, 17 });
            expect (bench.search->getPhase() == Phase::finished);
            expectEquals (bench.kept, 0);
        }

        beginTest ("Something kept in a pass keeps the arrangement and the range as they are for another pass");
        {
            Bench bench;
            bench.search->begin();
            bench.keep (0.3f, 0.0f); // one improvement, on the first plane
            int structuredTurns = 0, guard = 0;
            auto skipStructured = [&] (int upTo)
            {
                while (structuredTurns < upTo && ++guard < 2000)
                {
                    if (! bench.search->getPlane().isAdaptive)
                        ++structuredTurns;
                    bench.skip();
                }
            };
            skipStructured (20); // the rest of that pass, and well into the next
            expectEquals (bench.search->getCoverageLevel(), 0, "nothing's changed");
            expectWithinAbsoluteError (bench.search->getRangeDb(), 30.0f, 0.001f);
            skipStructured (27);
            expectEquals (bench.search->getCoverageLevel(), 1, "that pass kept nothing: now there's a layer between");
            expectWithinAbsoluteError (bench.search->getRangeDb(), 30.0f, 0.001f, "before the range gets smaller");
        }

        beginTest ("A layer between: each dimension has it in turn, the corners stay, and later it's nearer one side or the other too");
        {
            Bench bench;
            bench.search->begin();
            int guard = 0;
            while (bench.search->getCoverageLevel() == 0 && ++guard < 500)
                bench.skip();
            expect (bench.search->getPhase() == Phase::introducing, "twelve sounds are new: they play once through first");
            bench.listenFirst();

            std::set<int> dimensions;
            for (int turn = 0; turn < 3; ++turn)
            {
                const auto arrangement = bench.search->getArrangement();
                expectEquals (arrangement.size(), 12, "three layers of one dimension, two of the others");
                expect (hasEveryCorner (arrangement), "the box is still all there round it");
                if (const auto middle = arrangement.middleDimension())
                {
                    dimensions.insert ((int) *middle);
                    expect (arrangement.layersOf (*middle) == std::vector<float> { -1.0f, 0.0f, 1.0f }, "halfway between");
                    expect (! arrangement.readsDownColumns, "still in reading order");
                }
                expect (bench.search->getPhase() == Phase::exploring);
                bench.skip();
            }
            expectEquals ((int) dimensions.size(), 3, "width, height and depth each had the layer between");

            while (bench.search->getCoverageLevel() == 1 && ++guard < 1000)
                bench.skip();
            expectEquals (bench.search->getCoverageLevel(), 2);
            expectWithinAbsoluteError (bench.search->getRangeDb(), 30.0f, 0.001f, "all before the range is touched");
            std::set<std::pair<int, float>> layers;
            for (int turn = 0; turn < 9; ++turn)
            {
                const auto arrangement = bench.search->getArrangement();
                if (const auto middle = arrangement.middleDimension())
                    layers.insert ({ (int) *middle, arrangement.layersOf (*middle)[1] });
                expect (hasEveryCorner (arrangement));
                bench.skip();
            }
            expectEquals ((int) layers.size(), 9, "each dimension's had it halfway, nearer its low side, and nearer its high side");
            expect (layers.count ({ (int) Dimension::height, -0.5f }) == 1 && layers.count ({ (int) Dimension::depth, 0.5f }) == 1);
        }

        beginTest ("With a layer between already, a choice is checked in another order, and with the layer in another dimension");
        {
            Bench bench;
            bench.search->begin();
            int guard = 0;
            while (bench.search->getCoverageLevel() == 0 && ++guard < 500)
                bench.skip();
            const auto turn = bench.search->getArrangement();
            bench.choose (0.4f, 0.0f, true);

            const auto firstCheck = bench.search->getArrangement();
            expect (sameSounds (firstCheck, turn) && ! (firstCheck == turn), "the same twelve, in another order");
            expect (firstCheck.middleDimension() == turn.middleDimension());
            bench.check (Response::same);
            const auto secondCheck = bench.search->getArrangement();
            expect (secondCheck.middleDimension().has_value() && secondCheck.middleDimension() != turn.middleDimension(),
                    "then positions it wasn't made on: the layer between is in another dimension");
            expect (hasEveryCorner (secondCheck));
            bench.check (Response::better);
            expectEquals (bench.kept, 1);
        }

        beginTest ("Broad directions carry over to finer vertices, and everything's logged");
        {
            Bench bench;
            bench.search->begin();
            int guard = 0;
            while (bench.search->getVertexCount() == 5 && ++guard < 5000)
                bench.skip();
            expectEquals (bench.search->getVertexCount(), 9);

            // "Both ears at the lowest of five" is now a slope down over the first three of nine
            bool found = false;
            for (const auto& direction : bench.search->getDirections())
                found |= direction.vector == GuidedSearch::Vector { 1, 0.5f, 0, 0, 0, 0, 0, 0, 0,  1, 0.5f, 0, 0, 0, 0, 0, 0, 0 };
            expect (found, "an earlier direction, as the coordinated change to the finer vertices it now is");
            expectEquals (bench.search->getNumStructuredPlanes(), 8 + 9 + 8);

            expect (bench.events.contains ("begin") && bench.events.contains ("turn") && bench.events.contains ("pass")
                    && bench.events.contains ("coverage") && bench.events.contains ("range") && bench.events.contains ("refined"));
        }

        beginTest ("The high-frequency bias bunches the points towards the top, and doubling them keeps the ones there were");
        {
            auto at = [] (int vertex, int count, float bias) { return GuidedCalibrationView::vertexFrequency (vertex, count, bias); };
            expectWithinAbsoluteError (at (0, 5, 1.5f), 20.0f, 0.01f);
            expectWithinAbsoluteError (at (4, 5, 1.5f), 20000.0f, 1.0f);
            expectWithinAbsoluteError (at (2, 5, 1.0f), 632.5f, 1.0f, "a bias of 1 is evenly spaced in octaves");
            expectWithinAbsoluteError (at (1, 5, 1.5f), 225.0f, 2.0f);
            expectWithinAbsoluteError (at (2, 5, 1.5f), 1739.0f, 10.0f, "1.5 moves the middle point up from 632 Hz to about 1.7 kHz");
            expectWithinAbsoluteError (at (3, 5, 1.5f), 8434.0f, 40.0f);
            expectGreaterThan (at (2, 5, 2.0f), at (2, 5, 1.5f), "and 2 further still");
            for (float bias : { 1.0f, 1.5f, 2.0f })
                for (int i = 0; i < 5; ++i)
                {
                    expectWithinAbsoluteError (at (2 * i, 9, bias), at (i, 5, bias), at (i, 5, bias) * 1.0e-4f, "nine points include the five");
                    expectWithinAbsoluteError (at (4 * i, 17, bias), at (i, 5, bias), at (i, 5, bias) * 1.0e-4f, "and seventeen too");
                }
        }

        beginTest ("In the app: it redraws the curve through five points, plays the arrangement, and a preview is heard but not kept");
        {
            CabinEqAudioProcessor processor;
            processor.setPlayConfigDetails (2, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);
            auto profile = processor.getSelectedProfile();
            profile.setBandsOn (false), profile.setCurveOn (true);
            profile.setPoints ({ { 0, 100.0f, 4.0f }, { 1, 1000.0f, 0.0f, 0.0f, 6.0f }, { 2, 9000.0f, -3.0f } });
            const auto before = profile.getBandProfile();

            GuidedCalibrationView view (processor);
            view.setBounds (0, 0, 560, 212);
            expectWithinAbsoluteError (view.getBias(), 1.5f, 0.001f, "the bias starts at 1.5");
            expectEquals (view.getRepetitions(), 4, "and each sound plays four times");
            view.start();

            const auto points = profile.getBandProfile().getPoints();
            expectEquals ((int) points.size(), 5);
            if (points.size() == 5)
            {
                expectWithinAbsoluteError (points[0].freq, 20.0f, 0.01f);
                expectWithinAbsoluteError (points[2].freq, 1739.0f, 10.0f, "bunched towards the top");
                expectWithinAbsoluteError (points[4].freq, 20000.0f, 1.0f);
                for (const auto& point : points)
                {
                    const auto ears = profile.getBandProfile();
                    expectWithinAbsoluteError (ears.sharedDbAt (point.freq, 0), before.sharedDbAt (point.freq, 0), 0.01f, "each ear's curve is where it was, at the vertices");
                    expectWithinAbsoluteError (ears.sharedDbAt (point.freq, 1), before.sharedDbAt (point.freq, 1), 0.01f);
                }
            }

            auto& player = processor.getCalibration();
            expect (player.isPlaying() && player.getMode() == CalibrationPlayer::Mode::sequence);
            player.setRate (CalibrationPlayer::maxRate); // so it doesn't take long to play through

            // Plays sound until this many more of the arrangement's sounds have started
            auto playBursts = [&] (int count)
            {
                const int target = player.getSequenceBurstsPlayed() + count;
                juce::AudioBuffer<float> buffer (2, blockSize);
                for (int guard = 0; guard < 20000 && player.getSequenceBurstsPlayed() < target; ++guard)
                {
                    buffer.clear();
                    player.process (buffer);
                }
            };
            auto press = [&view] (const juce::String& text)
            {
                for (auto* child : view.getChildren())
                    if (auto* button = dynamic_cast<juce::TextButton*> (child))
                        if (button->getButtonText() == text && button->isEnabled() && button->isVisible())
                        {
                            if (button->getClickingTogglesState()) // as a real click would, before its handler runs
                                button->setToggleState (! button->getToggleState(), juce::dontSendNotification);
                            button->onClick();
                            return true;
                        }
                return false;
            };
            XYPad* pad = nullptr;
            for (auto* child : view.getChildren())
                if (auto* found = dynamic_cast<XYPad*> (child))
                    pad = found;
            expect (pad != nullptr && view.getSearch() != nullptr);
            if (pad == nullptr || view.getSearch() == nullptr || points.size() != 5)
                return;
            const auto* search = view.getSearch();

            // The arrangement plays once through before the pad does anything: eight sounds, four times each
            expect (search->getPhase() == GuidedSearch::Phase::introducing);
            expect (! pad->isEnabled(), "the pad's locked while you listen");
            playBursts (32);
            view.poll();
            expect (search->getPhase() == GuidedSearch::Phase::introducing, "thirty-two sounds started isn't the last one finished");
            playBursts (1);
            view.poll();
            expect (search->getPhase() == GuidedSearch::Phase::exploring, "once it's all played through, the pad's yours");
            expect (pad->isEnabled());

            // Drag the pad: the first plane's across is both ears at the fourth vertex
            const float startGain = points[3].gain;
            pad->onChange ({ 0.75f, 0.5f }); // halfway to the right: +15 dB
            expectWithinAbsoluteError (profile.getBandProfile().getPoints()[3].gain, startGain + 15.0f, 0.01f, "you hear (and see) the preview");

            // Hear start, and back
            expect (press ("Hear start"));
            expectWithinAbsoluteError (profile.getBandProfile().getPoints()[3].gain, startGain, 0.01f, "the starting setting");
            expect (search->isHearingStart());
            pad->onChange ({ 0.75f, 0.5f });
            expect (! search->isHearingStart());
            expectWithinAbsoluteError (profile.getBandProfile().getPoints()[3].gain, startGain + 15.0f, 0.01f);

            // More repetitions: the same sounds, each eight times
            view.setRepetitions (8);
            expectEquals (view.getRepetitions(), 8);
            pad->onGestureEnd();
            expect (search->getPhase() == GuidedSearch::Phase::settling);
            playBursts (33);
            view.poll();
            expect (search->getPhase() == GuidedSearch::Phase::settling, "it's twice as long now");
            playBursts (32);
            view.poll();
            expect (search->getPhase() == GuidedSearch::Phase::deciding);
            view.setRepetitions (4);

            // Skip to the fifth plane: both ears at the top vertex, with its difference between the ears
            for (int skipped = 0; skipped < 4; ++skipped)
                expect (press (skipped == 0 ? "No better" : "Skip"), "a turn can be skipped");
            expectWithinAbsoluteError (profile.getBandProfile().getPoints()[3].gain, startGain, 0.01f, "what wasn't kept is gone");
            expect (search->getPhase() == GuidedSearch::Phase::exploring, "the sounds are known by now");

            // All the way up: 30 dB more in the left ear and 30 less in the right, with nothing clipped
            pad->onChange ({ 0.5f, 1.0f });
            auto top = profile.getBandProfile().getPoints()[4];
            expectWithinAbsoluteError (top.gainSplit, 60.0f, 0.01f, "the ears can be the whole range apart, each way");
            expectWithinAbsoluteError (top.gain, points[4].gain, 0.01f, "with the level between them where it was");

            // Spacing the heights another way starts the same sounds again with their new low cuts
            const int playedSoFar = player.getSequenceBurstsPlayed();
            playBursts (3);
            expectGreaterThan (player.getSequenceBurstsPlayed(), playedSoFar);
            player.setHeightSpacing (CalibrationPlayer::HeightSpacing::hertz);
            view.poll();
            expectEquals (player.getSequenceBurstsPlayed(), 0, "from the first of them");
            player.setHeightSpacing (CalibrationPlayer::HeightSpacing::octaves);
            view.poll();

            // Leaving partway through a turn (another mode, or hiding the panel) puts the curve back
            view.leave();
            expectWithinAbsoluteError (profile.getBandProfile().getPoints()[4].gainSplit, points[4].gainSplit, 0.01f, "what was being tried isn't kept");
            expect (! player.isPlaying());
            view.start();
            expect (player.isPlaying() && view.getSearch() == search, "and the search carries on from that turn");

            // Starting over ends the search; a different bias then puts the points somewhere else
            pad->onChange ({ 0.5f, 1.0f });
            view.startOver();
            expect (view.getSearch() == nullptr && ! player.isPlaying());
            expectWithinAbsoluteError (profile.getBandProfile().getPoints()[4].gainSplit, points[4].gainSplit, 0.01f);
            view.setBias (1.0f);
            view.start();
            expectWithinAbsoluteError (profile.getBandProfile().getPoints()[2].freq, 632.5f, 1.0f, "a bias of 1: evenly spaced");
            view.setBias (1.5f);
            view.startOver();
        }
    }
};

static GuidedSearchTests guidedSearchTests;
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
static int writeSnapshot (const juce::File& file, int width, int height, bool channelSpecific, bool showCalibration, bool spotsMode, bool zoomed, bool curveMode, bool split, bool bothLayers, bool editBands)
{
    GuidedCalibrationView::shouldWriteLog = false;
    CabinEqAudioProcessor processor;
    auto profile = processor.getSelectedProfile();
    profile.setBands ({ Band::withQ (0, 32.0f, 0.0f, 0.71f, Band::Type::both, Band::Shape::lowCut),
                        Band::withQ (0, 105.0f, 5.5f, 0.7f, Band::Type::both, Band::Shape::lowShelf),
                        Band::withQ (0, 2300.0f, -3.2f, 1.9f, channelSpecific ? Band::Type::left : Band::Type::both),
                        Band::withQ (0, 5400.0f, 4.0f, 3.0f, Band::Type::both),
                        Band::withQ (0, 9800.0f, -2.5f, 0.7f, Band::Type::both, Band::Shape::highShelf) });
    profile.setVolume (-6.0f);
    profile.setBandsOn (! curveMode || bothLayers), profile.setCurveOn (curveMode);
    processor.setEditingCurve (! editBands);
    if (curveMode)
    {
        BandEqCurve bands;
        bands.updateWithBands (profile.getBandProfile().getBands());
        if (bothLayers)
            profile.setPoints ({ { 0, 60.0f, 0.0f }, { 1, 300.0f, -2.0f }, { 2, 1200.0f, 1.5f }, { 3, 7000.0f, -3.0f }, { 4, 14000.0f, 1.0f } });
        else
            profile.setPoints (CurveResponse::tracing ([&bands] (float f) { return bands.dbAtFrequency (f); }));
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
    if (juce::SystemStats::getEnvironmentVariable ("CABINEQ_SNAPSHOT_FULL_GRID", {}).isNotEmpty())
    {
        processor.parameters.state.setProperty ("calibrationRows", CalibrationPlayer::maxRows, nullptr);
        processor.parameters.state.setProperty ("calibrationColumns", CalibrationPlayer::maxColumns, nullptr);
    }
    if (const auto mode = juce::SystemStats::getEnvironmentVariable ("CABINEQ_SNAPSHOT_MODE", {}); mode.isNotEmpty())
        processor.parameters.state.setProperty ("calibrationMode", mode.getIntValue(), nullptr);
    if (const auto spacing = juce::SystemStats::getEnvironmentVariable ("CABINEQ_SNAPSHOT_SPACING", {}); spacing.isNotEmpty())
        processor.parameters.state.setProperty (CalibrationSettings::idHeightSpacing, spacing.getIntValue(), nullptr); // 0 octaves, 1 mels, 2 Hz
    if (const auto noise = juce::SystemStats::getEnvironmentVariable ("CABINEQ_SNAPSHOT_NOISE", {}); noise.isNotEmpty())
        processor.parameters.state.setProperty ("calibrationNoise", noise.getIntValue(), nullptr);
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
    if (juce::SystemStats::getEnvironmentVariable ("CABINEQ_SNAPSHOT_MODE", {}) == "3")
    {
        // Guided mode, a turn in, with the pad moved
        std::function<GuidedCalibrationView* (juce::Component&)> findGuided = [&] (juce::Component& parent) -> GuidedCalibrationView*
        {
            for (auto* child : parent.getChildren())
            {
                if (auto* view = dynamic_cast<GuidedCalibrationView*> (child))
                    return view;
                if (auto* found = findGuided (*child))
                    return found;
            }
            return nullptr;
        };
        const auto stage = juce::SystemStats::getEnvironmentVariable ("CABINEQ_SNAPSHOT_GUIDED", "explore");
        if (auto* view = findGuided (*editor); view != nullptr && stage != "setup")
        {
            view->start();
            auto& player = processor.getCalibration();
            auto playBursts = [&player] (int count)
            {
                const int target = player.getSequenceBurstsPlayed() + count;
                juce::AudioBuffer<float> buffer (2, 256);
                player.setRate (CalibrationPlayer::maxRate);
                for (int guard = 0; guard < 40000 && player.getSequenceBurstsPlayed() < target; ++guard)
                {
                    buffer.clear();
                    player.process (buffer);
                }
            };
            XYPad* pad = nullptr;
            for (auto* child : view->getChildren())
                if (auto* found = dynamic_cast<XYPad*> (child))
                    pad = found;

            auto click = [view] (const juce::String& text)
            {
                for (auto* child : view->getChildren())
                    if (auto* button = dynamic_cast<juce::TextButton*> (child); button != nullptr && button->getButtonText() == text)
                    {
                        button->onClick();
                        return;
                    }
            };

            // "middle" skips on a pass, to where the arrangement has a layer between two of its sides
            for (int skips = stage == "middle" ? 13 : 0; skips > 0; --skips)
                click ("Skip");

            playBursts (stage == "intro" ? 6 : stage == "middle" ? 22 : 33); // partway through, or right through it
            view->poll();
            if (stage != "intro" && stage != "middle" && pad != nullptr)
            {
                pad->onChange ({ 0.7f, 0.35f });
                playBursts (10);
                view->poll();
                if (stage == "check" || stage == "check2")
                {
                    pad->onGestureEnd();
                    playBursts (33);
                    view->poll();
                    click ("Better");
                    playBursts (38); // before has played; now after
                    view->poll();
                    if (stage == "check2")
                    {
                        click ("Same");
                        playBursts (14); // the second check's before
                        view->poll();
                    }
                }
            }
            juce::MessageManager::getInstance()->runDispatchLoopUntil (50);
        }
    }
    if (auto* graph = dynamic_cast<CabinPeqGraph*> (findGraph (*editor)))
    {
        if (juce::SystemStats::getEnvironmentVariable ("CABINEQ_SNAPSHOT_SHAPE", {}).isNotEmpty())
        {
            // Three points, the middle one different in each ear, all selected: the shape pads
            processor.getSelectedProfile().setPoints ({ { 0, 300.0f, 0.0f }, { 1, 1500.0f, 6.0f, 0.5f, 4.0f }, { 2, 6000.0f, -1.0f } });
            graph->refresh();
            graph->keyPressed (juce::KeyPress ('a', juce::ModifierKeys::commandModifier, 0));
        }
        else
        {
            graph->focusBand (2);
        }
    }
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
                              argc >= 6 && juce::String (argv[5]).contains ("split"),
                              argc >= 6 && juce::String (argv[5]).contains ("both"),
                              argc >= 6 && juce::String (argv[5]).contains ("editbands"));

    CabinEqProfileManager::shouldBackUpOldState = false;
    GuidedCalibrationView::shouldWriteLog = false;

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runTestsInCategory ("CabinEQ");

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    std::cout << (failures == 0 ? "All tests passed" : juce::String (failures) + " failures") << std::endl;
    return failures == 0 ? 0 : 1;
}
