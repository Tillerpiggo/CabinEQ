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
    Note (double gain, double frequency, double pan, double phase)
    : gain (gain), frequency (frequency), pan (pan), phase (phase) {}
    
    double gain; // in dB; 0 dB = silent
    double frequency; // hz
    double pan; // from -1 (hard left) to 1 (hard right)
    double phase; // from 0 to 360, phase shift of left side
};
