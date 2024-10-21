/*
  ==============================================================================

    SineSweepGenerator.h
    Created: 28 Jul 2024 7:34:18pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SineWaveGenerator.h"
#include "Constants.h"
#include "Curve.h"

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

/// This class provides an easy interface to generate a sine sweep in a given frequency range.
class SineSweepGenerator
{
public:
    SineSweepGenerator();
    
    std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void setSweepPattern (SweepPattern sweepPattern); // must be called before getNextSample is called
    float getCurrFreq() const;
    
private:
    SineWaveGenerator sineWaveGenerator;
    std::optional<SweepPattern> sweepPattern;
};
