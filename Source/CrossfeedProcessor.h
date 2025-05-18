/*
  ==============================================================================

    CrossfeedProcessor.h
    Created: 29 Oct 2024 10:00:00am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class CrossfeedProcessor
{
public:
    CrossfeedProcessor();

    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::dsp::AudioBlock<float>& block);
    
    void setDelaySamples (int samples);
    void setCrossfeedVolume (float volume);
    void setEnabled (bool enabled);

private:
    juce::AudioBuffer<float> leftDelayBuffer;
    juce::AudioBuffer<float> rightDelayBuffer;
    
    int delaySamples = 0;
    float crossfeedVolume = 0.0f;
    
    int writePosition = 0;
    double sampleRate = 44100.0;
    int maxBufferSamples = 0; // Maximum samples the buffer can hold, determined by max delay
    bool isEnabled = false;
}; 