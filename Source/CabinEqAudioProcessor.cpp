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
    
//    // Sweeps
    SpatialStep sweeps;
    SweepPattern sPattern ({{ 50, -1 }, { 400, 1 }, { 3200, -1 }, { 10000, 1 }, { 3200, -1 }, { 400, 1 }}, 2.0f, sampleRate);
    SweepPattern confounding1 ({{ 800, -1.0 }, { 400, 0.0 }, { 200, -1.0 }}, 1.0f, sampleRate);
    SweepPattern confounding2 ({{ 6400, 1.0 }, { 3200, 0.0 }, { 1600, 1.0 }}, 1.0f, sampleRate);
    SweepPattern confounding3 ({{ 18000, -1.0 }, { 12800, 0.0 }, { 6400, -1.0 }}, 1.0f, sampleRate);
    SweepPattern confounding4 ({{ 50, 1.0 }, { 50, 0.5 }}, 1.0f, sampleRate);
    SweepPattern sPatternRev ({{ 50, 1 }, { 400, -1 }, { 3200, 1 }, { 12800, -1 }, { 3200, 1 }, { 400, -1 }}, 2.0f, sampleRate);
    SweepPattern confounding1Rev ({{ 400, 1.0 }, { 400, 0.5 }}, 1.0f, sampleRate);
    SweepPattern confounding2Rev ({{ 3200, -1.0 }, { 3200, -0.5 }}, 1.0f, sampleRate);
    SweepPattern confounding3Rev ({{ 12800, 1.0 }, { 12800, 0.5 }}, 1.0f, sampleRate);
    SweepPattern confounding4Rev ({{ 50, -1.0 }, { 50, -0.5 }}, 1.0f, sampleRate);
    
    SweepPattern smallerPattern ({{ 20, -0.5 }, { 160, 0.5 }, { 1280, -0.5 }, { 10000, 0.5 }, { 1280, -0.5 }, { 160, 0.5 } }, 2.0f, sampleRate);
    SweepPattern obstacle1 ({{ 160, 0 }, { 160, 0 }}, 2.0f, sampleRate);
    SweepPattern obstacle2 ({{ 1280, 0 }, { 1280, 0 }}, 2.0f, sampleRate);
    SweepPattern obstacle3 ({{ 10000, 0 }, { 10000, 0 }}, 2.0f, sampleRate);
    sweeps.addStage (Glyph ({ sPattern, confounding1, confounding2, confounding3, confounding4 }));
    sweeps.addStage (Glyph ({ sPatternRev, confounding1Rev, confounding2Rev, confounding3Rev, confounding4Rev }));
//    sweeps.addStage (Glyph ({ smallerPattern, obstacle1, obstacle2, obstacle3 }));
    
//    // Diamond Sweep
//    SpatialStep sweeps;
//    SweepPattern diamond ({{ 100, 0 }, { 1000, 1 }, { 10000, 0 }, { 1000, -1 }}, 2.0f, sampleRate);
//    SweepPattern corner1 ({{ 100, -1 }, { 100, -1 }}, 1.0f, sampleRate);
//    SweepPattern corner2 ({{ 10000, -1 }, { 10000, -1 }}, 1.0f, sampleRate);
//    SweepPattern corner3 ({{ 100, 1 }, { 100, 1 }}, 1.0f, sampleRate);
//    SweepPattern corner4 ({{ 10000, 1 }, { 10000, 1 }}, 1.0f, sampleRate);
//    SweepPattern corner5 ({{ 1000, 0 }, { 1000, 0 }}, 1.0f, sampleRate);
//    sweeps.addStage (Glyph ({ diamond, corner1, corner2, corner3, corner4 }));
    
