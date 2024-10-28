/*
  ==============================================================================

    MelodicNoiseSequencer.h
    Created: 27 Oct 2024 1:05:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PinkNoise.h"
#include "SpatialPatternGenerator.h" // for NoiseNote
#include "SineWaveGenerator.h"

class MelodicNoiseSequencer
{
public:
    MelodicNoiseSequencer();
    
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setPattern (std::vector<NoiseNote> notes);
    std::pair<float, float> getNextSample();
    
private:
    NoiseNote getCurrNote();
    void updateNotchFilter();
    void goToNextNote();
    
    std::vector<NoiseNote> notes;
    
    PinkNoise pinkNoise;
    
    int numSamplesNoteHasBeenPlaying;
    int currNoteIdx;
    float sampleRate;
    
    juce::dsp::IIR::Filter<float> notchFilter; // to add a notch in the main noise
    SineWaveGenerator sineWaveGenerator;
    int snapToZeroCounter = 0;
};
