/*
  ==============================================================================

    DelayFilter.cpp
    Created: 20 Aug 2024 10:37:31pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "DelayFilter.h"

std::pair<float, float> DelayFilter::processSample (std::pair<float, float> sample)
{
    auto [leftSample, rightSample] = sample;
    
    if (delayInSamples < 0)
    {
        buffer.push (leftSample);
    }
    else
    {
        buffer.push (rightSample);
    }
    
    if (buffer.size() >= std::abs (delayInSamples))
    {
        if (delayInSamples < 0)
        {
            leftSample = buffer.front();
            buffer.pop();
        }
        else
        {
            rightSample = buffer.front();
            buffer.pop();
        }
    }
    
    return { leftSample, rightSample };
}

void DelayFilter::clear()
{
    while (! buffer.empty()) buffer.pop();
}

void DelayFilter::setDelay (float delayInSamples)
{
    this->delayInSamples = delayInSamples;
    clear();
}
