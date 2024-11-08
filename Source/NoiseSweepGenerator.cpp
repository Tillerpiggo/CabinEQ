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
    if (isMuted || ! sweepPattern.has_value())
        return { 0.0f, 0.0f };
    
    float pinkNoiseSample = pinkNoise.generate() * 10.0f;
    pinkNoiseSample = bandpass.processSample (pinkNoiseSample);
//    pinkNoiseSample = peakFilter.processSample (pinkNoiseSample);
    
    if (snapToZeroCounter >= 1000)
    {
        bandpass.snapToZero();
        peakFilter.snapToZero();
        snapToZeroCounter = 0;
    }
    snapToZeroCounter++;
    
    auto [nextFreq, nextPan] = sweepPattern->getNextFrequencyAndPan (isFrozen ? 0.0f : speedFactor);
    setBandpass (nextFreq * freqFactor);
    setPan (nextPan); // updates leftAmplitudeCompensation and rightAmplitudeCompensation
    
    return { pinkNoiseSample * leftAmplitudeCompensation, pinkNoiseSample * rightAmplitudeCompensation };
}

void NoiseSweepGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->sampleRate = spec.sampleRate;
    bandpass.prepare (spec);
    peakFilter.prepare (spec);
}

void NoiseSweepGenerator::setBandwidth (float bandwidth)
{
    this->bandwidth = bandwidth;
}

void NoiseSweepGenerator::setPan (float pan)
{
    this->pan = pan;
    float angle = (pan + 1.0f) * M_PI / 4.0f; // Map pan from [-1, 1] to angle [0, π/2]
    leftAmplitudeCompensation = std::cos(angle);
    rightAmplitudeCompensation = std::sin(angle);
}

void NoiseSweepGenerator::setSweepPattern (SweepPattern sweepPattern)
{
    this->sweepPattern = sweepPattern;
    auto [currFreq, currPan] = sweepPattern.getCurrFrequencyAndPan();
    setBandpass (currFreq);
    setPan (currPan);
    
    // Make sure to update the sweep pattern with our listener
    this->sweepPattern->setListener (listener);
    isMuted = false;
}

void NoiseSweepGenerator::setPeakFilter (float centerFreq, float bandwidth, float ampl)
{
    float qFactor = Band::bandwidthToQFactor (bandwidth);
    *peakFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, centerFreq, qFactor, juce::Decibels::decibelsToGain (ampl));
}

void NoiseSweepGenerator::setSpeedFactor (float speedFactor)
{
    this->speedFactor = speedFactor;
}

void NoiseSweepGenerator::setListener (SequencerListener* listener)
{
    this->listener = listener;
    std::cout << "set noise sweep generator listener" << std::endl;
    if (sweepPattern.has_value())
        sweepPattern->setListener (listener);
}

void NoiseSweepGenerator::setFreqFactor (float freqFactor)
{
    this->freqFactor = freqFactor;
}

std::optional<float> NoiseSweepGenerator::getCurrPlayingFreq()
{
    if (! sweepPattern.has_value())
        return std::nullopt;
    
    return sweepPattern->getCurrFrequencyAndPan().first;
}

void NoiseSweepGenerator::mute()
{
    isMuted = true;
}

void NoiseSweepGenerator::setIsFrozen (bool isFrozen)
{
    this->isFrozen = isFrozen;
}

void NoiseSweepGenerator::setBandpass (float centreFreq)
{
    float freq = std::min (std::max (centreFreq, 20.0f), sampleRate * 0.49f);
//    float freq = 1000.0f;
//    std::cout << "freq: " << freq << std::endl;
    *bandpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, freq, Band::bandwidthToQFactor (bandwidth));
}
