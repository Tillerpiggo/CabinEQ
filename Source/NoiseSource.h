/*
  ==============================================================================

    NoiseSource.h
    Created: 8 Feb 2025 2:05:18pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "Stroke.h"

class NoiseSource
{
public:
    NoiseSource (float x, float y, float startTime, float endTime, float rampLength);
    
    NoisePoint noisePointAtTime (float time) const;
    float volumeAtTime (float time) const; // time in [0, 1)
    
private:
    float volumeAtUnboundedTime (float time) const;
    
    float x;
    float y;
    float startTime;
    float endTime;
    float rampLength;
    float rampLengthInTime;
};
