/*
  ==============================================================================

    SpatialPinkNoiseGenerator.h
    Created: 15 Oct 2024 11:33:41am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PinkNoise.h"
#include "BandProfile.h"

class SpatialPinkNoiseGenerator
{
public:
    SpatialPinkNoiseGenerator();
    
    std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void setBandpass (float centreFreq, float bandwidth);
    
private:
    void fillBuffers();
    
    float sampleRate;
    float centreFreq = 1000.0f;
    float bandwidth = 100.0f;
    
    // Bandpass filter
    using BandpassFilter = juce::dsp::IIR::Filter<float>;
    
    // Pink noise generation
    PinkNoise pinkNoise;
    juce::Random noiseSrc;
    BandpassFilter bandpass;
    
    // Buffers
    int bufferIdx = 0;
    int bufferSize = 2048;
    juce::AudioBuffer<float> leftBuffer;
    juce::AudioBuffer<float> rightBuffer;
};
