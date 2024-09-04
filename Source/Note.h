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
    Note (float frequency, float ampl, float pan, float phase)
        : frequency (frequency), amplitude (ampl), pan (pan), phase (phase)
    {}
    
    float frequency; // hz
    float amplitude; // in dB
    float pan; // dB difference between channels; -3 means +1.5db on the left, -1.5db on the right
    float phase;
};
