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

enum class StereoGainEnvelopeType
{
    HARD_LEFT,
    HARD_RIGHT,
    SILENT
};

class StereoGainEnvelope
{
public:
    StereoGainEnvelope (int rampDurationInSamples = 5000);
    StereoGainEnvelope (int rampDurationInSamples, int rightDelayInSamples);
    StereoGainEnvelope (StereoGainEnvelopeType type, int rampDurationInSamples = 5000);
    
    const std::pair<float, float> getGainAtSample (int sample, int noteDurationInSamples) const;
    
private:
    GainEnvelope leftRamp;
    GainEnvelope rightRamp;
};