//    // Step Triangle
//    SpatialStep triangleStep;
//    float bandwidth = 1.0f;
//    MelodicNotes above = MelodicNotes::withFreqs ({ 5000 }).withPan (0).withNoteDurationInSeconds (0.1f).withBandwidth (bandwidth);
//    MelodicNotes lowerLeft = MelodicNotes::withFreqs ({ 200 }).withPan (-1).withNoteDurationInSeconds (0.09f).withBandwidth (bandwidth);
//    MelodicNotes lowerRight = MelodicNotes::withFreqs ({ 200 }).withPan (1).withNoteDurationInSeconds (0.11f).withBandwidth (bandwidth);
//    MelodicNotes center = MelodicNotes::withMelodicPattern ({ 1, 1, 1, 0 }, { 1000.0f }, bandwidth, { 0 }).withPan (0).withNoteDurationInSeconds (0.2f);
//    Glyph triangleGlyph = Glyph({ above, lowerLeft, lowerRight, center });
//    Glyph triangleSweep = Glyph(SweepPattern ({ {200, -1}, {5000, 0}, {200, 1}, { 200, -1 }}, 1.0f, sampleRate));
//    triangleStep.addStage (triangleGlyph);
//    triangleStep.addStage (triangleSweep);
    
    // Patterns
//    PatternsStep patterns;
//    FauxMusicPattern fauxMusicPattern ({{ 1, 0, 1, 0 }, { 1, 1, 0, 0 }, { 0, 0, 1, 1 }}, {{ 20, 400 }, { 200, 4000 }, { 2000, 20000 }});
//    patterns.addStage (fauxMusicPattern);
//    std::cout << "fauxMusicPattern size: " << fauxMusicPattern.getNumPatterns() << std::endl;
    
    // X Pattern
    SpatialStep xPattern;
    float xBandwidth = 2.0f;
    MelodicNotes diagonal1 =
    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0 }, { 50, 100, 200, 400, 800, 1600, 3200, 6400 }, 2.0f, { -1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1 });
    xPattern.addStage (Glyph ({ diagonal1 }));
    
    // SpatialStep
    SpatialStep higher;
    float bandwidth = 2.0f;
    float freqFactor = 2.0f;
    MelodicNotes lowPattern = MelodicNotes::withFreqs ({ 100, 400, 1600, 6400 }).withCyclingBandwidths ({ bandwidth, 0.0f, bandwidth, 0.0f });
    MelodicNotes midPattern = MelodicNotes::withFreqs ({ 300, 900, 2700, 8100  }).withCyclingBandwidths ({ bandwidth, 0.0f, 0.0f, bandwidth });
    MelodicNotes hiPattern = MelodicNotes::withFreqs ({ 500, 1500, 4500, 1500 }).withCyclingBandwidths ({ bandwidth, bandwidth, 0.0f, 0.0f });
    
    SweepPattern pitchPattern = SweepPattern ({{ 0.33f, 0 }, { 3.0f, 0 }}, 5.0f, sampleRate);
    Glyph wrinkleFinder = Glyph ({ lowPattern, midPattern, hiPattern });
    Glyph lowWrinkleFinder = Glyph ({ lowPattern });
    Glyph midWrinkleFinder = Glyph ({ midPattern });
    Glyph highWrinkleFinder = Glyph ({ hiPattern });
    
    wrinkleFinder.setPitchPattern (pitchPattern);
    lowWrinkleFinder.setPitchPattern (pitchPattern);
    midWrinkleFinder.setPitchPattern (pitchPattern);
    highWrinkleFinder.setPitchPattern (pitchPattern);
    
    higher.addStage (lowWrinkleFinder);
    higher.addStage (midWrinkleFinder);
    higher.addStage (highWrinkleFinder);
    higher.addStage (wrinkleFinder);
    higher.addStage (wrinkleFinder.withPan (1));
    higher.addStage (wrinkleFinder.withPan (-1));
    
