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

class SineLookupTable {
public:
    SineLookupTable(int tableSize = 2048) // Increased table size for better accuracy
        : tableSize(tableSize), lookupTable(tableSize)
    {
        const float twoPi = 2.0f * juce::MathConstants<float>::pi;
        for (int i = 0; i < tableSize; ++i) {
            lookupTable[i] = std::sin(i * twoPi / tableSize);
        }
    }

    float get(float angle) const
    {
        // Normalize angle to [0, 2pi]
        angle = std::fmod(angle, juce::MathConstants<float>::twoPi);
        if (angle < 0) angle += juce::MathConstants<float>::twoPi;

        float index = angle * tableSize / juce::MathConstants<float>::twoPi;
        int lowerIndex = static_cast<int>(index) % tableSize;
        int upperIndex = (lowerIndex + 1) % tableSize;

        float fraction = index - lowerIndex;
        return lookupTable[lowerIndex] * (1.0f - fraction) + lookupTable[upperIndex] * fraction;
    }

private:
    int tableSize;
    std::vector<float> lookupTable;
};

class SpatialNoiseGenerator {
public:
    SpatialNoiseGenerator();

    std::pair<float, float> getNextSample();
    void setSampleRate(float newSampleRate);
    void setAmplCurve(Curve amplCurve);
    void setBandpass(float centralFreq, float bandwidth, float bwHeadFactor, float bwTailFactor);

private:
    void fillBuffer();

    float sampleRate;
    int bufferSize;
    int bufferIndex;
    std::vector<float> buffer;
    juce::Random random;
    static const int numSinWaves = 100;
    std::vector<float> frequencies;
    std::vector<float> amplitudes;
    std::vector<float> phases;          // Added phase storage
    std::vector<float> phaseIncrements; // Added phase increment storage
    int crossfadeLength = 500;

    Curve amplCurve;
    juce::IIRFilter bandpassFilter;
    float centralFrequency;
    float bandwidth;
    float bwHeadFactor;
    float bwTailFactor;
    bool toggle;
    SineLookupTable sineTable;
};
