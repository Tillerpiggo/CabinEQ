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
    Note (double gain, double frequency, double pan)
        : gain (gain), frequency (frequency), pan (pan) {}
    
    double gain; // in dB; 0 dB = silent
    double frequency; // hz
    double pan; // from -1 (hard left) to 1 (hard right)
};
