/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "CabinEqAudioProcessor.h"
#include "CabinEqProcessorEditor.h"
#include <chrono>

//==============================================================================
CabinEqAudioProcessor::CabinEqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters (*this, nullptr, "Params", createParameterLayout()),
                          cabinEqProfileManager (parameters)

#endif
{
//    // Step 0 - Grids
//    SpatialStep step0;
//    Glyph twoByTwo ({ { 100, -1 }, { 100, 1 }, { 5000, -1 }, { 5000, 1 }});
//    step0.addStage (twoByTwo);
//    
//    // Step I - Intervals
////    IntelligibilityStep step1;
//    MelodicNotes wideScale = MelodicNotes ({ -12, 0, 5, 7, 9, 7, 11, 14 }, 800.0f);
////    step1.addStage (wideScale.withTransposition (-48));
////    step1.addStage (wideScale.withTransposition (-24));
////    step1.addStage (wideScale.withTransposition (0));
////    step1.addStage (wideScale.withTransposition (24));
////    step1.addStage (wideScale.withTransposition (48));
//    
//    IntelligibilityStep step1;
//    MelodicNotes majorFifth = MelodicNotes ({ 0, 4, 7, 4 }, 500.0f);
//    MelodicNotes majorThird = MelodicNotes ({ 0, 3, 7, 3 }, 500.0f);
//    MelodicNotes octave = MelodicNotes ({ 0, 12 }, 500.0f);
//    MelodicNotes majorSecond = MelodicNotes ({ 0, 2 }, 500.0f);
//    step1.addStage (majorFifth);
//    step1.addStage (majorThird);
//    step1.addStage (octave);
//    step1.addStage (majorSecond);
//    step1.addStage (wideScale);
//    
////    // Step II - Three Stack
////    SpatialStep step2;
////    MelodicNotes threeStack =
////    MelodicNotes::withFreqs ({ 200, 1000, 5000 })
////        .withBandwidth (2.0f);
////    step2.addStage (threeStack);
////    step2.addStage (threeStack.withPan (-1));
////    step2.addStage (threeStack.withPan (1));
//    
//    // Step II - square separation
//    SpatialStep step2;
//    MelodicNotes square =
//    MelodicNotes::withFreqs ({ 200, 2000, 200, 2000 })
//        .withPans ({ -0.5, 0.5, 0.5, -0.5 })
//        .withBandwidth (1.5f);
//    MelodicNotes sequence =
//    MelodicNotes::withFreqs ({ 200, 4000 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.1);
//    step2.addStage (square);
//    step2.addStage (sequence);
//    
//    MelodicNotes horizontal =
//    MelodicNotes::withFreqs ({ 500, 500 })
//        .withBandwidth (1.0f)
//        .withPans ({ -0.5, 0.5 })
//        .withNoteDurationInSeconds (0.1);
//    step2.addStage (horizontal);
////    step2.addStage (sequence);
//    
//    // Step III - Solfeggietto
//    IntelligibilityStep step3;
//    MelodicNotes solfeggietto =
//    MelodicNotes ({ 0, -3, 0, 4, 9, 12, 11, 9, 8, 4, 8, 11, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 23, 21, 20, 18, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 20, 16, 20, 23, 28, 26, 24, 23, 24, 21, 24, 28, 33, 36, 35, 33, 35, 33, 32, 30, 28, 26, 24, 23, 24, 21, 16, 12, 9, 33, 28, 24, 29, 2, 5, 9, 14, 17, 21, 24, 23, 19, 14, 11, 7, 31, 26, 23, 28, 0, 4, 7, 12, 16, 19, 23, 21, 18, 17, 18, 21, 18, 17, 18, 24, 21, 16, 18, 24, 21, 16, 18, 23, 21, 15, 18, 30, 21, 15, 18, 27, 21, 11, 18, 21, 18, 15, 11, 19, -8, -5, -1, 4, 7, 6, 4, 3, -1, 3, 6, 11, 9, 7, 6, 7, 4, 7, 11, 16, 19, 18, 16, 18, 16, 15, 13, 11, 9, 7, 6, 7, 4, 7, 11, 16, 19, 18, 16, 15, 11, 15, 18, 23, 21, 19, 18, 19, 16, 19, 23, 28, 31, 30, 28, 30, 28, 27, 25, 23, 21, 19, 18, 19, 4, -8, 16, 19, 23, 28, 23, 19, 16, 2, -10, 28, 23, 20, 16, 20, 23, 28, 21, 12, 16, 28, 16, 21, 12, 16, 28, 16, 20, 11, 16, 26, 16, 20, 11, 16, 26, 16, 24, 9, -3, 21, 24, 28, 33, 28, 24, 21, 7, -5, 33, 28, 25, 21, 25, 28, 33, 26, 17, 21, 33, 21, 26, 17, 21, 33, 21, 25, 16, 21, 31, 21, 25, 16, 21, 31, 21, 29, -10, -7, -3, 2, 5, 4, 2, 1, -3, 1, 4, 9, 7, 5, 4, 5, 2, 5, 9, 14, 17, 16, 14, 16, 14, 13, 11, 9, 7, 5, 4, 5, 2, 5, 9, 14, 17, 16, 14, 13, 9, 13, 16, 21, 19, 17, 16, 17, 14, 17, 21, 26, 29, 28, 26, 28, 26, 25, 23, 21, 19, 17, 16, 17, 17, 26, 21, 17, 14, 14, 21, 17, 14, 9, 9, 17, 14, 9, 5, 5, 14, 9, 5, -2, -14, 29, 26, 25, 26, 28, 26, 25, 26, -3, -15, 17, 14, 13, 14, 16, 14, 13, 14, -4, -16, 35, 26, 28, 29, 28, 26, 24, 23, 24, -3, -15, 28, 33, 28, 31, 2, 29, 28, 26, 24, 4, -8, 23, 24, 23, 21, 23, 21, 12, 16, 28, 16, 21, 12, 16, 28, 16, 20, 11, 16, 26, 16, 20, 11, 16, 26, 16, 19, 9, 16, 25, 16, 19, 9, 16, 25, 16, 18, 14, 24, 33, 24, 18, 14, 24, 33, 24, 17, 7, 14, 23, 14, 17, 7, 14, 23, 14, 16, 12, 22, 31, 22, 16, 12, 22, 31, 22, 15, 5, 12, 21, 12, 15, 5, 12, 21, 12, 12, 3, 21, 33, 21, 12, 3, 21, 33, 21, 12, 4, 21, 24, 28, 33, 28, 24, 21, 28, 24, 21, 16, 26, -8, 23, 20, 14, 12, -3, 0, 4, 9, 12, 11, 9, 8, 4, 8, 11, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 23, 21, 20, 18, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 20, 16, 20, 23, 28, 26, 24, 23, 24, 21, 24, 28, 33, 36, 35, 32, 33, 28, 24, 23, 21, 16, 12, 11, 9}, 800.0f);
//    step3.addStage (solfeggietto.withTranspositionInOctaves (-2));
//    step3.addStage (solfeggietto.withTranspositionInOctaves (0));
//    step3.addStage (solfeggietto.withTranspositionInOctaves (2));
//    
//    // Step IV - fancy pattern
//    SpatialStep step4;
//    float lowFreq = 20;
//    float hiFreq = 15000;
//    float sampleRate = 44100; // for now, but this isn't ideal
//    SweepPattern forwardSlash ({{ lowFreq, -1 }, { hiFreq, 1 }}, 2.0f, sampleRate);
//    SweepPattern downwardsRight ({{ hiFreq, 1 }, { lowFreq, 1 }}, 2.0f, sampleRate);
//    SweepPattern backslash ({{ lowFreq, 1 }, { hiFreq, -1 }}, 2.0f, sampleRate);
//    SweepPattern downwardsLeft ({{ hiFreq, -1 }, { lowFreq, -1 }}, 2.0f, sampleRate);
//    SweepPattern zigZag ({{ 20, -1 }, { 200, 1 }, { 2000, -1 }, { 200, 1 }, { 20, -1 }}, 2.0f, sampleRate);
//    step4.addStage (forwardSlash);
//    step4.addStage (downwardsRight);
//    step4.addStage  (backslash);
//    step4.addStage (downwardsLeft);
//    step4.addStage (zigZag);
//    
//    // Initialize QualityStepManager steps imperatively
//    qualityStepManager.addSpatialStep (step0);
//    qualityStepManager.addIntelligibilityStep (step1);
//    qualityStepManager.addSpatialStep (step2);
//    qualityStepManager.addIntelligibilityStep (step3);
//    qualityStepManager.addSpatialStep (step4);
//    
//    playbackManager.setQualityStep (qualityStepManager.getCurrStep());
}

