/*
  ==============================================================================

    NoiseGenerator.cpp
    Created: 5 Nov 2024 4:30:21pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "NoiseGenerator.h"

NoiseGenerator::NoiseGenerator()
{
}

std::pair<float, float> NoiseGenerator::getNextSample()
{
    float pinkNoiseSample = pinkNoise.generate() * 010.0f;
    pinkNoiseSample = bandpass.processSample (pinkNoiseSample);
    
    if (snapToZeroCounter >= 1000)
    {
        bandpass.snapToZero();
        snapToZeroCounter = 0;
    }
    snapToZeroCounter++;
    
    return { pinkNoiseSample * leftGain, pinkNoiseSample * rightGain };
}

void NoiseGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->sampleRate = spec.sampleRate;
    bandpass.prepare (spec);
}

void NoiseGenerator::setBandwidth (float bandwidth)
{
    this->bandwidth = bandwidth;
}

void NoiseGenerator::setPan (float pan)
{
    this->pan = pan;
    float angle = (pan + 1.0f) * M_PI / 4.0f; // Map pan from [-1, 1] to angle [0, π/2]
    leftGain = std::cos (angle);
    rightGain = std::sin (angle);
}

void NoiseGenerator::setBandpass (float centerFreq)
{
    float freq = std::min (std::max (centerFreq, 20.0f), sampleRate * 0.49f);
    
    *bandpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, freq, Band::bandwidthToQFactor (bandwidth));
}
