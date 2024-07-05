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

const std::pair<float, float> StereoGainEnvelope::getGainAtSample (int sample, int noteDurationInSamples) const
{
    float leftSample = leftRamp.gainAtSample (sample, noteDurationInSamples);
    float rightSample = rightRamp.gainAtSample (sample, noteDurationInSamples);
    return { leftSample, rightSample };
}