CabinEqAudioProcessor::~CabinEqAudioProcessor()
{
}

//==============================================================================
const juce::String CabinEqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CabinEqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CabinEqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CabinEqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CabinEqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CabinEqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CabinEqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CabinEqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CabinEqAudioProcessor::getProgramName (int index)
{
    return {};
}

void CabinEqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CabinEqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    playbackManager.prepare (spec);
    
    // Define glyphs
    Glyph xGlyph ({ Stroke ({{ -1, -1 }, { 1, 1 }}), Stroke ({{ 1, -1 }, { -1, 1 }})});
    Glyph diamondPlusGlyph ({ Stroke ({{ 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }}), Stroke ({{ 0, -1 }, { 0, 1 }}), Stroke ({{ -1, 0 }, { 1, 0 }})});
    Glyph fourXGlyph ({ Stroke ({{ -1, -1 }, { 0, 0 }}), Stroke ({{ 0, -1 }, { -1, 0 }}),
        Stroke ({{ 0, -1 }, { 1, 0 }}), Stroke ({{ 1, -1 }, { 0, 0 }}),
        Stroke ({{ -1, 0 }, { 0, 1 }}), Stroke ({{ 0, 0 }, { -1, 1 }}),
        Stroke ({{ 0, 0 }, { 1, 1 }}), Stroke ({{ 1, 0 }, { 0, 1 }})});
    Glyph triangleStrokes ({ Stroke ({{ -1, -1 }, { -0.66, 1 }, { -0.33, -1 }, { 0, 1 }, { 0.33, -1 }, { 0.66, 1 }, { 1, -1 }})});
    Glyph edgeStrokes ({ Stroke ({{ -1, 1 }, { -0.5, 0.5 }, { -1, 0 }, { -0.5, -0.5 }, { -1, -1 }}),
        Stroke ({{ 1, 1 }, { 0.5, 0.5 }, { 1, 0 }, { 0.5, -0.5 }, { 1, -1 }})});
    Glyph spiralGlyph ({
        Stroke ({{ -1, -1 }, { -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 }}),
        Stroke ({{ 1, 1 }, { 1, 0 }, { 0, 0 }, { 0, -1 }, { -1, -1 }})
    });
    Glyph complexFractalGlyph ({
        Stroke ({{ -1, -1 }, { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 }}),  // Outer square boundary
        Stroke ({{ -0.5, -0.5 }, { -0.5, 0.5 }, { 0.5, 0.5 }, { 0.5, -0.5 }, { -0.5, -0.5 }}),  // Inner square
        Stroke ({{ -1, 0 }, { 1, 0 }}),  // Horizontal line through center
        Stroke ({{ 0, -1 }, { 0, 1 }}),  // Vertical line through center
        Stroke ({{ -1, -0.5 }, { 1, -0.5 }}),  // Additional horizontal lines
        Stroke ({{ -1, 0.5 }, { 1, 0.5 }}),
        Stroke ({{ -0.5, -1 }, { -0.5, 1 }}),  // Additional vertical lines
        Stroke ({{ 0.5, -1 }, { 0.5, 1 }})
    });
    
    Glyph floatingSquaresGlyph ({
        Stroke ({{ -0.8, -0.8 }, { -0.6, -0.8 }, { -0.6, -0.6 }, { -0.8, -0.6 }, { -0.8, -0.8 }}),  // Bottom-left square
        Stroke ({{ 0.6, -0.8 }, { 0.8, -0.8 }, { 0.8, -0.6 }, { 0.6, -0.6 }, { 0.6, -0.8 }}),    // Bottom-right square
        Stroke ({{ -0.8, 0.6 }, { -0.6, 0.6 }, { -0.6, 0.8 }, { -0.8, 0.8 }, { -0.8, 0.6 }}),    // Top-left square
        Stroke ({{ 0.6, 0.6 }, { 0.8, 0.6 }, { 0.8, 0.8 }, { 0.6, 0.8 }, { 0.6, 0.6 }}),        // Top-right square
        Stroke ({{ -0.1, -0.1 }, { 0.1, -0.1 }, { 0.1, 0.1 }, { -0.1, 0.1 }, { -0.1, -0.1 }})   // Center square
    });
    
    Glyph distributedSquaresGlyph ({
        Stroke ({{ -0.9, -0.9 }, { -0.7, -0.9 }, { -0.7, -0.7 }, { -0.9, -0.7 }, { -0.9, -0.9 }}),  // Bottom-left
        Stroke ({{ 0.7, -0.9 }, { 0.9, -0.9 }, { 0.9, -0.7 }, { 0.7, -0.7 }, { 0.7, -0.9 }}),    // Bottom-right
        Stroke ({{ -0.9, 0.7 }, { -0.7, 0.7 }, { -0.7, 0.9 }, { -0.9, 0.9 }, { -0.9, 0.7 }}),    // Top-left
        Stroke ({{ 0.7, 0.7 }, { 0.9, 0.7 }, { 0.9, 0.9 }, { 0.7, 0.9 }, { 0.7, 0.7 }}),        // Top-right
        Stroke ({{ -0.2, -0.3 }, { 0.0, -0.3 }, { 0.0, -0.1 }, { -0.2, -0.1 }, { -0.2, -0.3 }}), // Center-left
        Stroke ({{ 0.2, 0.2 }, { 0.4, 0.2 }, { 0.4, 0.4 }, { 0.2, 0.4 }, { 0.2, 0.2 }}),        // Center-right
        Stroke ({{ -0.5, 0.0 }, { -0.3, 0.0 }, { -0.3, 0.2 }, { -0.5, 0.2 }, { -0.5, 0.0 }}),   // Middle-left
        Stroke ({{ 0.3, -0.5 }, { 0.5, -0.5 }, { 0.5, -0.3 }, { 0.3, -0.3 }, { 0.3, -0.5 }})    // Middle-right
    });
    Glyph graphPaperSquaresGlyph ({
        Stroke ({{ -0.8, -0.8 }, { -0.8, 0.0 }, { 0.0, 0.0 }, { 0.0, -0.8 }, { -0.8, -0.8 }}),  // Bottom-left square
        Stroke ({{ -0.4, -0.4 }, { -0.4, 0.4 }, { 0.4, 0.4 }, { 0.4, -0.4 }, { -0.4, -0.4 }}),  // Center square
        Stroke ({{ 0.0, 0.0 }, { 0.0, 0.8 }, { 0.8, 0.8 }, { 0.8, 0.0 }, { 0.0, 0.0 }}),       // Top-right square
        Stroke ({{ -0.8, 0.2 }, { -0.8, 1.0 }, { 0.0, 1.0 }, { 0.0, 0.2 }, { -0.8, 0.2 }}),    // Top-left square
        Stroke ({{ 0.2, -1.0 }, { 0.2, -0.2 }, { 1.0, -0.2 }, { 1.0, -1.0 }, { 0.2, -1.0 }})   // Bottom-right square
    });
    
    Glyph triforceGlyph({
        Stroke({{-0.5, 0}, {-1, -0.866}, {0, -0.866}, {-0.5, 0}}), // Bottom-left triangle
        Stroke({{0.5, 0}, {0, -0.866}, {1, -0.866}, {0.5, 0}}),    // Bottom-right triangle
        Stroke({{0, 0.866}, {-0.5, 0}, {0.5, 0}, {0, 0.866}})      // Top triangle
    });
    
    Glyph gridGlyph({
        // Row 1
        Stroke({{-0.9, 0.9}, {-0.7, 0.9}, {-0.7, 0.7}, {-0.9, 0.7}, {-0.9, 0.9}}), // Top-left square
        Stroke({{-0.4, 0.9}, {-0.2, 0.9}, {-0.2, 0.7}, {-0.4, 0.7}, {-0.4, 0.9}}), // Top-middle square
        Stroke({{0.1, 0.9}, {0.3, 0.9}, {0.3, 0.7}, {0.1, 0.7}, {0.1, 0.9}}),     // Top-right square

        // Row 2
        Stroke({{-0.9, 0.4}, {-0.7, 0.4}, {-0.7, 0.2}, {-0.9, 0.2}, {-0.9, 0.4}}), // Middle-left square
        Stroke({{-0.4, 0.4}, {-0.2, 0.4}, {-0.2, 0.2}, {-0.4, 0.2}, {-0.4, 0.4}}), // Center square
        Stroke({{0.1, 0.4}, {0.3, 0.4}, {0.3, 0.2}, {0.1, 0.2}, {0.1, 0.4}}),     // Middle-right square

        // Row 3
        Stroke({{-0.9, -0.1}, {-0.7, -0.1}, {-0.7, -0.3}, {-0.9, -0.3}, {-0.9, -0.1}}), // Bottom-left square
        Stroke({{-0.4, -0.1}, {-0.2, -0.1}, {-0.2, -0.3}, {-0.4, -0.3}, {-0.4, -0.1}}), // Bottom-middle square
        Stroke({{0.1, -0.1}, {0.3, -0.1}, {0.3, -0.3}, {0.1, -0.3}, {0.1, -0.1}})      // Bottom-right square
    });
    
    glyphManager.addGlyphs ({ xGlyph, diamondPlusGlyph, fourXGlyph, triangleStrokes, edgeStrokes, spiralGlyph, complexFractalGlyph, floatingSquaresGlyph, distributedSquaresGlyph, graphPaperSquaresGlyph, triforceGlyph, gridGlyph });
    playbackManager.setGlyph (getCurrGlyph());
}

void CabinEqAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CabinEqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CabinEqAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Clear buffer before handing it off to playbackManager
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    playbackManager.processBlock (buffer);
}

//==============================================================================
bool CabinEqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CabinEqAudioProcessor::createEditor()
{
    return new CabinEqProcessorEditor (*this);
}

//==============================================================================
void CabinEqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr <juce::XmlElement> xml (state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CabinEqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr <juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get())
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            cabinEqProfileManager.initProfiles();
            
            if (! hasLoadedData)
            {
                for (auto listener : listeners)
                    if (listener != nullptr)
                        listener->didLoadData();
                hasLoadedData = true;
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CabinEqAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout CabinEqAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::NormalisableRange<float> range (-1.0f, 1.0f, 0.01f);
    
    juce::String paramID = "dummyParam";
    return { std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (paramID, 1), paramID, range, 0.0f) };
}

//==============================================================================
void CabinEqAudioProcessor::setVolume (float volume)
{
    playbackManager.setVolume (volume);
}

void CabinEqAudioProcessor::setIsFilterOn (bool isFilterOn)
{
    playbackManager.setIsFilterOn (isFilterOn);
}

void CabinEqAudioProcessor::setIsPlaying (bool isPlaying)
{
    playbackManager.setIsPlayingNoise (isPlaying);
}

