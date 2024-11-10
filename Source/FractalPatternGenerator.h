/*
  ==============================================================================

    FractalPatternGenerator.h
    Created: 9 Nov 2024 7:32:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "NoiseGenerator.h"
#include "GainEnvelope.h"
#include "FractalPattern.h"

// Allows the easy playing of sweep patterns that all follow a single fixed path
class FractalPatternGenerator
{
public:
    FractalPatternGenerator();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setPattern (FractalPattern fractalPattern);
    void setSpeedFactor (float speedFactor);
    void setBandwidth (float bandwidth);
    void setNumConfoundingGenerators (int numConfoundingGenerators);
    void mute();
    
    std::optional<float> getCurrPlayingFreq();
    
private:
    void updateBandwidth();
    void updateNoiseGenerators();
    void updateGeneratorBandpassFilters(); // update the actual panning/bandwidth of the bandpass filters used for each noise generator
    
    juce::dsp::ProcessSpec spec;
    
    std::optional<FractalPattern> fractalPattern;
    bool isMuted = false;
    
    NoiseGenerator hiddenGenerator;
    std::vector<NoiseGenerator> confoundingGenerators;
    int numConfoundingGenerators = 3;
    std::vector<float> offsets { 0.25, 0.5, 0.75 };
    
    float bandwidth = 1.0f;
    float speedFactor = 1.0f;
    float currTime = 0.0f; // value in the cycle, from 0 to 1, where we're at
    float timeIncrement = 1.0f / 40000.0f;
    
    // Hidden pattern
    std::vector<bool> hits { 1, 0, 1, 0 }; // true for a hit and false for a rest
    GainEnvelope gainEnvelope;
    float hitDurationInSamples = 2000; // TODO: arbitrary + change this to seconds + maybe make this scale with speed as well
    float sampleIdx = 0;
    float hitIdx = 0;
};
