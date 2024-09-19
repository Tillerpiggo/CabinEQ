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
    StereoGainEnvelope (int rampDurationInSamples = 500);
    StereoGainEnvelope (int startRampDurationInSamples, int endRampDurationInSamples, int endEarlyInSamples);
    StereoGainEnvelope (int rampDurationInSamples, int rightDelayInSamples);
    StereoGainEnvelope (float pan, int leftRampDurationInSamples, int rightRampDurationInSamples, int leftDelayInSamples, int rightDelayInSamples);
    StereoGainEnvelope (StereoGainEnvelopeType type, int rampDurationInSamples = 500);
    StereoGainEnvelope (GainEnvelope leftEnvelope, GainEnvelope rightEnvelope, int leftDelayInSamples, int rightDelayInSamples); // creates envelope with the two envelopes, but adds the left delay and right delay on top of whatever was already there.
    
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
    
    static StereoGainEnvelope clap()
    {
        return StereoGainEnvelope (500, 500, 0);
    }
    
    StereoGainEnvelope withPan (float pan) const;
    StereoGainEnvelope withPhase (float phase, float freq) const;
    
private:
    GainEnvelope leftRamp;
    GainEnvelope rightRamp;
};
