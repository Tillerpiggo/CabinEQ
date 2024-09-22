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
    void setSampleRate(float newSampleRate);
    void setAmplCurve(Curve amplCurve);
    void setBandpass(float centralFreq, float bandwidth, float bwHeadFactor, float bwTailFactor);

private:
    void precomputeSineTables();
    void updateAmplitudes();

    static const int numSinWaves = 10;

    float sampleRate;
    juce::Random random;

    std::vector<std::vector<float>> sineWaveTables; // Precomputed sine tables for each oscillator
    std::vector<int> tableSizes;                    // Sizes of the sine tables
    std::vector<float> frequencies;
    std::vector<float> amplitudes;
    std::vector<float> phaseIndices;                // Current indices in the sine tables
    std::vector<float> phaseIncrements;             // Increment amounts per sample

    Curve amplCurve;
    float centralFrequency;
    float bandwidth;
    float bwHeadFactor;
    float bwTailFactor;
};
