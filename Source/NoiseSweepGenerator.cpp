/*
  ==============================================================================

    NoiseSweepGenerator.cpp
    Created: 20 Oct 2024 11:13:55pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "NoiseSweepGenerator.h"

NoiseSweepGenerator::NoiseSweepGenerator()
{
}
    
std::pair<float, float> NoiseSweepGenerator::getNextSample()
{
    if (! sweepPattern.has_value())
        return { 0.0f, 0.0f };
    
    float pinkNoiseSample = pinkNoise.generate();
    pinkNoiseSample = bandpass.processSample (pinkNoiseSample);
    
    if (snapToZeroCounter >= 1000)
    {
        bandpass.snapToZero();
        snapToZeroCounter = 0;
    }
    snapToZeroCounter++;
    
    setBandpass (sweepPattern->getNextFreq());
    
    return { pinkNoiseSample, pinkNoiseSample };
}

void NoiseSweepGenerator::setSampleRate (float sampleRate)
{
    this->sampleRate = sampleRate;
}

void NoiseSweepGenerator::setBandwidth (float bandwidth)
{
    this->bandwidth = bandwidth;
}

void NoiseSweepGenerator::setSweepPattern (SweepPattern sweepPattern)
{
    this->sweepPattern = sweepPattern;
    setBandpass (sweepPattern.getCurrFreq());
}

void NoiseSweepGenerator::setBandpass (float centreFreq)
{
    float freq = std::min (std::max (centreFreq, 20.0f), sampleRate * 0.49f);
    *bandpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, freq, Band::bandwidthToQFactor (bandwidth));
}
