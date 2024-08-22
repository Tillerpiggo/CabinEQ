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

StereoGainEnvelope::StereoGainEnvelope (int startRampDurationInSamples, int endRampDurationInSamples,
                                        int endEarlyInSamples)
{
    leftRamp.setStartDurationInSamples (startRampDurationInSamples);
    rightRamp.setStartDurationInSamples (startRampDurationInSamples);
    leftRamp.setEndDurationInSamples (endRampDurationInSamples);
    rightRamp.setEndDurationInSamples (endRampDurationInSamples);
    leftRamp.setEndEarlyInSamples (endEarlyInSamples);
    rightRamp.setEndEarlyInSamples (endEarlyInSamples);
}

StereoGainEnvelope::StereoGainEnvelope (int rampDurationInSamples, int rightDelayInSamples)
    : StereoGainEnvelope (rampDurationInSamples)
{
    rightRamp.setStartDelayInSamples(rightDelayInSamples);
}

StereoGainEnvelope::StereoGainEnvelope (float pan, int rampDurationInSamples)
{
    leftRamp.setRampDurationInSamples (rampDurationInSamples);
    rightRamp.setRampDurationInSamples (rampDurationInSamples);
    
    // Assume normal target gain of 1.0 in left and right
    float angle = pan * M_PI / 4.0f; // go from [-1, 1] to [-pi/4, pi/4]
    float leftGain = std::sqrt (2.0f) / 2.0f * (std::cos (angle) - std::sin(angle));
    float rightGain = std::sqrt (2.0f) / 2.0f * (std::cos(angle) + std::sin(angle));
    leftRamp.setTargetGain (leftGain);
    rightRamp.setTargetGain (rightGain);
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
        case StereoGainEnvelopeType::SOFT_LEFT:
            rightRamp.setTargetGain (0.5f);
            break;
        case StereoGainEnvelopeType::SOFT_RIGHT:
            leftRamp.setTargetGain (0.5f);
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

StereoGainEnvelope StereoGainEnvelope::withPan (float pan) const
{
    return StereoGainEnvelope (pan, leftRamp.getStartDurationInSamples());
}
