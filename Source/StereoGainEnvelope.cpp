/*
  ==============================================================================

    StereoGainEnvelope.cpp
    Created: 5 Jul 2024 4:03:26pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "StereoGainEnvelope.h"

StereoGainEnvelope::StereoGainEnvelope() : leftRamp (500, 500), rightRamp (500, 500)
{
    
}

std::pair<float, float> StereoGainEnvelope::gainAtSample (int sample, int noteDurationInSamples)
{
    float leftSample = leftRamp.gainAtSample (sample, noteDurationInSamples);
    float rightSample = rightRamp.gainAtSample (sample, noteDurationInSamples);
    return { leftSample, rightSample };
}



