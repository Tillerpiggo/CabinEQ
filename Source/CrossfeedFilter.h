/*
  ==============================================================================

    CrossfeedFilter.h
    Created: 17 Aug 2024 8:20:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Channel.h"

// This class takes in audio a block at a time and applies a crossfeed effect to it
class CrossfeedFilter
{
public:
    CrossfeedFilter();
    
    std::pair<float, float> processSample (std::pair<float, float> sample); // processes a single sample and returns the next sample
    void processBlock (juce::AudioBuffer<float>& buffer);
    void setSampleRate (const float newSampleRate);
    void clear(); // clears the buffers that store the delay
    
    void setCrossfeedGain (float crossfeedGain);
    void setDelay (float delayInMs);
    void setChannelPlaying (Channel channel); // sets the channel that sound comes through; mutes the other channel
    
private:
    void updateNumSamplesToDelay();
    
    // Pushes everything from the buffer into the queue, and returns what is popped
    std::vector<float> pushAndPop (std::queue<float>& queue, const float* buf, const int numSamples);
    std::queue<float> leftBuffer; // stores audio from left channel and is added to right channel
    std::queue<float> rightBuffer; // stores audio from right channel and is added to left channel
    
    int delayInMs = 7;
    int numSamplesToDelay = 0;
    float crossfeedGain = 0.5;
    float sampleRate = 44100;
    Channel currChannel;
};
