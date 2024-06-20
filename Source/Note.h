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
    Note (float gain, float frequency, float pan, float phase)
    : gain (gain), frequency (frequency), pan (pan), phase (phase) {}
    
    float gain; // in dB; 0 dB = silent
    float frequency; // hz
    float pan; // from -1 (hard left) to 1 (hard right)
    float phase; // from 0 to 360, phase shift of left side
};
