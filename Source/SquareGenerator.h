/*
  ==============================================================================

    SquareGenerator.h
    Created: 10 Feb 2025 3:15:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PinkNoise.h"

class SquareGenerator
{
public:
    SquareGenerator();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setResolution (int resolution);
    void setFreqIdx (int freqIdx);
    void setVolumeGain (float volumeGain);
    
private:
    void updateGeneratorsIfNeeded(); // recalculates generators to match state if shouldUpdateGenerators is true
    
    float sampleRate;
    std::vector<std::pair<float, float>> leftRightGains; // stores the left and right gains for all noise gens
    float totalGain = 1.0f;
    
    // Pink noise generation
    std::vector<PinkNoise> pinkNoises;
    std::vector<juce::dsp::IIR::Filter<float>> lowPassFilters;
    std::vector<juce::dsp::IIR::Filter<float>> highPassFilters;
    int order = 16;
    int snapToZeroCounter = 0;
    
    int resolution = 2;
    int freqIdx; // the frequency idx from [0, resolution). Panning width assumed to match resolution width. 0 is lowest and resolution is highest.
    int panIdx; // the pan idx from [0, resolution]. 0 is farthest to left, 1 is farthest to right.
    int density = 4; // noise sources per unit resolution
    
    bool shouldUpdateGenerators = true; // updates generators and position to match resolution, freqIdx, and volumeGain
    
    float MIN_FREQ = 20.0f;
    float MAX_FREQ = 20000.0f;
};
