/*
  ==============================================================================

    CrossfeedProcessor.cpp
    Created: 29 Oct 2024 10:00:00am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CrossfeedProcessor.h"

void CrossfeedProcessor::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    delayBuffer.setSize (2, (int) std::ceil (sampleRate * maxDelayMs / 1000.0) + 2);
    lowpassCoefficient = (float) std::exp (-juce::MathConstants<double>::twoPi * lowpassHz / sampleRate);
    gain.reset (sampleRate, 0.05);
    delaySamples.reset (sampleRate, 0.05);
    reset();
}

void CrossfeedProcessor::reset()
{
    delayBuffer.clear();
    writePosition = 0;
    lowpassState = { 0.0f, 0.0f };
    gain.setCurrentAndTargetValue (isEnabled ? juce::Decibels::decibelsToGain (levelDb.load()) : 0.0f);
    delaySamples.setCurrentAndTargetValue (delayInSamples());
}

float CrossfeedProcessor::delayInSamples() const
{
    return juce::jlimit (0.0f, (float) delayBuffer.getNumSamples() - 2.0f, (float) (delayMs.load() * sampleRate / 1000.0));
}

void CrossfeedProcessor::setEnabled (bool enabled)
{
    isEnabled = enabled;
}

void CrossfeedProcessor::setLevelDb (float newLevelDb)
{
    levelDb = newLevelDb;
}

void CrossfeedProcessor::setDelayMs (float newDelayMs)
{
    delayMs = juce::jlimit (0.0f, maxDelayMs, newDelayMs);
}

void CrossfeedProcessor::process (juce::dsp::AudioBlock<float>& block) noexcept
{
    const int bufferSize = delayBuffer.getNumSamples();
    if (block.getNumChannels() < 2 || bufferSize == 0)
        return;

    gain.setTargetValue (isEnabled ? juce::Decibels::decibelsToGain (levelDb.load()) : 0.0f);
    if (! gain.isSmoothing() && gain.getTargetValue() == 0.0f)
        return;

    delaySamples.setTargetValue (delayInSamples());
    auto* left = block.getChannelPointer (0);
    auto* right = block.getChannelPointer (1);
    auto* leftDelay = delayBuffer.getWritePointer (0);
    auto* rightDelay = delayBuffer.getWritePointer (1);
    const float a = lowpassCoefficient;

    for (size_t i = 0; i < block.getNumSamples(); ++i)
    {
        // Low-pass what crosses over, since the head shadows the high frequencies from the far speaker
        lowpassState[0] = left[i] + a * (lowpassState[0] - left[i]);
        lowpassState[1] = right[i] + a * (lowpassState[1] - right[i]);
        leftDelay[writePosition] = lowpassState[0];
        rightDelay[writePosition] = lowpassState[1];

        // Read between samples, since the delay can be fractional
        const float readPosition = (float) writePosition - delaySamples.getNextValue() + (float) bufferSize;
        const int before = (int) readPosition;
        const float fraction = readPosition - (float) before;
        const int i0 = before % bufferSize, i1 = (before + 1) % bufferSize;

        const float g = gain.getNextValue();
        const float fromRight = (rightDelay[i0] + fraction * (rightDelay[i1] - rightDelay[i0])) * g;
        const float fromLeft = (leftDelay[i0] + fraction * (leftDelay[i1] - leftDelay[i0])) * g;
        left[i] += fromRight;
        right[i] += fromLeft;

        writePosition = (writePosition + 1) % bufferSize;
    }
}
