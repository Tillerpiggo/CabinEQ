/*
  ==============================================================================

    PlaybackManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PlaybackManager.h"

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    maxChunkSize = (int) std::max (spec.maximumBlockSize, (juce::uint32) 512);
    dryBuffer.setSize ((int) std::max (spec.numChannels, (juce::uint32) 2), maxChunkSize);
    gainRamp.assign ((size_t) maxChunkSize, 1.0f);
    mixRamp.assign ((size_t) maxChunkSize, 1.0f);

    filter.prepare (spec.sampleRate);
    crossfeed.prepare (spec);
    calibration.prepare (spec.sampleRate);

    gain.reset (spec.sampleRate, 0.05);
    gain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (gainDb.load()));
    wetMix.reset (spec.sampleRate, 0.03);
    wetMix.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);
    isFullyBypassed = bypassed;
}

void PlaybackManager::setBands (const std::vector<Band>& bands)
{
    filter.setBands (bands);
}

void PlaybackManager::setGainDb (float newGainDb)
{
    gainDb = newGainDb;
}

void PlaybackManager::setBypassed (bool shouldBeBypassed)
{
    bypassed = shouldBeBypassed;
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& buffer) noexcept
{
    gain.setTargetValue (juce::Decibels::decibelsToGain (gainDb.load()));
    wetMix.setTargetValue (bypassed ? 0.0f : 1.0f);

    // The calibration sounds go through the EQ, so you hear what it does to them
    calibration.process (buffer);

    if (maxChunkSize > 0)
    {
        // Hosts occasionally send more than the block size they promised, so work in chunks
        for (int start = 0; start < buffer.getNumSamples(); start += maxChunkSize)
            processChunk (buffer, start, std::min (maxChunkSize, buffer.getNumSamples() - start));
    }

    analyzer.push (buffer);
}

void PlaybackManager::processChunk (juce::AudioBuffer<float>& buffer, int start, int length) noexcept
{
    const int numChannels = std::min (buffer.getNumChannels(), dryBuffer.getNumChannels());

    if (! wetMix.isSmoothing() && wetMix.getTargetValue() == 0.0f)
    {
        // Fully bypassed: skip the work, and start the filters clean when the EQ comes back
        isFullyBypassed = true;
        return;
    }

    if (isFullyBypassed)
    {
        filter.clearState();
        crossfeed.reset();
        isFullyBypassed = false;
    }

    const bool isCrossfading = wetMix.isSmoothing();
    if (isCrossfading)
        for (int channel = 0; channel < numChannels; ++channel)
            dryBuffer.copyFrom (channel, 0, buffer, channel, start, length);

    juce::dsp::AudioBlock<float> block (buffer.getArrayOfWritePointers(), (size_t) numChannels, (size_t) start, (size_t) length);
    filter.process (block);
    crossfeed.process (block);

    if (gain.isSmoothing())
    {
        for (int i = 0; i < length; ++i)
            gainRamp[(size_t) i] = gain.getNextValue();
        for (int channel = 0; channel < numChannels; ++channel)
            juce::FloatVectorOperations::multiply (block.getChannelPointer ((size_t) channel), gainRamp.data(), length);
    }
    else if (gain.getCurrentValue() != 1.0f)
    {
        block.multiplyBy (gain.getCurrentValue());
    }

    if (isCrossfading)
    {
        for (int i = 0; i < length; ++i)
            mixRamp[(size_t) i] = wetMix.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* wet = block.getChannelPointer ((size_t) channel);
            const auto* dry = dryBuffer.getReadPointer (channel);
            for (int i = 0; i < length; ++i)
                wet[i] = dry[i] + mixRamp[(size_t) i] * (wet[i] - dry[i]);
        }
    }
}