//    // Patterns
//    PatternsStep patterns;
//    FauxMusicPattern fauxMusicPattern ({{ 1, 0, 1, 0 }, { 1, 1, 0, 0 }, { 0, 0, 1, 1 }}, {{ 20, 400 }, { 200, 4000 }, { 2000, 20000 }});
//    patterns.addStage (fauxMusicPattern);
//    std::cout << "fauxMusicPattern size: " << fauxMusicPattern.getNumPatterns() << std::endl;
//    
//    // SpatialStep
//    SpatialStep higher;
//    float bandwidth = 2.0f;
//    float freqFactor = 2.0f;
//    MelodicNotes lowPattern = MelodicNotes::withFreqs ({ 50 * freqFactor }).withCyclingBandwidths ({ bandwidth, 0.0f, bandwidth, 0.0f });
//    MelodicNotes midPattern = MelodicNotes::withFreqs ({ 500 * freqFactor }).withCyclingBandwidths ({ bandwidth, 0.0f, 0.0f, bandwidth });
//    MelodicNotes hiPattern = MelodicNotes::withFreqs ({ 5000 * freqFactor }).withCyclingBandwidths ({ bandwidth, bandwidth, 0.0f, 0.0f });
//    
//    SweepPattern pitchPattern = SweepPattern ({{ 0.33f, 0 }, { 3.0f, 0 }}, 5.0f, sampleRate);
//    Glyph wrinkleFinder = Glyph ({ lowPattern, midPattern, hiPattern });
//    Glyph lowWrinkleFinder = Glyph ({ lowPattern });
//    Glyph midWrinkleFinder = Glyph ({ midPattern });
//    Glyph highWrinkleFinder = Glyph ({ hiPattern });
//    
//    wrinkleFinder.setPitchPattern (pitchPattern);
//    lowWrinkleFinder.setPitchPattern (pitchPattern);
//    midWrinkleFinder.setPitchPattern (pitchPattern);
//    highWrinkleFinder.setPitchPattern (pitchPattern);
//    
//    higher.addStage (lowWrinkleFinder);
//    higher.addStage (midWrinkleFinder);
//    higher.addStage (highWrinkleFinder);
//    higher.addStage (wrinkleFinder);
//    higher.addStage (wrinkleFinder.withPan (1));
//    higher.addStage (wrinkleFinder.withPan (-1));
    
//    MelodicNotes lowPatternHigh = MelodicNotes::withFreqs ({ 2000 }).withCyclingBandwidths ({ 0.3f, 0.0f, 0.3f, 0.0f });
//    MelodicNotes midPatternHigh = MelodicNotes::withFreqs ({ 4000 }).withCyclingBandwidths ({ 0.3f, 0.0f, 0.0f, 0.3f });
//    MelodicNotes hiPatternHigh = MelodicNotes::withFreqs ({ 8000 }).withCyclingBandwidths ({ 0.3f, 0.3f, 0.0f, 0.0f });
    
//    // SpatialStep
    SpatialStep higher2;
