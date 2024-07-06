/*
  ==============================================================================

    StereoGainEnvelope.cpp
    Created: 5 Jul 2024 4:03:26pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "StereoGainEnvelope.h"

StereoGainEnvelope::StereoGainEnvelope (int rampDurationInSamples)
{
    leftRamp.setRampDurationInSamples (rampDurationInSamples);
    rightRamp.setRampDurationInSamples (rampDurationInSamples);
}

StereoGainEnvelope::StereoGainEnvelope (int rampDurationInSamples,
                    int rightDelayInSamples) : StereoGainEnvelope (rampDurationInSamples)
{
    rightRamp.setStartDelayInSamples(rightDelayInSamples);
}

StereoGainEnvelope::StereoGainEnvelope (StereoGainEnvelopeType type, int rampDurationInSamples)
: StereoGainEnvelope (rampDurationInSamples)
{
    switch (type)
    {
        case StereoGainEnvelopeType::HARD_LEFT:
            rightRamp.setTargetGain (0.0f);
            break;
        case StereoGainEnvelopeType::HARD_RIGHT:
            leftRamp.setTargetGain (0.0f);
            break;
        case StereoGainEnvelopeType::SILENT:
            leftRamp.setTargetGain (0.0f);
            rightRamp.setTargetGain (0.0f);
            break;
    }
}

const std::pair<float, float> StereoGainEnvelope::getGainAtSample (int sample, int noteDurationInSamples) const
{
    float leftSample = leftRamp.gainAtSample (sample, noteDurationInSamples);
    float rightSample = rightRamp.gainAtSample (sample, noteDurationInSamples);
    return { leftSample, rightSample };
}



