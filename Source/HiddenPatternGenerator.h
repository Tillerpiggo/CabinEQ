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
#include "MelodicNoiseSequencer.h"
#include "GainEnvelope.h"

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
    void setConfoundingBandwidthMultiplier (float confoundingBandwidthMultiplier);
    void mute();
    void setIsFrozen (bool isFrozen);
    
    std::optional<float> getCurrPlayingFreq();
    
private:
    void updateBandwidthsAndSpeedFactors();
    
    juce::dsp::ProcessSpec spec;
    
    std::optional<HiddenPattern> hiddenPattern;
    bool isMuted = false;
    bool isFrozen = false;
    
//    NoiseSweepGenerator hiddenGenerator; // for the main pattern
    SpatialPatternGenerator hiddenSequencer; // for the main pattern, played melodically
    std::vector<NoiseSweepGenerator> confoundingGenerators; // for the confounding noise
    
    float hiddenBandwidth = 1.0f;
    float confoundingBandwidth = 2.0f;
    float confoundingBandwidthMultiplier = 1.0f;
    float speedFactor = 1.0f;
    
    // Pattern
    std::vector<bool> hits { 1, 0, 1, 0 }; // true for a hit and false for a rest
    GainEnvelope gainEnvelope;
    float hitDurationInSamples = 3000; // TODO: arbitrary, also change this to seconds
    float sampleIdx = 0;
    float hitIdx = 0;
};
