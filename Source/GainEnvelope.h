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
    GainEnvelope (int startDurationInSamples, int endDurationInSamples, 
                  float targetGain = 1.0f, int startDelayInSamples = 0, int endEarlyInSamples = 0);
    const float gainAtSample (int sample, int noteDurationInSamples) const;
    
private:
    const float sigmoid (float x) const;
    
    int startDurationInSamples;
    int endDurationInSamples;
    float targetGain;
    
    int startDelayInSamples; // how much to delay the note's start
    int endEarlyInSamples; // how early to end the note by
};
