/*
  ==============================================================================

    TimeGainEnvelope.h
    Created: 17 Dec 2024 12:30:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class TimeGainEnvelope
{
public:
    TimeGainEnvelope (float startDurationInSeconds = 0.05, float endDurationInSeconds = 0.05);
    const float gainAtTime (float timeInSeconds, float noteDurationInSeconds) const; // gives gain at time, assuming the gain envelope is looping (so if seconds > noteDurationInSeconds, it will use seconds % noteDurationInSeconds)
    
    float getStartDurationInSeconds() const;
    float getEndDurationInSeconds() const;
    
    void setStartDurationInSeconds (float startDuration);
    void setEndDurationInSeconds (float endDuration);
private:
    float startDurationInSeconds;
    float endDurationInSeconds;
};
