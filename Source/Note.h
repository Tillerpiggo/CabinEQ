/*
  ==============================================================================

    Note.h
    Created: 20 Jun 2024 12:31:21pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Note
{
public:
    Note (float frequency, float gain, float pan, float phase)
        : frequency (frequency), gain (gain), pan (pan), phase (phase) {}
    
    float frequency; // hz
    float gain; // in dB
    float pan; // in dB; - is left, + is right (ex. -4 would mean +2 dB on the left, -2 dB on the right)
    float phase; // in radian offset of right channel
};
