/*
  ==============================================================================

    SpatialPatternGenerator.cpp
    Created: 12 Sep 2024 11:53:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SpatialPatternGenerator.h"

SpatialPatternGenerator::SpatialPatternGenerator()
    : alternationPeriod (20000), sampleCounter (0), useUpperBandpass (true)
{
}

void SpatialPatternGenerator::setSampleRate (float sampleRate)
{
    noiseGenerator.setSampleRate (sampleRate);
}

void SpatialPatternGenerator::setAmplCurve (Curve& amplCurve)
{
    noiseGenerator.setAmplCurve (amplCurve);
}

void SpatialPatternGenerator::setCenterFrequency (float centerFrequency)
{
    this->centerFrequency = centerFrequency;
    updateBandpass();
}

std::pair<float, float> SpatialPatternGenerator::getNextSample()
{
    std::pair<float, float> sample = noiseGenerator.getNextSample();

    sampleCounter++;
    if (sampleCounter >= alternationPeriod)
    {
        sampleCounter = 0;
        useUpperBandpass = !useUpperBandpass;
        updateBandpass();
    }

    return sample;
}

void SpatialPatternGenerator::updateBandpass()
{
    float offset = 0.1f; // Adjust the offset to control the separation between upper and lower bandpass
    float bandpassFrequency = useUpperBandpass ? centerFrequency * (1.0f + offset) : centerFrequency * (1.0f - offset);
    float bandwidth = 0.1f; // Adjust the bandwidth as needed
    noiseGenerator.setBandpass (bandpassFrequency, bandwidth);
}
