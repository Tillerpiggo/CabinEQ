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
    curveFilter.prepare ({ spec.sampleRate, (juce::uint32) maxChunkSize, (juce::uint32) std::min (spec.numChannels, (juce::uint32) 2) });
    crossfeed.prepare (spec);
    calibration.prepare (spec.sampleRate);

    gain.reset (spec.sampleRate, 0.05);
    gain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (gainDb.load()));
    wetMix.reset (spec.sampleRate, 0.03);
    volume.reset (spec.sampleRate, 0.05);
    volume.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (volumeDb.load()));
    monoMix.reset (spec.sampleRate, 0.03);
    monoMix.setCurrentAndTargetValue (mono ? 1.0f : 0.0f);
    wetMix.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);
    isFullyBypassed = bypassed;
}

void PlaybackManager::setBands (const std::vector<Band>& bands)
{
    filter.setBands (bands);
}

void PlaybackManager::setCurve (std::optional<std::vector<CurvePoint>> points, std::optional<CurveFilter::EarTweaks> tweaks)
{
    curveFilter.setCurve (std::move (points), std::move (tweaks));
}

void PlaybackManager::setGainDb (float newGainDb)
{
    gainDb = newGainDb;
}

void PlaybackManager::setVolumeDb (float newVolumeDb)
{
    volumeDb = newVolumeDb;
}

void PlaybackManager::setBypassed (bool shouldBeBypassed)
{
    bypassed = shouldBeBypassed;
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& buffer) noexcept
{
    gain.setTargetValue (juce::Decibels::decibelsToGain (gainDb.load()));
    wetMix.setTargetValue (bypassed ? 0.0f : 1.0f);
    volume.setTargetValue (juce::Decibels::decibelsToGain (volumeDb.load()));

    // Mono first: each side moves to the average of the two
    monoMix.setTargetValue (mono ? 1.0f : 0.0f);
    if (buffer.getNumChannels() > 1 && (monoMix.isSmoothing() || monoMix.getTargetValue() > 0.0f))
    {
        auto* left = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float mix = monoMix.getNextValue();
            const float middle = 0.5f * (left[i] + right[i]);
            left[i] += mix * (middle - left[i]);
            right[i] += mix * (middle - right[i]);
        }
    }

    // The calibration sounds go through the EQ, so you hear what it does to them
    calibration.process (buffer);

    if (maxChunkSize > 0)
    {
        // Hosts occasionally send more than the block size they promised, so work in chunks
        for (int start = 0; start < buffer.getNumSamples(); start += maxChunkSize)
        {
            const int length = std::min (maxChunkSize, buffer.getNumSamples() - start);
            processChunk (buffer, start, length);
            applyVolume (buffer, start, length);
        }
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
        curveFilter.reset();
        crossfeed.reset();
        isFullyBypassed = false;
    }

    const bool isCrossfading = wetMix.isSmoothing();
    if (isCrossfading)
        for (int channel = 0; channel < numChannels; ++channel)
            dryBuffer.copyFrom (channel, 0, buffer, channel, start, length);

    juce::dsp::AudioBlock<float> block (buffer.getArrayOfWritePointers(), (size_t) numChannels, (size_t) start, (size_t) length);
    filter.process (block);
    curveFilter.process (block);
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

void PlaybackManager::applyVolume (juce::AudioBuffer<float>& buffer, int start, int length) noexcept
{
    const int numChannels = std::min (buffer.getNumChannels(), 2);

    if (volume.isSmoothing())
    {
        for (int i = 0; i < length; ++i)
            gainRamp[(size_t) i] = volume.getNextValue();
        for (int channel = 0; channel < numChannels; ++channel)
            juce::FloatVectorOperations::multiply (buffer.getWritePointer (channel, start), gainRamp.data(), length);
    }
    else if (volume.getCurrentValue() != 1.0f)
    {
        for (int channel = 0; channel < numChannels; ++channel)
            buffer.applyGain (channel, start, length, volume.getCurrentValue());
    }
}
