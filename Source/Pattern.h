/*
  ==============================================================================

    Pattern.h
    Created: 5 Nov 2024 5:56:07pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SweepPattern.h"
#include "GainEnvelope.h"

// This represents a pattern of noise that has a certain rhythm, panning, and frequency. It's an alternative to SweepPattern that allows syncing between panning/frequency changes and the rhythm.
class Pattern
{
public:
    // Stores leftGain, rightGain, and freqFactor at a given point
    struct Feature
    {
        Feature (float leftGain, float rightGain, float freqFactor)
            : leftGain (leftGain), rightGain (rightGain), freqFactor (freqFactor)
        {}
        
        float leftGain;
        float rightGain;
        float freqFactor;
    };
    
    Pattern (std::vector<bool> hits, float cycleLengthInSeconds, float sampleRate); // for now, set freqPattern and pan pattern to static patterns of 1.0f/0.0f (so no change)
    
    Feature getNextFeature(); // the feature for the next sample
    Feature getCurrFeature();
    
private:
    std::vector<bool> hits; // true for a hit and false for a rest
    std::optional<SweepPattern> freqPattern;
    std::optional<SweepPattern> panPattern;
    
    float cycleLengthInSeconds;
    float sampleRate;
    
    GainEnvelope gainEnvelope;
    int hitDurationInSamples;
    int sampleIdx = 0;
};
