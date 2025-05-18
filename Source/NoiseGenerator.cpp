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
        float lowFreq = std::min(std::max(centerFreq * std::pow(2.0f, -bandwidth), 20.0f), sampleRate * 0.49f);
        float highFreq = std::min(std::max(centerFreq * std::pow(2.0f, bandwidth), 20.0f), sampleRate * 0.49f);
        
        // Configure bandpass filter
//        float Q = 1.0f / (2.0f * std::sinh(std::log(2.0f) * bandwidth / 2.0f));
//        *bandpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, centerFreq, Q);
        
//        // Add notch filter at center frequency
//        *notchFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
//            sampleRate, 
//            centerFreq, 
//            notchQ, 
//            juce::Decibels::decibelsToGain(notchDepth)
//        );
        
        // Comment out the existing low/high pass filter configuration
        
        for (int i = 0; i < order; ++i)
        {
            *lowPassFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, highFreq);
            *highPassFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, lowFreq);
        }
//        
        shouldUpdateGenerators = false;
    }
    
    if (shouldUpdatePan)
    {
        float angle = (pan + 1.0f) * M_PI / 4.0f; // Map pan from [-1, 1] to angle [0, π/2]
        leftGain = std::cos(angle);
        rightGain = std::sin(angle);
        shouldUpdatePan = false;
    }
    
    float pinkNoiseSample = random.nextFloat() * 2.0f - 1.0f;
    pinkNoiseSample *= 10.0f * totalGain;
    
//    // Use bandpass filter instead of separate high/low pass filters
//    pinkNoiseSample = bandpass.processSample(pinkNoiseSample);
    
    // Comment out the existing filter processing
//    
    for (int i = 0; i < order; ++i)
    {
        pinkNoiseSample = lowPassFilters[i].processSample(pinkNoiseSample);
        pinkNoiseSample = highPassFilters[i].processSample(pinkNoiseSample);
    }
    
    
//    // Apply the notch filter after the bandpass filtering
//    pinkNoiseSample = notchFilter.processSample(pinkNoiseSample);
    
    if (snapToZeroCounter >= 1000)
    {
//        bandpass.snapToZero();
//        notchFilter.snapToZero();
        for (int i = 0; i < order; ++i)
        {
            lowPassFilters[i].snapToZero();
            highPassFilters[i].snapToZero();
        }
        snapToZeroCounter = 0;
    }
    snapToZeroCounter++;
    
    return { pinkNoiseSample * leftGain, pinkNoiseSample * rightGain };
}

void NoiseGenerator::prepare(const juce::dsp::ProcessSpec& spec)
{
    this->sampleRate = spec.sampleRate;
    bandpass.prepare(spec);
    notchFilter.prepare(spec);
    
    for (int i = 0; i < order; ++i)
    {
        lowPassFilters[i].prepare(spec);
        highPassFilters[i].prepare(spec);
    }
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

void NoiseGenerator::setBandpass(float centerFreq)
{
    if (this->centerFreq != centerFreq)
    {
        this->centerFreq = centerFreq;
        shouldUpdateGenerators = true;
        shouldUpdatePan = true;
        isMuted = false;
    }
    
    float freq = std::min(std::max(centerFreq, 20.0f), sampleRate * 0.49f);
    
    // Configure bandpass filter
//    float Q = 1.0f / (2.0f * std::sinh(std::log(2.0f) * bandwidth / 2.0f));
//    *bandpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, freq, Q);
    
//    // Add notch filter at center frequency
//    *notchFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
//        sampleRate, 
//        freq, 
//        notchQ, 
//        juce::Decibels::decibelsToGain(notchDepth)
//    );
    
    // Comment out the old filter configuration
    
    float lowFreq = std::min(std::max(centerFreq * std::pow(2.0f, -bandwidth), 20.0f), sampleRate * 0.49f);
    float highFreq = std::min(std::max(centerFreq * std::pow(2.0f, bandwidth), 20.0f), sampleRate * 0.49f);
    
    for (int i = 0; i < order; ++i)
    {
        *lowPassFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, highFreq);
        *highPassFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, lowFreq);
    }
//    
}
