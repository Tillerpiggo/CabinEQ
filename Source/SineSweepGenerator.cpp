/*
  ==============================================================================

    SineSweepGenerator.cpp
    Created: 28 Jul 2024 7:34:18pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SineSweepGenerator.h"

SweepPattern::SweepPattern (float centerFreq, float bandwidth, float durationInSeconds, float sampleRate)
    : centerFreq (centerFreq), bandwidth (bandwidth), sampleRate (sampleRate), durationInSeconds (durationInSeconds),
      idx (0), cycleLen (durationInSeconds * sampleRate), currFreq (centerFreq)
{}

float SweepPattern::getNextFreq()
{
    // Calculate the normalized time (0 to 1) within the cycle
    float normalizedTime = static_cast<float>(idx) / cycleLen;

    // Calculate the sine wave value (-1 to 1) based on the normalized time
    float sineValue = std::sin(normalizedTime * 2.0f * juce::MathConstants<float>::pi);

    // Map the sine wave value to the logarithmic frequency range
    float logCenterFreq = std::log2 (centerFreq);
    float logMinFreq = logCenterFreq - bandwidth / 2.0f;
    float logMaxFreq = logCenterFreq + bandwidth / 2.0f;
    float logCurrFreq = juce::jmap (sineValue, -1.0f, 1.0f, logMinFreq, logMaxFreq);
    currFreq = std::pow (2.0f, logCurrFreq);

    // Increment the index and wrap around if it reaches the cycle length
    idx = (idx + 1) % cycleLen;

    return currFreq;
}

float SweepPattern::getCurrFreq() const
{
    return currFreq;
}

SineSweepGenerator::SineSweepGenerator()
{
}

std::pair<float, float> SineSweepGenerator::getNextSample()
{
    if (! sweepPattern.has_value())
        return { 0.0f, 0.0f };
    
    sineWaveGenerator.setFrequency (sweepPattern->getNextFreq());
    return sineWaveGenerator.getNextSample();
}

void SineSweepGenerator::setSampleRate (float newSampleRate)
{
    sineWaveGenerator.setSampleRate (newSampleRate);
}

void SineSweepGenerator::setSweepPattern (SweepPattern sweepPattern) // must be called before getNextSample is called
{
    this->sweepPattern = sweepPattern;
    sineWaveGenerator.setNote (Note (sweepPattern.getCurrFreq(), 0.0f, 0.0f, 0.0f));
}

float SineSweepGenerator::getCurrFreq() const
{
    if (! sweepPattern.has_value())
        return -1;
    return sweepPattern->getCurrFreq();
}
