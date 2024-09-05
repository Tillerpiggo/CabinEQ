/*
  ==============================================================================

    GainEnvelope.cpp
    Created: 5 Jul 2024 4:07:55pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "GainEnvelope.h"

GainEnvelope::GainEnvelope (int startDurationInSamples, int endDurationInSamples, 
                            float targetGain, int startDelayInSamples, int endEarlyInSamples)
: startDurationInSamples (startDurationInSamples), endDurationInSamples (endDurationInSamples), targetGain (targetGain), startDelayInSamples (startDelayInSamples), endEarlyInSamples (endEarlyInSamples)
{
}

float GainEnvelope::getStartDurationInSamples() const
{
    return startDurationInSamples;
}

float GainEnvelope::getStartDelayInSamples() const
{
    return startDelayInSamples;
}

const float GainEnvelope::gainAtSample (int sample, int noteDurationInSamples) const
{
    int startRampEnd = startDelayInSamples + startDurationInSamples;
    int endRampStart = noteDurationInSamples - endDurationInSamples - endEarlyInSamples;
    
    // Make sure ramps don't overlap
    if (startRampEnd > endRampStart) {
//        throw std::runtime_error("Warning: The initial and end ramps overlap.");
    }

    // Start ramp
    if (sample >= startDelayInSamples && sample < startRampEnd) {
        float t = (sample - startDelayInSamples) / static_cast<float>(startDurationInSamples);
        return targetGain * sigmoid(12.0f * (t - 0.5f));
    }

    // Note hold at targetGain
    if (sample >= startRampEnd && sample < endRampStart) {
        return targetGain;
    }

    // End ramp
    if (sample >= endRampStart && sample < noteDurationInSamples - endEarlyInSamples) {
        float t = (sample - endRampStart) / static_cast<float>(endDurationInSamples);
        return targetGain * sigmoid(12.0f * (0.5f - t));
    }
    
    return 0.0f;
}

void GainEnvelope::setRampDurationInSamples (int duration)
{
    startDurationInSamples = duration;
    endDurationInSamples = duration;
}

void GainEnvelope::setStartDurationInSamples (int startDuration)
{
    startDurationInSamples = startDuration;
}

void GainEnvelope::setEndDurationInSamples (int endDuration)
{
    endDurationInSamples = endDuration;
}

void GainEnvelope::setTargetGain (float gain)
{
    targetGain = gain;
}

void GainEnvelope::setStartDelayInSamples (int delay)
{
    startDelayInSamples = delay;
}

void GainEnvelope::setEndEarlyInSamples (int early)
{
    std::cout << "set end early in samples to: " << early << std::endl;
    endEarlyInSamples = early;
}

const float GainEnvelope::sigmoid (float x) const {
    return 1.0f / (1.0f + std::exp(-x));
}