////    float bandwidth = 2.5f;
////    float freqFactor = 1.3;
//    MelodicNotes lowPattern2 = MelodicNotes::withFreqs ({ 200 }).withCyclingBandwidths ({ 3.0f, 0.0f, 3.0f, 0.0f });
//    MelodicNotes midPattern2 = MelodicNotes::withFreqs ({ 5000 }).withCyclingBandwidths ({ 0.5f, 0.0f, 0.0f, 0.5f });
//    MelodicNotes hiPattern2 = MelodicNotes::withFreqs ({ 7500 }).withCyclingBandwidths ({ 0.5f, 0.5f, 0.0f, 0.0f });
//    
//    SweepPattern pitchPattern2 = SweepPattern ({{ 0.33f, 0 }, { 3.0f, 0 }}, 5.0f, sampleRate);
//    Glyph wrinkleFinder2 = Glyph ({ lowPattern2, midPattern2, hiPattern2 });
//    Glyph lowWrinkleFinder2 = Glyph ({ lowPattern2 });
//    Glyph midWrinkleFinder2 = Glyph ({ midPattern2 });
//    Glyph highWrinkleFinder2 = Glyph ({ hiPattern2 });
//    
//    wrinkleFinder2.setPitchPattern (pitchPattern);
//    lowWrinkleFinder2.setPitchPattern (pitchPattern);
//    midWrinkleFinder2.setPitchPattern (pitchPattern);
//    highWrinkleFinder2.setPitchPattern (pitchPattern);
//    
//    higher2.addStage (lowWrinkleFinder2);
//    higher2.addStage (midWrinkleFinder2);
//    higher2.addStage (highWrinkleFinder2);
//    higher2.addStage (wrinkleFinder2);
//    higher2.addStage (wrinkleFinder2.withPan (1));
//    higher2.addStage (wrinkleFinder2.withPan (-1));
    
    MelodicNotes lowPatternHigh = MelodicNotes::withFreqs ({ 100 }).withCyclingBandwidths ({ 2.5f, 0.0f, 0.3f, 0.0f });
    MelodicNotes midPatternHigh = MelodicNotes::withFreqs ({ 500 }).withCyclingBandwidths ({ 0.3f, 0.0f, 0.0f, 0.3f });
    MelodicNotes hiPatternHigh = MelodicNotes::withFreqs ({ 2500 }).withCyclingBandwidths ({ 2.5f, 0.3f, 0.0f, 0.0f });
    Glyph highWrinkleFinder2 = Glyph ({ lowPatternHigh, midPatternHigh, hiPatternHigh });
    Glyph lowWrinkleFinder2 = Glyph ({ lowPatternHigh });
    Glyph midWrinkleFinder2 = Glyph ({ midPatternHigh });
    Glyph hiWrinkleFinder2 = Glyph ({ hiPatternHigh });
    SweepPattern pitchPattern2 ({{ 3.0f, 0 }, { 1.0f / 3.0f, 0 }}, 6.0f, sampleRate);
    lowWrinkleFinder2.setPitchPattern (pitchPattern2);
    midWrinkleFinder2.setPitchPattern (pitchPattern2);
    highWrinkleFinder2.setPitchPattern (pitchPattern2);
    higher2.addStage (lowWrinkleFinder2);
    higher2.addStage (midWrinkleFinder2);
    higher2.addStage (hiWrinkleFinder2);
    highWrinkleFinder2.setPitchPattern (pitchPattern2);
    higher2.addStage (highWrinkleFinder2);
    higher2.addStage (highWrinkleFinder2.withPan (-1));
    higher2.addStage (highWrinkleFinder2.withPan (1));
    
//    higher.addStage (Glyph (MelodicNotes::withFreqs ({ 5000, 5000 }).withCyclingPans ({ -1, 1 }).withCyclingBandwidths ({ 0.5f, 0.5f, 1.0f, 1.0f, 1.5f, 1.5f })));
    
//    // Step Vertical
//    SpatialStep verticalStep;
//    verticalStep.addShape ({{ 100, 0 }, { 15000, 0 }}, sampleRate, 0.2f);
//    
//    // Step Square
//    SpatialStep squareStep;
//    squareStep.addShape ({{ 200, -1 }, { 200, 1 }, { 5000, 1 }, { 5000, -1 }}, sampleRate); // square
//    squareStep.addShape ({{ 200, -1 }, { 5000, 1 }, { 5000, -1 }, { 200, 1 }}, sampleRate); // hourglass
//    squareStep.addShape ({{ 1000, -1 }, { 200, 0 }, { 1000, 1 }, { 5000, 0 }}, sampleRate); // diamond
//    squareStep.addShape ({{ 600, -0.5 }, { 600, 0.5 }, { 2000, 0.5 }, { 2000, -0.5 }}, sampleRate); // small square
//    squareStep.addShape ({{ 2000, -0.5 }, { 2000, 0.5 }, { 5000, 0.5 }, { 5000, -0.5 }}, sampleRate); // small square
//    float bandwidth = 1.0f;
//    MelodicNotes squareNotes = MelodicNotes::withFreqs ({ 200, 200, 5000, 5000 }).withPans ({ -1, 1, -1, 1 }).withNoteDurationInSeconds (0.2f);
//    MelodicNotes center = MelodicNotes::withMelodicPattern ({ 1, 1, 1, 0 }, { 1000.0f }, bandwidth, { 0 }).withNoteDurationInSeconds (0.2f);
//    Glyph squareGlyph = Glyph ({ squareNotes, center });
//    Glyph squareShape = Glyph::shapeFromCorners ({{ 200, -1 }, { 200, 1 }, { 5000, -1 }, { 5000, 1 }});
//    squareStep.addStage (squareGlyph);
//    squareStep.addStage (squareShape);
    
    
    // Step 0 - Grids