void CabinEqAudioProcessor::setSpeedFactor (float speedFactor)
{
    playbackManager.setSpeedFactor (speedFactor);
}

void CabinEqAudioProcessor::setBandwidth (float bandwidth)
{
    playbackManager.setBandwidth (bandwidth);
}

void CabinEqAudioProcessor::addProfile (juce::String profileName)
{
    cabinEqProfileManager.addProfile (profileName);
}

void CabinEqAudioProcessor::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    cabinEqProfileManager.addDuplicateProfile (profileName, oldProfileName);
}

void CabinEqAudioProcessor::removeProfile (juce::String profileName)
{
    cabinEqProfileManager.removeProfile (profileName);
}

void CabinEqAudioProcessor::renameProfile (juce::String profileName, juce::String newProfileName)
{
    cabinEqProfileManager.renameProfile (profileName, newProfileName);
}

void CabinEqAudioProcessor::setProfileVolume (juce::String profileName, float masterVolume)
{
    cabinEqProfileManager.setProfileVolume (profileName, masterVolume);
}

const std::vector<juce::String> CabinEqAudioProcessor::getProfileNames() const
{
    return cabinEqProfileManager.getProfileNames();
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqAudioProcessor::getProfileNamed (juce::String profileName) const
{
    return cabinEqProfileManager.getProfileNamed (profileName);
}

BandProfile CabinEqAudioProcessor::getBandProfile (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getBandProfile();
    return BandProfile ({}, 0.0f, 0.0f, 0.0f);
}

std::optional<juce::String> CabinEqAudioProcessor::getLastSelectedProfileName()
{
    return cabinEqProfileManager.getLastSelectedProfileName();
}

void CabinEqAudioProcessor::setLastSelectedProfileName (juce::String profileName)
{
    cabinEqProfileManager.setLastSelectedProfileName (profileName);
}

void CabinEqAudioProcessor::updateFilter (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        playbackManager.updateFilterWithBandProfile (profile->get().getBandProfile());
}

int CabinEqAudioProcessor::addBand (const float freq, const float ampl, const float bandwidth, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        return profile->get().addBand (freq, ampl, bandwidth);
    }
        
    return -1;
}

