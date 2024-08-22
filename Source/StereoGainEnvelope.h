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
    SOFT_LEFT,
    SOFT_RIGHT,
    SILENT
};

class StereoGainEnvelope
{
public:
    StereoGainEnvelope (int rampDurationInSamples = 5000);
    StereoGainEnvelope (int startRampDurationInSamples, int endRampDurationInSamples, int endEarlyInSamples);
    StereoGainEnvelope (int rampDurationInSamples, int rightDelayInSamples);
    StereoGainEnvelope (float pan, int rampDurationInSamples = 500); // pan goes from -1 (hard left) to 1 (hard right)
    StereoGainEnvelope (StereoGainEnvelopeType type, int rampDurationInSamples = 5000);
    
    const std::pair<float, float> getGainAtSample (int sample, int noteDurationInSamples) const;
    
    static StereoGainEnvelope hardLeft()
    {
        return StereoGainEnvelope (StereoGainEnvelopeType::HARD_LEFT);
    }
    
    static StereoGainEnvelope hardRight()
    {
        return StereoGainEnvelope (StereoGainEnvelopeType::HARD_RIGHT);
    }
    
    static StereoGainEnvelope silent()
    {
        return StereoGainEnvelope (StereoGainEnvelopeType::SILENT);
    }
    
    StereoGainEnvelope withPan (float pan) const; // DOESN'T WORK WITH STEREO GAIN ENVELOPES
    
private:
    GainEnvelope leftRamp;
    GainEnvelope rightRamp;
};
