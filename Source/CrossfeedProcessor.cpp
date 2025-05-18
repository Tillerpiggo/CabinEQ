
/*
  ==============================================================================

    CrossfeedProcessor.cpp
    Created: 29 Oct 2024 10:00:00am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CrossfeedProcessor.h"

CrossfeedProcessor::CrossfeedProcessor()
{
    // Default values are set in the header
}

void CrossfeedProcessor::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    // Max delay of 1 second, plus a little buffer for block processing.
    // This could be made configurable if needed.
    maxBufferSamples = (int)std::ceil(sampleRate * 1.0) + spec.maximumBlockSize; 

    leftDelayBuffer.setSize (1, maxBufferSamples);
    rightDelayBuffer.setSize (1, maxBufferSamples);
    leftDelayBuffer.clear();
    rightDelayBuffer.clear();
    
    writePosition = 0;
}

void CrossfeedProcessor::setDelaySamples (int samples)
{
    // Ensure delaySamples is not negative and not greater than what buffer can hold minus block size
    // to prevent reading unwritten data or writing out of bounds.
    // The max usable delay is maxBufferSamples - spec.maximumBlockSize (from prepare context)
    // However, spec is not available here. For simplicity, we cap it at maxBufferSamples - 1.
    // A more robust solution might involve checking against a stored maximumBlockSize.
    delaySamples = juce::jmax (0, juce::jmin (samples, maxBufferSamples - 1));
}

void CrossfeedProcessor::setCrossfeedVolume (float volume)
{
    crossfeedVolume = juce::jlimit (0.0f, 1.0f, volume);
}

void CrossfeedProcessor::setEnabled (bool enabled)
{
    isEnabled = enabled;
}

void CrossfeedProcessor::process (juce::dsp::AudioBlock<float>& block)
{
    if (!isEnabled)
    {
        return;
    }

    const int numChannels = block.getNumChannels();
    const int numSamples = block.getNumSamples();

    // This processor is intended for stereo signals
    if (numChannels < 2 || maxBufferSamples == 0)
    {
        return;
    }

    auto* leftChannelData = block.getChannelPointer (0);
    auto* rightChannelData = block.getChannelPointer (1);

    auto* leftDelayData = leftDelayBuffer.getWritePointer (0);
    auto* rightDelayData = rightDelayBuffer.getWritePointer (0);

    for (int i = 0; i < numSamples; ++i)
    {
        // Store current samples into delay buffers before processing
        // to use this sample for the other channel's delay in the future
        leftDelayData[writePosition] = leftChannelData[i];
        rightDelayData[writePosition] = rightChannelData[i];

        // Calculate read position for the delay
        int readPosition = (writePosition - delaySamples + maxBufferSamples) % maxBufferSamples;

        // Get delayed samples
        float delayedLeftSample = leftDelayData[readPosition];
        float delayedRightSample = rightDelayData[readPosition];

        // Apply crossfeed
        // Left channel gets a bit of the delayed right channel
        float originalLeft = leftChannelData[i];
        leftChannelData[i] = originalLeft + (delayedRightSample * crossfeedVolume);

        // Right channel gets a bit of the delayed left channel
        float originalRight = rightChannelData[i];
        rightChannelData[i] = originalRight + (delayedLeftSample * crossfeedVolume);

        writePosition = (writePosition + 1) % maxBufferSamples;
    }
} 
