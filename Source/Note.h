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
    Note (float gain, float frequency, float pan)
        : gain (gain), frequency (frequency), pan (pan) {}
    
    float gain; // in dB; 0 dB = silent
    float frequency; // hz
    float pan; // in dB; - is left, + is right (ex. -4 would mean +2 dB on the left, -2 dB on the right)
};
