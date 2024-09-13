/*
  ==============================================================================

    SpatialNoiseGenerator.h
    Created: 12 Sep 2024 11:10:34pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"

class SpatialNoiseGenerator {
public:
    SpatialNoiseGenerator()
    
    std::pair<float, float> getNextSample();
    void setSampleRate(float newSampleRate);
    
    void setAmplCurve (Curve amplCurve);

private:
    void fillBuffer();
    
    float sampleRate;
    int bufferSize;
    int bufferIndex;
    std::vector<float> buffer;
    
    Curve amplCurve;
};
