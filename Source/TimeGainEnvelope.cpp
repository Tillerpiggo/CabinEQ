/*
  ==============================================================================

    TimeGainEnvelope.cpp
    Created: 17 Dec 2024 12:30:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "TimeGainEnvelope.h"

TimeGainEnvelope::TimeGainEnvelope (float startDurationInSeconds, float endDurationInSeconds)
    : startDurationInSeconds (startDurationInSeconds), endDurationInSeconds (endDurationInSeconds)
{
    
}

const float TimeGainEnvelope::gainAtTime (float timeInSeconds, float noteDurationInSeconds) const
{
    timeInSeconds = fmod (timeInSeconds, noteDurationInSeconds); // loop if timeInSeconds > noteDurationInSeconds
    
    float startRampEnd = startDurationInSeconds;
    float endRampStart = noteDurationInSeconds - endDurationInSeconds;
    
    // Alert if start and end ramp overlap
    if (startRampEnd > endRampStart)
    {
//        std::cerr << "Start ramp overlaps with end ramp in TimeGainEnvelope" << std::endl; // in case this causes any problems, it's helpful to know
        endRampStart = endDurationInSeconds;
    }
    
    float targetDb = 0.0f;
    float minDb = -120.0f;
    
    // Start ramp
    if (timeInSeconds < startRampEnd)
    {
        float t = timeInSeconds / startDurationInSeconds;
        float startRampDb = minDb + t * (targetDb - minDb);
        return juce::Decibels::decibelsToGain (startRampDb);
    }
    
    // Note hold in between
    if (timeInSeconds >= startRampEnd && timeInSeconds < endRampStart)
    {
        return juce::Decibels::decibelsToGain (targetDb);
    }
    
    // End ramp
    if (timeInSeconds >= endRampStart)
    {
        float t = (timeInSeconds - endRampStart) / endDurationInSeconds;
        float endRampDb = targetDb + t * (minDb - targetDb);
        return juce::Decibels::decibelsToGain (endRampDb);
    }
    return 0.0f; // default to silent
}

float TimeGainEnvelope::getStartDurationInSeconds() const
{
    return startDurationInSeconds;
}

float TimeGainEnvelope::getEndDurationInSeconds() const
{
    return endDurationInSeconds;
}

void TimeGainEnvelope::setStartDurationInSeconds (float startDuration)
{
    this->startDurationInSeconds = startDuration;
}

void TimeGainEnvelope::setEndDurationInSeconds (float endDuration)
{
    this->endDurationInSeconds = endDuration;
}