//    SpatialStep step0;
//    Glyph twoByTwo ({ { 100, -1 }, { 700, 1 }, { 5000, 0 }, { 5000, 0 }});
//    Glyph justBass ({ { 100, -1 }});
//    Glyph justMid ({ { 700, 0 }});
//    Glyph justTreble ({ { 5000, 1 }});
//    step0.addStage (justBass);
//    step0.addStage (justMid);
//    step0.addStage (justTreble);
//    step0.addStage (twoByTwo);
    
    // Step I - Intervals
//    IntelligibilityStep step1;
    MelodicNotes wideScale = MelodicNotes ({ -12, 0, 5, 7, 9, 7, 11, 14 }, 800.0f);
//    step1.addStage (wideScale.withTransposition (-48));
//    step1.addStage (wideScale.withTransposition (-24));
//    step1.addStage (wideScale.withTransposition (0));
//    step1.addStage (wideScale.withTransposition (24));
//    step1.addStage (wideScale.withTransposition (48));
    
    IntelligibilityStep step1;
    MelodicNotes majorFifth = MelodicNotes ({ 0, 4, 7, 4 }, 500.0f);
    MelodicNotes majorThird = MelodicNotes ({ 0, 3, 7, 3 }, 500.0f);
    MelodicNotes octave = MelodicNotes ({ 0, 12 }, 500.0f);
    MelodicNotes majorSecond = MelodicNotes ({ 0, 2 }, 500.0f);
    step1.addStage (majorFifth);
    step1.addStage (majorThird);
    step1.addStage (octave);
    step1.addStage (majorSecond);
    step1.addStage (wideScale);
    
//    // Step II - Three Stack
//    SpatialStep step2;
//    MelodicNotes threeStack =
//    MelodicNotes::withFreqs ({ 200, 1000, 5000 })
//        .withBandwidth (2.0f);
//    step2.addStage (threeStack);
//    step2.addStage (threeStack.withPan (-1));
//    step2.addStage (threeStack.withPan (1));
    
    // Step II - square separation
    SpatialStep step2;
    MelodicNotes square =
    MelodicNotes::withFreqs ({ 200, 2000, 200, 2000 })
        .withPans ({ -0.5, 0.5, 0.5, -0.5 })
        .withBandwidth (1.5f);
    MelodicNotes sequence =
    MelodicNotes::withFreqs ({ 200, 4000 })
        .withBandwidth (1.0f)
        .withNoteDurationInSeconds (0.1);
    step2.addStage (square);
    step2.addStage (sequence);
    
    MelodicNotes horizontal =
    MelodicNotes::withFreqs ({ 500, 500 })
        .withBandwidth (1.0f)
        .withPans ({ -0.5, 0.5 })
        .withNoteDurationInSeconds (0.1);
    step2.addStage (horizontal);
