/*
  ==============================================================================

    MultiplePatternGenerator.h
    Created: 6 Nov 2024 1:08:19am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "NoiseGenerator.h"
#include "FauxMusicPattern.h"

// This class provides a simple interface to adding and playing multiple noise patterns simultaneously
class MultiplePatternGenerator
{
public:
    MultiplePatternGenerator();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    void setSpeedFactor (float speedFactor); // does nothing, for now
    void setFreqFactor (float freqFactor); // does nothing, for now
    void setPattern (FauxMusicPattern pattern); // also must be called after prepare. Assumes len(hits) == len(freqRanges)
    void mute();
    
private:
    std::vector<std::unique_ptr<NoiseGenerator>> noiseGenerators;
    juce::dsp::ProcessSpec spec;
    bool isMuted = false;
};
