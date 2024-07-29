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

void SineSweepGenerator::setCenterFrequency (float centerFreq)
{
    this->centerFreq = centerFreq;
}

void SineSweepGenerator::incrementFreq()
{
    if (currStep >= TEMPO)
    {
        std::cout << "currFreq: " << currFreq << std::endl;
        currStep = 0;
        // TODO: Implement so that it goes up half the time and down the other half of the time
        if (increasingFreq)
            currFreq *= FREQ_STEP;
        else
            currFreq /= FREQ_STEP;
        
        if (currFreq < centerFreq / FREQ_RANGE_FACTOR)
            increasingFreq = true;
        
        if (currFreq > centerFreq * FREQ_RANGE_FACTOR)
            increasingFreq = false;
        
        float dbDifference = -4.5f * std::log2 (currFreq / 1000.0f);
        sineWaveGenerator.setNote (Note (currFreq, BASE_DB + dbDifference, 0.0f, 0.0f));
    }
    
    currStep++;
}

float SineSweepGenerator::getCurrFreq() const
{
    return currFreq;
}