//    step2.addStage (sequence);
    
    // Step III - Solfeggietto
    IntelligibilityStep step3;
    MelodicNotes solfeggietto =
    MelodicNotes ({ 0, -3, 0, 4, 9, 12, 11, 9, 8, 4, 8, 11, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 23, 21, 20, 18, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 20, 16, 20, 23, 28, 26, 24, 23, 24, 21, 24, 28, 33, 36, 35, 33, 35, 33, 32, 30, 28, 26, 24, 23, 24, 21, 16, 12, 9, 33, 28, 24, 29, 2, 5, 9, 14, 17, 21, 24, 23, 19, 14, 11, 7, 31, 26, 23, 28, 0, 4, 7, 12, 16, 19, 23, 21, 18, 17, 18, 21, 18, 17, 18, 24, 21, 16, 18, 24, 21, 16, 18, 23, 21, 15, 18, 30, 21, 15, 18, 27, 21, 11, 18, 21, 18, 15, 11, 19, -8, -5, -1, 4, 7, 6, 4, 3, -1, 3, 6, 11, 9, 7, 6, 7, 4, 7, 11, 16, 19, 18, 16, 18, 16, 15, 13, 11, 9, 7, 6, 7, 4, 7, 11, 16, 19, 18, 16, 15, 11, 15, 18, 23, 21, 19, 18, 19, 16, 19, 23, 28, 31, 30, 28, 30, 28, 27, 25, 23, 21, 19, 18, 19, 4, -8, 16, 19, 23, 28, 23, 19, 16, 2, -10, 28, 23, 20, 16, 20, 23, 28, 21, 12, 16, 28, 16, 21, 12, 16, 28, 16, 20, 11, 16, 26, 16, 20, 11, 16, 26, 16, 24, 9, -3, 21, 24, 28, 33, 28, 24, 21, 7, -5, 33, 28, 25, 21, 25, 28, 33, 26, 17, 21, 33, 21, 26, 17, 21, 33, 21, 25, 16, 21, 31, 21, 25, 16, 21, 31, 21, 29, -10, -7, -3, 2, 5, 4, 2, 1, -3, 1, 4, 9, 7, 5, 4, 5, 2, 5, 9, 14, 17, 16, 14, 16, 14, 13, 11, 9, 7, 5, 4, 5, 2, 5, 9, 14, 17, 16, 14, 13, 9, 13, 16, 21, 19, 17, 16, 17, 14, 17, 21, 26, 29, 28, 26, 28, 26, 25, 23, 21, 19, 17, 16, 17, 17, 26, 21, 17, 14, 14, 21, 17, 14, 9, 9, 17, 14, 9, 5, 5, 14, 9, 5, -2, -14, 29, 26, 25, 26, 28, 26, 25, 26, -3, -15, 17, 14, 13, 14, 16, 14, 13, 14, -4, -16, 35, 26, 28, 29, 28, 26, 24, 23, 24, -3, -15, 28, 33, 28, 31, 2, 29, 28, 26, 24, 4, -8, 23, 24, 23, 21, 23, 21, 12, 16, 28, 16, 21, 12, 16, 28, 16, 20, 11, 16, 26, 16, 20, 11, 16, 26, 16, 19, 9, 16, 25, 16, 19, 9, 16, 25, 16, 18, 14, 24, 33, 24, 18, 14, 24, 33, 24, 17, 7, 14, 23, 14, 17, 7, 14, 23, 14, 16, 12, 22, 31, 22, 16, 12, 22, 31, 22, 15, 5, 12, 21, 12, 15, 5, 12, 21, 12, 12, 3, 21, 33, 21, 12, 3, 21, 33, 21, 12, 4, 21, 24, 28, 33, 28, 24, 21, 28, 24, 21, 16, 26, -8, 23, 20, 14, 12, -3, 0, 4, 9, 12, 11, 9, 8, 4, 8, 11, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 23, 21, 20, 18, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 20, 16, 20, 23, 28, 26, 24, 23, 24, 21, 24, 28, 33, 36, 35, 32, 33, 28, 24, 23, 21, 16, 12, 11, 9}, 800.0f);
    step3.addStage (solfeggietto.withTranspositionInOctaves (-2));
    step3.addStage (solfeggietto.withTranspositionInOctaves (0));
    step3.addStage (solfeggietto.withTranspositionInOctaves (2));
    
    // Step IV - fancy pattern
    SpatialStep step4;
    float lowFreq = 20;
    float hiFreq = 15000;
    SweepPattern forwardSlash ({{ lowFreq, -1 }, { hiFreq, 1 }}, 2.0f, sampleRate);
    SweepPattern downwardsRight ({{ hiFreq, 1 }, { lowFreq, 1 }}, 2.0f, sampleRate);
    SweepPattern backslash ({{ lowFreq, 1 }, { hiFreq, -1 }}, 2.0f, sampleRate);
    SweepPattern downwardsLeft ({{ hiFreq, -1 }, { lowFreq, -1 }}, 2.0f, sampleRate);
    SweepPattern zigZag ({{ 20, -1 }, { 200, 1 }, { 2000, -1 }, { 200, 1 }, { 20, -1 }}, 2.0f, sampleRate);
    step4.addStage (forwardSlash);
    step4.addStage (downwardsRight);
    step4.addStage  (backslash);
    step4.addStage (downwardsLeft);
    step4.addStage (zigZag);
    
    // Initialize QualityStepManager steps imperatively
    qualityStepManager.addSpatialStep (sweeps);
    qualityStepManager.addSpatialStep (xPattern);
    qualityStepManager.addSpatialStep (higher);
    qualityStepManager.addSpatialStep (higher2);
