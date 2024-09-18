/*
  ==============================================================================

    SpatialNoiseGenerator.cpp
    Created: 12 Sep 2024 11:10:34pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SpatialNoiseGenerator.h"

SpatialNoiseGenerator::SpatialNoiseGenerator()
    : sampleRate (0), bufferSize (20000), bufferIndex (0), centralFrequency (0), bandwidth (0)
{
    buffer.resize (bufferSize);
    frequencies.resize (numSinWaves);
    amplitudes.resize (numSinWaves);
    phases.resize (numSinWaves);
    
    for (int i = 0; i < numSinWaves; ++i)
    {
        float minFreq = 20.0f;//std::max (20.0f, centralFrequency * std::pow (2.0f, -0.1f * bandwidth));
        float maxFreq = 20000.0f;//std::min (20000.0f, centralFrequency * std::pow (2.0f, 0.1f * bandwidth));
        frequencies[i] = minFreq * std::pow(10.0f, random.nextFloat() * std::log10(maxFreq / minFreq)); // generate randomly from 20 to 20000hz
        phases[i] = random.nextFloat() * M_PI * 2;
        
        if (bandwidth == 0)
        {
            amplitudes[i] = 0;
            continue;
        }

        amplitudes[i] = juce::Decibels::decibelsToGain (amplCurve.valueAtFrequency(frequencies[i]) +
                                                        -4.5 * std::log2 (frequencies[i] / 1000.0f)); // make it musical pink noise
        float freq = frequencies[i];
        float logDistance = std::abs (std::log2(freq / centralFrequency));
        if (freq > centralFrequency) // make the sound have a long head
            logDistance *= bwHeadFactor;
        else
            logDistance *= bwTailFactor;
        
        logDistance *= logDistance;
        float logRatio = logDistance / bandwidth;
        
        float bandpassGain = std::exp (-5.0 * logRatio);
        amplitudes[i] *= bandpassGain;
    }
    
    fillBuffer();
}

std::pair<float, float> SpatialNoiseGenerator::getNextSample()
{
    float sample = buffer[bufferIndex];
    bufferIndex++;
    if (bufferIndex >= bufferSize)
        bufferIndex = 0;

    if (bufferIndex == 0)
    {
        fillBuffer();
    }

    return std::make_pair(sample, sample);
}

void SpatialNoiseGenerator::setSampleRate(float newSampleRate)
{
    sampleRate = newSampleRate;
}

void SpatialNoiseGenerator::setAmplCurve(Curve amplCurve)
{
    this->amplCurve = amplCurve;
}

void SpatialNoiseGenerator::setBandpass (float centralFreq, float bw, float bwHeadFactor, float bwTailFactor)
{
    centralFrequency = centralFreq;
    bandwidth = bw;
    this->bwHeadFactor = bwHeadFactor;
    this->bwTailFactor = bwTailFactor;
}

void SpatialNoiseGenerator::fillBuffer()
{
    // Update amplitudes based on the current bandpass settings
    for (int i = 0; i < numSinWaves; ++i)
    {
        if (bandwidth == 0)
        {
            amplitudes[i] = 0;
            continue;
        }

        amplitudes[i] = juce::Decibels::decibelsToGain (amplCurve.valueAtFrequency(frequencies[i]) +
                                                        -4.5 * std::log2 (frequencies[i] / 1000.0f));
        float freq = frequencies[i];
        float logDistance = std::abs (std::log2(freq / centralFrequency));
        if (freq > centralFrequency) // make the sound have a long head
            logDistance *= bwHeadFactor;
        else
            logDistance *= bwTailFactor;
        
        logDistance *= logDistance;
        float logRatio = logDistance / bandwidth;
        
        float bandpassGain = std::exp (-5.0 * logRatio);
        amplitudes[i] *= bandpassGain;
    }

    // Calculate phase increments for each sine wave
    std::vector<float> phaseIncrements(numSinWaves);
    for (int j = 0; j < numSinWaves; ++j)
    {
        phaseIncrements[j] = 2.0f * juce::MathConstants<float>::pi * frequencies[j] / sampleRate;
    }

    // Fill the buffer with noise samples
    for (int i = 0; i < bufferSize; ++i)
    {
        float sample = 0.0f;
        for (int j = 0; j < numSinWaves; ++j)
        {
            // Update phase
            phases[j] += phaseIncrements[j];

            // Wrap phase to [0, 2*pi)
            if (phases[j] >= juce::MathConstants<float>::twoPi)
                phases[j] -= juce::MathConstants<float>::twoPi;

            // Compute sine sample
            sample += std::sin(phases[j]) * amplitudes[j];
        }
        buffer[i] = sample / numSinWaves;
        buffer[i] *= 50;
    }
}
