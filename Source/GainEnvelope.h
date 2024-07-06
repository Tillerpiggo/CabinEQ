/*
  ==============================================================================

    GainEnvelope.h
    Created: 5 Jul 2024 4:07:55pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class GainEnvelope
{
public:
    GainEnvelope (int startDurationInSamples = 500, int endDurationInSamples = 500,
                  float targetGain = 1.0f, int startDelayInSamples = 0, int endEarlyInSamples = 0);
    const float gainAtSample (int sample, int noteDurationInSamples) const;
    
    void setRampDurationInSamples (float duration);
    void setTargetGain (float gain);
    void setStartDelayInSamples (float delay);
    void setEndEarlyInSamples (float early);
    
private:
    const float sigmoid (float x) const;
    
    int startDurationInSamples;
    int endDurationInSamples;
    float targetGain;
    
    int startDelayInSamples; // how much to delay the note's start
    int endEarlyInSamples; // how early to end the note by
};
