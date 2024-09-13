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
    juce::Random random;
    const int numSinWaves = 100;

    for (int i = 0; i < bufferSize; ++i) 
    {
        float sample = 0.0f;

        for (int j = 0; j < numSinWaves; ++j) 
        {
            float randomFreq = random.nextFloat() * sampleRate;
            sample += std::sin(2 * juce::MathConstants<float>::pi * randomFreq * i / sampleRate);
        }

        buffer[i] = sample / numSinWaves;
    }
}
