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
        float minFreq = 20.0f;
        float maxFreq = 20000.0f;
        frequencies[i] = minFreq * std::pow(10.0f, random.nextFloat() * std::log10(maxFreq / minFreq));
        phases[i] = random.nextFloat() * juce::MathConstants<float>::twoPi;
        amplitudes[i] = 0.0f;  // Initialize to zero
    }
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

    sinPrevSamples1.resize(numSinWaves);
    sinPrevSamples2.resize(numSinWaves);
    cosOmegaTs.resize(numSinWaves);
    previousAmplitudes.resize(numSinWaves);

    for (int i = 0; i < numSinWaves; ++i)
    {
        float omegaT = 2.0f * juce::MathConstants<float>::pi * frequencies[i] / sampleRate;
        cosOmegaTs[i] = 2.0f * std::cos(omegaT);

        sinPrevSamples1[i] = amplitudes[i] * std::sin(phases[i]);
        sinPrevSamples2[i] = amplitudes[i] * std::sin(phases[i] - omegaT);

        previousAmplitudes[i] = amplitudes[i];
    }

    fillBuffer();
    bufferIndex = 0;
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
    // Update amplitudes
    for (int i = 0; i < numSinWaves; ++i)
    {
        if (bandwidth == 0)
        {
            amplitudes[i] = 0.0f;
            continue;
        }

        // [Your amplitude calculations remain the same]

        // Adjust sinPrevSamples for amplitude changes
        if (previousAmplitudes[i] != amplitudes[i])
        {
            if (previousAmplitudes[i] == 0.0f)
            {
                // Re-initialize if previous amplitude was zero
                float omegaT = 2.0f * juce::MathConstants<float>::pi * frequencies[i] / sampleRate;
                sinPrevSamples1[i] = amplitudes[i] * std::sin(phases[i]);
                sinPrevSamples2[i] = amplitudes[i] * std::sin(phases[i] - omegaT);
            }
            else
            {
                // Scale previous samples
                float amplitudeRatio = amplitudes[i] / previousAmplitudes[i];
                sinPrevSamples1[i] *= amplitudeRatio;
                sinPrevSamples2[i] *= amplitudeRatio;
            }
            previousAmplitudes[i] = amplitudes[i];
        }
    }

    // Generate samples using recursion
    for (int i = 0; i < bufferSize; ++i)
    {
        float sample = 0.0f;
        for (int j = 0; j < numSinWaves; ++j)
        {
            float s = cosOmegaTs[j] * sinPrevSamples1[j] - sinPrevSamples2[j];
            sinPrevSamples2[j] = sinPrevSamples1[j];
            sinPrevSamples1[j] = s;
            sample += s;
        }
        buffer[i] = (sample / numSinWaves) * 50.0f; // Adjust scaling as needed
    }
}
