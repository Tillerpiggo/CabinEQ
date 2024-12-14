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

//const float GainEnvelope::gainAtSample (int sample, int noteDurationInSamples) const
//{
//    int startRampEnd = startDelayInSamples + startDurationInSamples;
//    int endRampStart = noteDurationInSamples - endDurationInSamples - endEarlyInSamples;
//    
//    // Make sure ramps don't overlap
//    if (startRampEnd > endRampStart) {
////        throw std::runtime_error("Warning: The initial and end ramps overlap.");
//    }
//
//    // Start ramp
//    if (sample >= startDelayInSamples && sample < startRampEnd) {
//        float t = (sample - startDelayInSamples) / static_cast<float>(startDurationInSamples);
//        return targetGain * sigmoid(12.0f * (t - 0.5f));
//    }
//
//    // Note hold at targetGain
//    if (sample >= startRampEnd && sample < endRampStart) {
//        return targetGain;
//    }
//
//    // End ramp
//    if (sample >= endRampStart && sample < noteDurationInSamples - endEarlyInSamples) {
//        float t = (sample - endRampStart) / static_cast<float>(endDurationInSamples);
//        return targetGain * sigmoid(12.0f * (0.5f - t));
//    }
//    
//    return 0.0f;
//}

const float GainEnvelope::gainAtSample(int sample, int noteDurationInSamples) const
{
    int startRampEnd = startDelayInSamples + startDurationInSamples;
    int endRampStart = noteDurationInSamples - endDurationInSamples - endEarlyInSamples;

    // Make sure ramps don't overlap
    if (startRampEnd > endRampStart) {
        // Optionally throw an error or handle the overlap case
        // throw std::runtime_error("Warning: The initial and end ramps overlap.");
    }

    // Convert targetGain (linear) to targetGainDb (decibels)
    float targetGainDb = juce::Decibels::gainToDecibels(targetGain, -100.0f); // Use -100 dB as minimum value

    // Start ramp
    if (sample >= startDelayInSamples && sample < startRampEnd) {
        float t = (sample - startDelayInSamples) / static_cast<float>(startDurationInSamples);
        float startRampDb = t * targetGainDb; // Linearly interpolate in dB
        return juce::Decibels::decibelsToGain(startRampDb); // Convert dB to linear gain
    }

    // Note hold at targetGain
    if (sample >= startRampEnd && sample < endRampStart) {
        return targetGain; // Already linear; no conversion needed
    }

    // End ramp
    if (sample >= endRampStart && sample < noteDurationInSamples - endEarlyInSamples) {
        float t = (sample - endRampStart) / static_cast<float>(endDurationInSamples);
        float endRampDb = (1.0f - t) * targetGainDb; // Linearly interpolate in dB
        return juce::Decibels::decibelsToGain(endRampDb); // Convert dB to linear gain
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
    endEarlyInSamples = early;
}

const float GainEnvelope::sigmoid (float x) const {
    return 1.0f / (1.0f + std::exp(-x));
}
