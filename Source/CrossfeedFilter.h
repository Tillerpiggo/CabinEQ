/*
  ==============================================================================

    CrossfeedFilter.h
    Created: 17 Aug 2024 8:20:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This class takes in audio a block at a time and applies a crossfeed effect to it
class CrossfeedFilter
{
public:
    CrossfeedFilter();
    
    void processBlock (juce::AudioBuffer<float>& buffer);
    void prepare (const juce::dsp::ProcessSpec& spec);
    
private:
    // Pushes everything from the buffer into the queue, and returns what is popped
    std::vector<float> pushAndPop (std::queue<float>& queue, juce::AudioBuffer<float>& buffer);
    
    std::queue<float> leftBuffer; // stores audio from left channel and is added to right channel
    std::queue<float> rightBuffer; // stores audio from right channel and is added to left channel
    
    int delayInSeconds = 0.2;
    int numSamplesToDelay = 0.2 * 44100;
    float crossfeedGain = 0.3;
};
