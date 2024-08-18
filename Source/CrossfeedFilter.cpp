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

void CrossfeedFilter::processBlock (juce::AudioBuffer<float>& buffer)
{
    // Store delayed signals before adding back to channel
    int blockSize = buffer.getNumSamples();
    std::vector<float> delayedLeft (blockSize);
    std::vector<float> delayedRight (blockSize);
    
    // Push left channel into leftBuffer and pop leftBuffer into delayedLeft
    auto buf = buffer.getReadPointer (0);
    for (int i = 0; i < blockSize; ++i)
    {
        // as we do this, add what we pop off to the right buffer in reverse
        leftBuffer.push (buf[i]);
        
        if (leftBuffer.size() >= numSamplesToDelay)
        {
            delayedLeft[i] = leftBuffer.front();
            leftBuffer.pop();
        }
    }
    
    // Push right channel into rightBuffer and pop rightBuffer into delayedRight
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        // as we do this, add what we pop off to the left buffer in reverse
        rightBuffer.push (buf[i]);
        
        if (rightBuffer.size() >= numSamplesToDelay)
        {
            delayedRight[i] = rightBuffer.front();
            rightBuffer.pop();
        }
    }
    
    auto leftPtr = buffer.getWritePointer (0); // get write pointer to the left channel
    auto rightPtr = buffer.getWritePointer (1); // get write pointer to the right channel
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        leftPtr[i] += delayedRight[i] * crossfeedGain;
        rightPtr[i] += delayedLeft[i] * crossfeedGain;
    }
}

void CrossfeedFilter::prepare (const juce::dsp::ProcessSpec& spec)
{
    numSamplesToDelay = spec.sampleRate * delayInSeconds;
}

std::vector<float> CrossfeedFilter::pushAndPop (std::queue<float>& queue, juce::AudioBuffer<float>& buffer)
{
    std::vector<float> poppedData (buffer.getNumSamples());
    
    auto buf = buffer.getReadPointer (0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
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
