/*
  ==============================================================================

    SineSweepGenerator.cpp
    Created: 28 Jul 2024 7:34:18pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SineSweepGenerator.h"

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
