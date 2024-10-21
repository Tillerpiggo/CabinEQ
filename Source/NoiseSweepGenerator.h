/*
  ==============================================================================

    NoiseSweepGenerator.h
    Created: 20 Oct 2024 11:13:55pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "SweepPattern.h"
#include "PinkNoise.h"
#include "BandProfile.h"

/// Generates noise sweeps of various bandwidths and overall envelope
class NoiseSweepGenerator
{
public:
    NoiseSweepGenerator();
    
    std::pair<float, float> getNextSample();
    void setSampleRate (float sampleRate);
    void setBandwidth (float bandwidth);
    void setSweepPattern (SweepPattern sweepPattern); // must be called before getNextSample is called for audio output
    
private:
    void setBandpass (float centreFreq);
    
    std::optional<SweepPattern> sweepPattern;
    float sampleRate;
    
    // Pink noise generation
    PinkNoise pinkNoise;
    
    // Bandpass filter
    using BandpassFilter = juce::dsp::IIR::Filter<float>;
    BandpassFilter bandpass;
    float bandwidth = 2.0f;
    int snapToZeroCounter = 0;
};
