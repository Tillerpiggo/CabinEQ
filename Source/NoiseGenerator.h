/*
  ==============================================================================

    NoiseGenerator.h
    Created: 5 Nov 2024 4:30:21pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
// #include "PinkNoise.h"
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
    void setVolumeDB (float volumeDB);
    void setVolumeGain (float volumeGain);
    void mute();
    
private:
    float sampleRate;
    
    // Pink noise generation
    // PinkNoise pinkNoise;
    juce::dsp::IIR::Filter<float> bandpass;
    juce::dsp::IIR::Filter<float> bandpass2;
    juce::dsp::IIR::Filter<float> bandpass3;
    juce::dsp::IIR::Filter<float> bandpass4;
    std::vector<juce::dsp::IIR::Filter<float>> lowPassFilters;
    std::vector<juce::dsp::IIR::Filter<float>> highPassFilters;
    int order = 16;
    int snapToZeroCounter = 0;
    
    // Constants
    float bandwidth = 0.5f;
    float pan = 0.0f;
    float centerFreq = 1000.0f;
    float leftGain = 0.0f;
    float rightGain = 0.0f;
    float totalGain = 1.0f;
    
//    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> leftDelayLine { 45 };
//    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> rightDelayLine { 45 };
    
    bool isMuted = false;
    bool shouldUpdateGenerators = true;
    bool shouldUpdatePan = true;

    juce::Random random;
};
