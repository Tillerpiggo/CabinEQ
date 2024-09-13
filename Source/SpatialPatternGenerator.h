/*
  ==============================================================================

    SpatialPatternGenerator.h
    Created: 12 Sep 2024 11:53:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SpatialNoiseGenerator.h"

class SpatialPatternGenerator {
public:
    SpatialPatternGenerator();

    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setCentralFrequency (float centralFreq);
    void setAmplCurve (Curve& amplCurve);

private:
    SpatialNoiseGenerator noiseGenerator;
    int patternInterval;
    int sampleCounter;
    float centralFrequency;
};

