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

//void SineSweepGenerator::setCenterFrequency (float centerFreq, std::optional<float> ampl)
//{
//    this->centerFreq = centerFreq;
//    currFreq = centerFreq;
//    currAmpl = ampl;
//}
//
//void SineSweepGenerator::updateCenterFrequency (float centerFreq, std::optional<float> ampl)
//{
//    this->centerFreq = centerFreq;
//    currAmpl = ampl;
//}

void SineSweepGenerator::setSweep (float bottomFreq, float topFreq, Curve amplCurve, Curve panCurve)
{
    this->centerFreq = std::sqrt (topFreq * bottomFreq);
    this->FREQ_RANGE_FACTOR = topFreq / centerFreq;
    this->amplCurve = amplCurve;
    this->panCurve = panCurve;
    
    currFreq = centerFreq;
}

void SineSweepGenerator::updateSweep (float bottomFreq, float topFreq, Curve amplCurve, Curve panCurve)
{
    this->centerFreq = std::sqrt (topFreq * bottomFreq);
    this->FREQ_RANGE_FACTOR = topFreq / centerFreq;
    this->amplCurve = amplCurve;
    this->panCurve = panCurve;
}

void SineSweepGenerator::incrementFreq()
{
    if (currStep >= TEMPO)
    {
        currStep = 0;
        
//        if (currFreq >= centerFreq / FREQ_STEP && currFreq <= centerFreq * FREQ_STEP)
//            currFreq = centerFreq;
        if (currFreq > centerFreq * FREQ_RANGE_FACTOR)
        {
            increasingFreq = false;
        }
        if (currFreq < centerFreq / FREQ_RANGE_FACTOR)
        {
            increasingFreq = true;
        }
        
        if (increasingFreq) currFreq *= FREQ_STEP;
        else currFreq /= FREQ_STEP;
        
//        float dbDifference = currAmpl.has_value() ? currAmpl.value() : 0.0f;
        float dbDifference = juce::Decibels::gainToDecibels (amplCurve.valueAtFrequency (currFreq));
        float pan = juce::Decibels::gainToDecibels (panCurve.valueAtFrequency (currFreq));
        sineWaveGenerator.setNote (Note (currFreq, BASE_DB + dbDifference, pan));
    }
    
    currStep++;
}

float SineSweepGenerator::getCurrFreq() const
{
    return currFreq;
}
