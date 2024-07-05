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
    const std::pair<float, float> getGainAtSample (int sample, int noteDurationInSamples) const;
    
private:
    GainEnvelope leftRamp;
    GainEnvelope rightRamp;
};
