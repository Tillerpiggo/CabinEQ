/*
  ==============================================================================

    SweepPattern.h
    Created: 20 Oct 2024 11:17:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SweepPattern
{
public:
    SweepPattern (float centerFreq, float bandwidth, float durationInSeconds, float sampleRate);
    
    float getNextFreq(); // returns the next frequency. Should be called each sample.
    float getCurrFreq() const; // returns the current frequency, without changing anything
    
private:
    float centerFreq;
    float bandwidth;
    float sampleRate;
    float durationInSeconds;
    
    int idx; // the time we are at in the cycle
    int cycleLen; // # samples the cycle is
    float currFreq;
};
