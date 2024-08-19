/*
  ==============================================================================

    CrossfeedFilter.cpp
    Created: 17 Aug 2024 8:20:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CrossfeedFilter.h"

CrossfeedFilter::CrossfeedFilter()
{}

std::pair<float, float> CrossfeedFilter::processSample (std::pair<float, float> sample)
{
    auto [leftSample, rightSample] = sample;
    if (currChannel == Channel::LEFT)
    {
        rightSample = 0.0f;
    }
    if (currChannel == Channel::RIGHT)
    {
        leftSample = 0.0f;
    }
    
    float delayedLeft = 0.0f;
    float delayedRight = 0.0f;
    leftBuffer.push (leftSample);
    rightBuffer.push (rightSample);

    if (leftBuffer.size() >= numSamplesToDelay)
    {
        delayedLeft = leftBuffer.front();
        leftBuffer.pop();
    }
    
    if (rightBuffer.size() >= numSamplesToDelay)
    {
        delayedRight = rightBuffer.front();
        rightBuffer.pop();
    }
    
    return { leftSample + delayedRight * crossfeedGain, rightSample + delayedLeft * crossfeedGain };
}

void CrossfeedFilter::processBlock (juce::AudioBuffer<float>& buffer)
{
    // Mute channel if it's not the one playing
    if (currChannel == Channel::LEFT)
    {
        buffer.clear (1, 0, buffer.getNumSamples()); // clear the right channel
    }
    else if (currChannel == Channel::RIGHT)
    {
        buffer.clear (0, 0, buffer.getNumSamples()); // clear the left channel
    }
    
    // Store delayed signals before adding back to channel
    auto delayedLeft = pushAndPop (leftBuffer, buffer.getReadPointer (0), buffer.getNumSamples());
    auto delayedRight = pushAndPop (rightBuffer, buffer.getReadPointer (1), buffer.getNumSamples());
    
    // Perform the crossfeed
    auto leftPtr = buffer.getWritePointer (0); // get write pointer to the left channel
    auto rightPtr = buffer.getWritePointer (1); // get write pointer to the right channel
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        leftPtr[i] += delayedRight[i] * crossfeedGain;
        rightPtr[i] += delayedLeft[i] * crossfeedGain;
    }
}

void CrossfeedFilter::setSampleRate (const float newSampleRate)
{
    this->sampleRate = newSampleRate;
    updateNumSamplesToDelay();
}

void CrossfeedFilter::clear()
{
    while (! leftBuffer.empty()) leftBuffer.pop();
    while (! rightBuffer.empty()) rightBuffer.pop();
}

void CrossfeedFilter::setCrossfeedGain (float crossfeedGain)
{
    this->crossfeedGain = crossfeedGain;
}

void CrossfeedFilter::setDelay (float delayInMs)
{
    this->delayInMs = delayInMs;
    updateNumSamplesToDelay();
}

void CrossfeedFilter::setChannelPlaying (Channel channel)
{
    this->currChannel = channel;
}

//=================================================================
std::vector<float> CrossfeedFilter::pushAndPop (std::queue<float>& queue, const float* buf, const int numSamples)
{
    std::vector<float> poppedData (numSamples);
    
    for (int i = 0; i < numSamples; ++i)
    {
        queue.push (buf[i]);
        if (queue.size() >= numSamplesToDelay)
        {
            poppedData[i] = queue.front();
            queue.pop();
        }
    }
    
    return poppedData;
}

void CrossfeedFilter::updateNumSamplesToDelay()
{
    numSamplesToDelay = sampleRate * delayInMs * 0.001;
}
