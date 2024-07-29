/*
  ==============================================================================

    SineSweepGenerator.cpp
    Created: 28 Jul 2024 7:34:18pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SineSweepGenerator.h"

SineSweepGenerator::SineSweepGenerator()
{}

std::pair<float, float> SineSweepGenerator::getNextSample()
{
    incrementFreq();
    return sineWaveGenerator.getNextSample();
}

void SineSweepGenerator::setSampleRate (float newSampleRate)
{
    sineWaveGenerator.setSampleRate (newSampleRate);
}

void SineSweepGenerator::setFrequencyCenter (float freq)
{
    centerFreq = freq;
}

void SineSweepGenerator::incrementFreq()
{
    if (currStep >= TEMPO)
    {
        currStep = 0;
        // TODO: Implement so that it goes up half the time and down the other half of the time
    }
    
    currStep++;
}
