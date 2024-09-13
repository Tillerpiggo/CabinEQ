/*
  ==============================================================================

    SpatialPatternGenerator.cpp
    Created: 12 Sep 2024 11:53:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SpatialPatternGenerator.h"

SpatialPatternGenerator::SpatialPatternGenerator()
    : patternInterval (44100), // Hardcoded to 1 second at 44.1kHz
      sampleCounter (0),
      centralFrequency (1000.0f) // Default central frequency
{
    noiseGenerator.setCentralFrequency (centralFrequency);
}

std::pair<float, float> SpatialPatternGenerator::getNextSample()
{
    if (++sampleCounter >= patternInterval)
    {
        sampleCounter = 0;
        noiseGenerator.setCentralFrequency (centralFrequency);
    }

    return noiseGenerator.getNextSample();
}

void SpatialPatternGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
//    noiseGenerator.prepare (spec);
}

void SpatialPatternGenerator::setCentralFrequency (float centralFreq)
{
    centralFrequency = centralFreq;
    noiseGenerator.setCentralFrequency (centralFrequency);
}

void SpatialPatternGenerator::setAmplCurve (Curve& amplCurve)
{
    noiseGenerator.setAmplCurve (amplCurve);
}
