/*
  ==============================================================================

    NoiseGenerator.h
    Created: 5 Nov 2024 4:30:21pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PinkNoise.h"
#include "BandProfile.h"

// This class generates pink noise with a certain center frequency and bandwidth ( or starting frequency and ending frequency )
class NoiseGenerator
{
public:
    NoiseGenerator();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setBandwidth (float bandwidth);
    void setBandpass (float centerFreq);
    void setPan (float pan);
    
private:
    float sampleRate;
    
    // Pink noise generation
    PinkNoise pinkNoise;
    juce::dsp::IIR::Filter<float> bandpass;
    int snapToZeroCounter = 0;
    
    // Constants
    float bandwidth = 0.5f;
    float pan = 0.0f;
    float leftGain = 0.0f;
    float rightGain = 0.0f;
};