void CabinEqAudioProcessor::updateBand (const int id, const float freq, const float ampl, const float bandwidth, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().updateBand (id, freq, ampl, bandwidth);
}

void CabinEqAudioProcessor::removeBand (const int id, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().removeBand (id);
}

void CabinEqAudioProcessor::addListener (Listener* listener)
{
    this->listeners.push_back (listener);
}

void CabinEqAudioProcessor::removeListener()
{
    // VERY BAD FIX THIS: eh whatever
}

bool CabinEqAudioProcessor::hasNextGlyph()
{
    return glyphManager.hasNext();
}

bool CabinEqAudioProcessor::hasPrevGlyph()
{
    return glyphManager.hasPrev();
}

void CabinEqAudioProcessor::goToNextGlyph()
{
    glyphManager.goToNext();
    playbackManager.setGlyph (getCurrGlyph());
}

void CabinEqAudioProcessor::goToPrevGlyph()
{
    glyphManager.goToPrev();
    playbackManager.setGlyph (getCurrGlyph());
}

Glyph CabinEqAudioProcessor::getCurrGlyph()
{
    std::cout << "Getting curr glyph" << std::endl;
    return glyphManager.getCurrGlyph();
}

void CabinEqAudioProcessor::setSizeFactor (float sizeFactor)
{
    glyphManager.setSizeFactor (sizeFactor);
}

void CabinEqAudioProcessor::setCenterPos (juce::Point<float> centerPos)
{
    glyphManager.setCenterPos (centerPos);
}

float CabinEqAudioProcessor::getSizeFactor() const
{
    return glyphManager.getSizeFactor();
}

juce::Point<float> CabinEqAudioProcessor::getCenterPos() const
{
    return glyphManager.getCenterPos();
}

float CabinEqAudioProcessor::getCurrPlayingTime()
{
    return playbackManager.getCurrPlayingTime();
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqAudioProcessor::profileNamed (juce::String profileName) const
{
    return cabinEqProfileManager.getProfileNamed (profileName);
}
