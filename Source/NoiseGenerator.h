/*
  ==============================================================================

    NoiseGenerator.h
    Created: 5 Nov 2024 4:30:21pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CutoffFilter.h"
#include "PinkNoise.h"
#include "Pattern.h"

// This class generates pink noise with a certain center frequency and bandwidth ( or starting frequency and ending frequency )
class NoiseGenerator
{
public:
    NoiseGenerator();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setCenterFrequencyAndBandwidth (float centerFreq, float bandwidthInOctaves);
    void setStartAndEndFrequency (float startFreq, float endFreq);
    void setFrequencyRange (std::pair<float, float> freqRange);
    void setPattern (Pattern pattern);
    
private:
    void fillBuffer();
    void updateFilters();
    
    // Noise generation
    CutoffFilter lowCutFilter, highCutFilter;
    PinkNoise pinkNoise;
    juce::dsp::ProcessSpec spec;
    bool shouldUpdateFilters = false;
    
    // Frequency and panning
    float startFreq, endFreq;
    float leftGain = 1.0f, rightGain = 1.0f;
    
    // Buffered generation
    int bufferIdx = 0;
    int bufferSize; // above so it's initialized before buffer
    juce::AudioBuffer<float> buffer;
    
    // Pattern
    std::optional<Pattern> pattern;
    float percentIncrementPerSample = 0.1; // adjust this to taste, I guess
};
