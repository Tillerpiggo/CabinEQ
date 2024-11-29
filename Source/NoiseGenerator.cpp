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

//void NoiseGenerator::setPan (float pan)
//{
////    this->pan = pan;
////    float angle = (pan + 1.0f) * M_PI / 4.0f; // Map pan from [-1, 1] to angle [0, π/2]
////    leftGain = std::cos (angle);
////    rightGain = std::sin (angle);
//    this->pan = pan;
//    leftGain = fmin (1.0f, -pan + 1.0f);
//    rightGain = fmin (1.0f, pan + 1.0f);
//}


// 3db pan law
//void NoiseGenerator::setPan(float pan)
//{
//    this->pan = pan;
//
//    // Map pan [-1, 1] to angle θ [0, π/2]
//    float theta = (pan) * M_PI / 4.0f;
//
//    // Calculate amplitudes for left (Aamp) and right (Bamp) channels
//    leftGain = (std::sqrt(2.0f) / 2.0f) * (std::cos(theta) - std::sin(theta));
//    rightGain = (std::sqrt(2.0f) / 2.0f) * (std::cos(theta) + std::sin(theta));
//}

//// 6db pan law
//void NoiseGenerator::setPan(float pan)
//{
//    this->pan = pan;
//    leftGain = 0.5f * (1.0f - pan);
//    rightGain = 0.5f * (1.0f + pan);
//}

void NoiseGenerator::setBandpass (float centerFreq)
{
    float freq = std::min (std::max (centerFreq, 20.0f), sampleRate * 0.49f);
//    float erbBandwidth = 24.7f * (4.37f * freq / 1000.0f + 1.0f);
//    erbBandwidth = std::log2 (1.0f + erbBandwidth / freq) * bandwidth;
//    
    *bandpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, freq, Band::bandwidthToQFactor (bandwidth));
}
