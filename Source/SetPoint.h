/*
  ==============================================================================

    SetPoint.h
    Created: 24 Jul 2024 10:37:34pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

/// This stores the raw data for each set point in a given EQ curve.
struct SetPoint
{
    SetPoint (float frequency, float amplitude, float pan)
        : frequency (frequency), amplitude (amplitude), pan (pan)
    {}
    
    float frequency;
    float amplitude; // in DB
    float pan; // in DB. (-3 = +1.5dB on left channel, -1.5dB on right channel)
};