//    qualityStepManager.addSpatialStep (step0);
    qualityStepManager.addIntelligibilityStep (step1);
    qualityStepManager.addSpatialStep (step2);
    qualityStepManager.addIntelligibilityStep (step3);
    qualityStepManager.addSpatialStep (step4);
    
    playbackManager.setQualityStep (qualityStepManager.getCurrStep());
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

void CabinEqAudioProcessor::setIsProcessing (bool isProcessing)
{
    playbackManager.setIsProcessing (isProcessing);
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

std::optional<float> CabinEqAudioProcessor::getCurrPlayingFreq()
{
    return playbackManager.getCurrPlayingFreq();
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

void CabinEqAudioProcessor::setDifficulty (float difficulty)
{
    playbackManager.setDifficulty (difficulty);
}

void CabinEqAudioProcessor::setOctaveShift (float octaveShift)
{
    playbackManager.setOctaveShift (octaveShift);
}

void CabinEqAudioProcessor::setIsPlaying (bool isPlaying)
{
    playbackManager.setIsCalibrating (isPlaying);
}

void CabinEqAudioProcessor::setIsCycling (bool isCycling)
{
    playbackManager.setIsCycling (isCycling);
}

QualityStep CabinEqAudioProcessor::getCurrStep()
{
    return qualityStepManager.getCurrStep();
}

QualityStep CabinEqAudioProcessor::goToPrevStep()
{
    qualityStepManager.goToPrevStep();
    QualityStep qualityStep = qualityStepManager.getCurrStep();
    playbackManager.setQualityStep (qualityStep);
    return qualityStep;
}

QualityStep CabinEqAudioProcessor::goToNextStep()
{
    qualityStepManager.goToNextStep();
    QualityStep qualityStep = qualityStepManager.getCurrStep();
    playbackManager.setQualityStep (qualityStep);
    return qualityStep;
}

int CabinEqAudioProcessor::getCurrStage()
{
    return playbackManager.getCurrStage();
}

void CabinEqAudioProcessor::setStage (int stageIdx)
{
    playbackManager.setStage (stageIdx);
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqAudioProcessor::profileNamed (juce::String profileName) const
{
    return cabinEqProfileManager.getProfileNamed (profileName);
}
