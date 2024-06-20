/*
  ==============================================================================

    SineWaveGenerator.cpp
    Created: 14 Jun 2024 3:52:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SineWaveGenerator.h"

void SineWaveGenerator::setSampleRate (double newSampleRate)
{
    sampleRate = newSampleRate;
}

double SineWaveGenerator::getNextSample()
{
    double sample = std::sin(phase) * amplitudeCompensation;
    
    phase += phaseIncrement;
    if (phase > 2.0 * juce::MathConstants<double>::pi)
        phase -= 2.0 * juce::MathConstants<double>::pi;
    
    return sample;
}

void SineWaveGenerator::updatePhaseIncrementAndAmplitudeCompensation()
{
    phaseIncrement = 2.0 * juce::MathConstants<double>::pi * note.frequency / sampleRate;
    amplitudeCompensation = std::pow(TILT, std::log2(note.frequency / REFERENCE_FREQ));
}
