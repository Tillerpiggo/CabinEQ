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
    for (int i = 0; i < order; ++i)
    {
        lowPassFilters.push_back (juce::dsp::IIR::Filter<float>());
        highPassFilters.push_back (juce::dsp::IIR::Filter<float>());
    }
}

std::pair<float, float> NoiseGenerator::getNextSample()
{
    if (isMuted)
        return { 0.0f, 0.0f };
    
    if (shouldUpdateGenerators)
    {
        float lowFreq = std::min (std::max (centerFreq * std::pow (2.0f, -bandwidth), 20.0f), sampleRate * 0.49f);
        float highFreq = std::min (std::max (centerFreq * std::pow (2.0f, bandwidth), 20.0f), sampleRate * 0.49f);
        
        for (int i = 0; i < order; ++i)
        {
            *lowPassFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, highFreq);
            *highPassFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, lowFreq);
        }
        shouldUpdateGenerators = false;
    }
    
    if (shouldUpdatePan)
    {
        float angle = (pan + 1.0f) * M_PI / 4.0f; // Map pan from [-1, 1] to angle [0, π/2]
        leftGain = std::cos (angle);
        rightGain = std::sin (angle);
        
//        // This is reversed... but it works really well??
//        leftDelayLine.setDelay (abs (fmin (pan, 0)) * 29);
//        rightDelayLine.setDelay (abs (fmax (pan, 0)) * 29);
        
//        // "Correct" order"
//        leftDelayLine.setDelay (abs (fmax (pan, 0)) * 29);
//        rightDelayLine.setDelay (abs (fmin (pan, 0)) * 29);
//        leftGain = 1.0f;
//        rightGain = 1.0f;
//        float slopeGain = juce::Decibels::decibelsToGain (-1.5 * std::log2 (centerFreq / 1000.0f));
//        leftGain *= slopeGain;
//        rightGain *= slopeGain;
    }
    
    float pinkNoiseSample = pinkNoise.generate() * 10.0f * totalGain;
//    pinkNoiseSample = bandpass.processSample (pinkNoiseSample);
//    pinkNoiseSample = bandpass2.processSample (pinkNoiseSample);
//    pinkNoiseSample = bandpass3.processSample (pinkNoiseSample);
//    pinkNoiseSample = bandpass4.processSample (pinkNoiseSample);
    for (int i = 0; i < order; ++i)
    {
        pinkNoiseSample = lowPassFilters[i].processSample (pinkNoiseSample);
        pinkNoiseSample = highPassFilters[i].processSample (pinkNoiseSample);
    }
    
    if (snapToZeroCounter >= 1000)
    {
        bandpass.snapToZero();
        bandpass2.snapToZero();
        bandpass3.snapToZero();
        bandpass4.snapToZero();
        for (int i = 0; i < order; ++i)
        {
            lowPassFilters[i].snapToZero();
            highPassFilters[i].snapToZero();
        }
        snapToZeroCounter = 0;
    }
    snapToZeroCounter++;
    
    // Delay appropriately
//    leftDelayLine.pushSample (0, pinkNoiseSample * leftGain);
//    rightDelayLine.pushSample (0, pinkNoiseSample * rightGain);
//    float leftSample = leftDelayLine.popSample (0);
//    float rightSample = rightDelayLine.popSample (0);
    
//    return { leftSample, rightSample };
    return { pinkNoiseSample * leftGain, pinkNoiseSample * rightGain };
}

void NoiseGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->sampleRate = spec.sampleRate;
    bandpass.prepare (spec);
    bandpass2.prepare (spec);
    bandpass3.prepare (spec);
    bandpass4.prepare (spec);
    
    for (int i = 0; i < order; ++i)
    {
        lowPassFilters[i].prepare (spec);
        highPassFilters[i].prepare (spec);
    }
    
//    leftDelayLine.reset();
//    rightDelayLine.reset();
//    leftDelayLine.prepare (spec);
//    rightDelayLine.prepare (spec);
}

void NoiseGenerator::setBandwidth (float bandwidth)
{
    if (this->bandwidth != bandwidth)
    {
        this->bandwidth = bandwidth;
        shouldUpdateGenerators = true;
    }
}

void NoiseGenerator::setPan (float pan)
{
    if (this->pan != pan)
    {
        this->pan = pan;
        shouldUpdatePan = true;
    }
}

void NoiseGenerator::setVolumeDB (float volumeDB)
{
    this->totalGain = juce::Decibels::decibelsToGain (volumeDB);
}

void NoiseGenerator::setVolumeGain (float volumeGain)
{
    this->totalGain = volumeGain;
}

void NoiseGenerator::mute()
{
    isMuted = true;
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
    if (this->centerFreq != centerFreq)
    {
        this->centerFreq = centerFreq;
        shouldUpdateGenerators = true;
        shouldUpdatePan = true;
        isMuted = false;
    }
    
    float freq = std::min (std::max (centerFreq, 20.0f), sampleRate * 0.49f);
    float lowFreq = std::min (std::max (centerFreq * std::pow (2.0f, -bandwidth), 20.0f), sampleRate * 0.49f);
    float highFreq = std::min (std::max (centerFreq * std::pow (2.0f, bandwidth), 20.0f), sampleRate * 0.49f);
//    float erbBandwidth = 24.7f * (4.37f * freq / 1000.0f + 1.0f);
//    erbBandwidth = std::log2 (1.0f + erbBandwidth / freq) * bandwidth;
//    
//    *bandpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, freq, Band::bandwidthToQFactor (bandwidth));
    
//    *bandpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, freq);
//    *bandpass2.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, freq);
//    *bandpass3.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, freq);
//    *bandpass4.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, freq);
    
//    auto lowPassCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod (highFreq, sampleRate, order);
//    auto highPassCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod (lowFreq, sampleRate, order);
//    
    for (int i = 0; i < order; ++i)
    {
        *lowPassFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, highFreq);
        *highPassFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, lowFreq);
    }
//
}
