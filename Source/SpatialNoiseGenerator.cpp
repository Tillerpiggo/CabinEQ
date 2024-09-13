/*
  ==============================================================================

    SpatialNoiseGenerator.cpp
    Created: 12 Sep 2024 11:10:34pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SpatialNoiseGenerator.h"

SpatialNoiseGenerator::SpatialNoiseGenerator()
    : sampleRate (0), bufferSize (1024), bufferIndex (0)
{
    buffer.resize (bufferSize);
    frequencies.resize (numSinWaves);
    amplitudes.resize (numSinWaves);
    fillBuffer();
}

std::pair<float, float> SpatialNoiseGenerator::getNextSample() 
{
    float sample = buffer[bufferIndex];
    bufferIndex = (bufferIndex + 1) % bufferSize;

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

void SpatialNoiseGenerator::setAmplCurve (Curve amplCurve)
{
    this->amplCurve = amplCurve;
}

void SpatialNoiseGenerator::fillBuffer()
{
    // Generate random frequencies and amplitudes for each sine wave
    for (int i = 0; i < numSinWaves; ++i)
    {
        frequencies[i] = random.nextFloat() * sampleRate;
        amplitudes[i] = juce::Decibels::decibelsToGain(amplCurve.valueAtFrequency (frequencies[i]));
    }

    // Fill the buffer with noise samples
    for (int i = 0; i < bufferSize; ++i)
    {
        float sample = 0.0f;

        for (int j = 0; j < numSinWaves; ++j)
        {
            sample += std::sin(2 * juce::MathConstants<float>::pi * frequencies[j] * i / sampleRate) * amplitudes[j];
        }

        buffer[i] = sample / numSinWaves;
        buffer[i] *= 50;

        // Apply linear fade-in/fade-out to the buffer boundaries
        float fadeValue = 1.0f;
        if (i < crossfadeLength)
        {
            fadeValue = static_cast<float>(i) / crossfadeLength;
        }
        else if (i >= bufferSize - crossfadeLength)
        {
            fadeValue = static_cast<float>(bufferSize - i) / crossfadeLength;
        }
        buffer[i] *= fadeValue;
    }
}
