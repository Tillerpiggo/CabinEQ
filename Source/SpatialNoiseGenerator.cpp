/*
  ==============================================================================

    SpatialNoiseGenerator.cpp
    Created: 12 Sep 2024 11:10:34pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SpatialNoiseGenerator.h"

SpatialNoiseGenerator::SpatialNoiseGenerator()
    : sampleRate (0), bufferSize (1024), bufferIndex (0), centralFrequency (0), bandwidth (0)
{
    buffer.resize(bufferSize);
    frequencies.resize(numSinWaves);
    amplitudes.resize(numSinWaves);
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

void SpatialNoiseGenerator::setBandpass(float centralFreq, float bw)
{
    centralFrequency = centralFreq;
    bandwidth = bw;
}

void SpatialNoiseGenerator::fillBuffer()
{

    for (int i = 0; i < numSinWaves; ++i)
    {
        float minFreq = 20.0f;
        float maxFreq = 20000.0f;
        frequencies[i] = minFreq * std::pow(10.0f, random.nextFloat() * std::log10(maxFreq / minFreq)); // generate randomly from 20 to 20000hz

        amplitudes[i] = juce::Decibels::decibelsToGain (amplCurve.valueAtFrequency(frequencies[i]) +
                                                        -4.5 * std::log2 (frequencies[i] / 1000.0f));
        float freq = frequencies[i];
        float logDistance = std::abs (std::log2(freq / centralFrequency));
        float logRatio = logDistance / bandwidth;
        float bandpassGain = std::exp (-1.0 * logRatio);
        amplitudes[i] *= bandpassGain;
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
