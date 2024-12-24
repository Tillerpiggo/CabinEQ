/*
  ==============================================================================

    GlyphGridPlayer.h
    Created: 23 Dec 2024 3:58:50pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "NoiseGenerator.h"
#include "Glyph.h"
#include "GainEnvelope.h"

// This plays a grid of glyphs simultaneously
class GlyphGridPlayer
{
public:
    GlyphGridPlayer();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setGlyphs (std::vector<Glyph> glyphs);
    void setSpeedFactor (float speedFactor);
    void setBandwidth (float bandwidth);
    void setFrequencyRange (float minFreq, float maxFreq);
    void setMinFreq (float minFreq);
    void setMaxFreq (float maxFreq);
    void setPanRange (float leftmostPan, float rightmostPan); // max leftmostPan is -1 and max rightmostPan is 1
    
    float getCurrPlayingTime();
    
private:
    void addRemoveNoiseGeneratorsIfNeeded();
    void updateNoiseGeneratorsIfNeeded();
    
    std::pair<float, float> getFreqAndPanFromNormalizedCoords (juce::Point<float> coords);
    
    juce::dsp::ProcessSpec spec;
    std::vector<Glyph> glyphs;
    
    std::vector<NoiseGenerator> noiseGenerators;
    GainEnvelope gainEnvelope { 1000 };
    float currTime = 0.0f;
    float timeInterval = 0.0f; // must be set in prepare first
    
    // Settings
    float speedFactor = 1.0f;
    float bandwidth = 1.0f;
    float minFreq = 50.0f;
    float maxFreq = 12000.0f;
    float leftmostPan = -1.0f;
    float rightmostPan = 1.0f;
    
    // Counters
    int updateBandpassCounter = 0;
    int samplesUntilBandpassUpdate = 500;
    
    bool shouldAddRemoveNoiseGenerators = false;
    bool shouldUpdateNoiseGenerators = false;
};
