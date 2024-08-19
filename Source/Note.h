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
    Note (float frequency, float gain)
        : frequency (frequency), amplitude (gain)
    {}
    
    float frequency; // hz
    float amplitude; // in dB
};
