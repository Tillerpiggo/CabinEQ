/*
  ==============================================================================

    PatternEnvelope.cpp
    Created: 9 Feb 2025 5:57:13pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PatternEnvelope.h"

float PatternEnvelope::volumeAtTime (float time, int patternIdx) const
{
    auto hits = patterns[patternIdx % patterns.size()];
    
    if (time < 0 || time >= 1)
        return 0.0f;
    
    float MIN_DB = -120.0f; // supposed to be about silent
    
    // Make sure this is a hit
    float idx = std::floor (time * (float) tempo);
    if (! hits[idx])
        return 0.0f;
    
    // Normalize time to tempo
    time = fmod (time, 1.0f / (float) tempo);
    
    // Figure out start and end time based on tempo
    float startTime = 0.0f;
    float endTime = 1.0f / (float) tempo;
    float rampLengthInTime = 0.4 * endTime;
        
    if ((time > startTime + rampLengthInTime && time < endTime - rampLengthInTime))
    {
        return 1.0f;
    }
    
    // Start ramp
    if (time < startTime + rampLengthInTime)
    {
        float rampPercent = (time - startTime) / rampLengthInTime;
        float volDB = MIN_DB * (1.0f - rampPercent);
        float gain = juce::Decibels::decibelsToGain (volDB);
        return gain;
    }
    
    // End ramp
    else
    {
        float rampPercent = (endTime - time) / rampLengthInTime;
        float volDB = MIN_DB * (1.0f - rampPercent);
        float gain = juce::Decibels::decibelsToGain (volDB);
        return gain;
    }
}
