/*
  ==============================================================================

    SweepPattern.cpp
    Created: 20 Oct 2024 11:17:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SweepPattern.h"

SweepPattern::SweepPattern (float centerFreq, float bandwidth, float durationInSeconds, float sampleRate)
    : centerFreq (centerFreq), bandwidth (bandwidth), sampleRate (sampleRate), durationInSeconds (durationInSeconds),
      idx (0), cycleLen (durationInSeconds * sampleRate), currFreq (centerFreq)
{}

float SweepPattern::getNextFreq()
{
    // Sine
//    // Calculate the normalized time (0 to 1) within the cycle
//    float normalizedTime = static_cast<float>(idx) / cycleLen;
//
//    // Calculate the sine wave value (-1 to 1) based on the normalized time
//    float sineValue = std::sin(normalizedTime * 2.0f * juce::MathConstants<float>::pi);
//
//    // Map the sine wave value to the logarithmic frequency range
//    float logCenterFreq = std::log2 (centerFreq);
//    float logMinFreq = logCenterFreq - bandwidth / 2.0f;
//    float logMaxFreq = logCenterFreq + bandwidth / 2.0f;
//    float logCurrFreq = juce::jmap (sineValue, -1.0f, 1.0f, logMinFreq, logMaxFreq);
//    currFreq = std::pow (2.0f, logCurrFreq);
//
//    // Increment the index and wrap around if it reaches the cycle length
//    idx = (idx + 1) % cycleLen;
//
//    return currFreq;
    
    // Linear
    // Calculate the normalized time (0 to 1) within the cycle
    float normalizedTime = static_cast<float>(idx) / cycleLen;

    // Determine the direction of the sweep (0 to 1 for upward, 1 to 0 for downward)
    float sweepDirection = (idx < cycleLen / 2) ? normalizedTime * 2.0f : 2.0f - normalizedTime * 2.0f;

    // Map the sweep direction to the logarithmic frequency range
    float logCenterFreq = std::log2(centerFreq);
    float logMinFreq = logCenterFreq - bandwidth / 2.0f;
    float logMaxFreq = logCenterFreq + bandwidth / 2.0f;
    float logCurrFreq = juce::jmap(sweepDirection, 0.0f, 1.0f, logMinFreq, logMaxFreq);
    currFreq = std::pow(2.0f, logCurrFreq);

    // Increment the index and wrap around if it reaches the cycle length
    idx = (idx + 1) % cycleLen;

    return currFreq;
}

float SweepPattern::getCurrFreq() const
{
    return currFreq;
}
