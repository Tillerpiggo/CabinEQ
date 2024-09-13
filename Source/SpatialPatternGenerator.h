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
    
    void setSampleRate (float sampleRate);
    void setCenterFrequency (float centerFrequency);
    void setAmplCurve (Curve& amplCurve);
    std::pair<float, float> getNextSample();

private:
    void updateBandpass();

    SpatialNoiseGenerator noiseGenerator;
    int alternationPeriod;
    int sampleCounter;
    bool useUpperBandpass;
    float centerFrequency;
};

