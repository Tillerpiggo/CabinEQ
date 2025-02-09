/*
  ==============================================================================

    NoiseSource.cpp
    Created: 8 Feb 2025 2:05:18pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "NoiseSource.h"

NoiseSource::NoiseSource (float x, float y, float startTime, float endTime, float rampLength)
    : x (x), y (y), startTime (startTime), endTime (endTime), rampLength (rampLength)
{
    rampLengthInTime = (endTime - startTime) * rampLength;
}

NoisePoint NoiseSource::noisePointAtTime (float time) const
{
    return NoisePoint (x, y, volumeAtTime (time));
}

float NoiseSource::volumeAtTime (float time) const
{
    float MIN_DB = -120.0f; // supposed to be about silent
    
    // First, let's move to relative time where startTime = 0
    // Then, let's offset the time by the amount we moved it back...
    
    // End ramp, but at the start
    if (endTime > 1.0f && time < fmod (endTime, 1.0f) && time < startTime)
    {
        float normalizedEndTime = fmod (endTime, 1.0f);
        float rampPercent = std::min ((normalizedEndTime - time) / rampLengthInTime, 1.0f);
        float volDB = MIN_DB * (1.0f - rampPercent);
        float gain = juce::Decibels::decibelsToGain (volDB);

        return gain;
    }
    
    if (time < startTime || time > endTime)
        return 0.0f;
    
    if (time > startTime + rampLengthInTime && time < endTime - rampLengthInTime)
        return 1.0f;
    
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
