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

void SineSweepGenerator::setCenterFrequency (float centerFreq, std::optional<float> ampl)
{
    this->centerFreq = centerFreq;
    currFreq = centerFreq;
    currAmpl = ampl;
}

void SineSweepGenerator::updateCenterFrequency (float centerFreq, std::optional<float> ampl)
{
    this->centerFreq = centerFreq;
    currAmpl = ampl;
}

void SineSweepGenerator::incrementFreq()
{
    if (currStep >= TEMPO)
    {
        currStep = 0;
        
        if (currFreq >= centerFreq / FREQ_STEP && currFreq <= centerFreq * FREQ_STEP)
            currFreq = centerFreq;
        if (currFreq < centerFreq)
            currFreq *= FREQ_STEP;
        if (currFreq > centerFreq)
            currFreq /= FREQ_STEP;
        
        float dbDifference = currAmpl.has_value() ? currAmpl.value() : 0.0f;
        dbDifference += -4.5f * std::log2 (currFreq / 3100.0f);
//        float dbDifference = 0.0f;
        sineWaveGenerator.setNote (Note (currFreq, BASE_DB + dbDifference, 0.0f, 0.0f));
    }
    
    currStep++;
}

float SineSweepGenerator::getCurrFreq() const
{
    return currFreq;
}
