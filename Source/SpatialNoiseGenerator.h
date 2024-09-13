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
    SpatialNoiseGenerator();
    
    std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void setAmplCurve (Curve amplCurve);
    void setBandpass (float centralFreq, float bandwidth);

private:
    void fillBuffer();

    float sampleRate;
    int bufferSize;
    int bufferIndex;
    std::vector<float> buffer;
    juce::Random random;
    static const int numSinWaves = 500;
    std::vector<float> frequencies;
    std::vector<float> amplitudes;
    int crossfadeLength = 100;

    Curve amplCurve;
    juce::IIRFilter bandpassFilter;
    float centralFrequency;
    float bandwidth;
    bool toggle;
};
