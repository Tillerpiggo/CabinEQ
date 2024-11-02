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
#include "SequencerListener.h"

/// Generates noise sweeps of various bandwidths and overall envelope
class NoiseSweepGenerator
{
public:
    NoiseSweepGenerator();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setBandwidth (float bandwidth);
    void setPan (float pan);
    void setSweepPattern (SweepPattern sweepPattern); // must be called before getNextSample is called for audio output
    
    void setPeakFilter (float centerFreq, float bandwidth, float ampl);
    void setListener (SequencerListener* listener);
    
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
    float pan = 0.0f;
    float leftAmplitudeCompensation = 1.0f; // in gain
    float rightAmplitudeCompensation = 1.0f; // in gain
    int snapToZeroCounter = 0;
    
    // Peak filter
    juce::dsp::IIR::Filter<float> peakFilter;
    
    SequencerListener* listener = listener;
};
