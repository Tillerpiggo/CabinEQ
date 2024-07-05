/*
  ==============================================================================

    StereoGainEnvelope.h
    Created: 5 Jul 2024 4:03:26pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GainEnvelope.h"

class StereoGainEnvelope
{
public:
    StereoGainEnvelope();
    std::pair<float, float> gainAtSample (int sample, int noteDurationInSamples);
    
private:
    GainEnvelope leftRamp;
    GainEnvelope rightRamp;
};
