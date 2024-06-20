/*
  ==============================================================================

    SineWaveGenerator.cpp
    Created: 14 Jun 2024 3:52:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SineWaveGenerator.h"

SineWaveGenerator::SineWaveGenerator ()
{
    adsrParams.attack = 0.0;
    adsrParams.decay = 0.0;
    adsrParams.sustain = 0.0;
    adsrParams.release = 0.0;
    adsr.setParameters(adsrParams);
    
    startTimer(500);
}

void SineWaveGenerator::setSampleRate (double newSampleRate)
{
    sampleRate = newSampleRate;
    adsr.setSampleRate (newSampleRate);
}

void SineWaveGenerator::setFrequency (double frequency)
{
    this->frequency = frequency;
}

double SineWaveGenerator::getNextSample ()
{
    if (gainRamp == 0)
    {
        phase = 0;
        currentFrequency = nextFrequency;
        
        isPlayingReferenceFrequency = (currentFrequency != referenceFrequency);
        
        updatePhaseAndAmplitude();
        nextFrequency = -1;
        gainRamp--;
    }
    
    // Calculate sample
    double sample = std::sin(phase) * amplitudeScale;
    
    if (gainRamp > 0)
    {
        gainRamp--;
        sample *= static_cast<double>(gainRamp) / 500.0;
    }
    
    // Increment phase
    phase += phaseIncrement;
    if (phase > 2.0 * juce::MathConstants<double>::pi)
        phase -= 2.0 * juce::MathConstants<double>::pi;
    
    return sample;
}

void SineWaveGenerator::timerCallback()
{
    if (currentFrequency == referenceFrequency)
    {
        nextFrequency = frequency;
        startNote();
    }
    
    else
    {
        nextFrequency = referenceFrequency;
        startNote();
    }
}

void SineWaveGenerator::startNote()
{
    gainRamp = 500;
}

void SineWaveGenerator::updatePhaseAndAmplitude()
{
    phaseIncrement = 2.0 * juce::MathConstants<double>::pi * currentFrequency / sampleRate;
    amplitudeScale = std::pow(tilt, std::log2(currentFrequency / referenceFrequency));
}
