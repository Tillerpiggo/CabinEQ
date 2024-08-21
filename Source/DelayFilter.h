/*
  ==============================================================================

    DelayFilter.h
    Created: 20 Aug 2024 10:37:31pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This class takes in audio a block at a time and applies a crossfeed effect to it
class DelayFilter
{
public:
    DelayFilter() {};
    
    std::pair<float, float> processSample (std::pair<float, float> sample); // processes a single sample and returns the next sample
    void clear(); // clears the buffers that store the delay
    void setDelay (float delayInSamples); // negative delay means delay the left channel
    
private:
    std::queue<float> buffer;
    int delayInSamples = 0;
};
