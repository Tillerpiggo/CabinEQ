/*
  ==============================================================================

    HiddenPatternGenerator.h
    Created: 7 Nov 2024 11:37:04am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "HiddenPattern.h"
#include "NoiseSweepGenerator.h"

// Allows the easy playing of hidden patterns, as well as live adjustments of the bandwidths of the pattern/confounding noise and the tempo
class HiddenPatternGenerator
{
public:
    HiddenPatternGenerator();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setPattern (HiddenPattern hiddenPattern);
    void setSpeedFactor (float speedFactor);
    void setHiddenBandwidth (float hiddenBandwidth);
    void setConfoundingBandwidth (float confoundingBandwidth);
    void mute();
    
    std::optional<float> getCurrPlayingFreq();
    
private:
    void prepareGeneratorsWithHiddenPattern(); // adds and prepares any generates needed to render the pattern, removes unnecessary generators, and sets the patterns for all of them
    
    juce::dsp::ProcessSpec spec;
    
    std::optional<HiddenPattern> hiddenPattern;
    bool isMuted = false;
    
    NoiseSweepGenerator hiddenGenerator; // for the main pattern
    std::vector<NoiseSweepGenerator> confoundingGenerators; // for the confounding noise
    
    float hiddenBandwidth;
    float confoundingBandwidth;
    float speedFactor = 1.0f;
};
