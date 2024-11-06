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

// This class provides a simple interface to adding and playing multiple noise patterns simultaneously
class MultiplePatternGenerator
{
public:
    MultiplePatternGenerator();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    void setSpeedFactor (float speedFactor); // does nothing, for now
    void setFreqFactor (float freqFactor); // does nothing, for now
    
    void addPattern (std::vector<bool> hits); // lets just work on hits for now, and add frequency sweeps in later. Let's make it so that you should call this before calling prepare
    void addPatterns (std::vector<std::vector<bool>> hits);
    
private:
    std::vector<NoiseGenerator> noiseGenerators;
    juce::dsp::ProcessSpec spec;
};
