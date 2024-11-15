/*
  ==============================================================================

    GlyphGenerator.h
    Created: 13 Nov 2024 11:13:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "NoiseGenerator.h"
#include "Glyph.h"

// Allows the playing of a glyph. It controls the speed, bandwidth, frequency range, panning range, and is initialized with sample rate/spec
class GlyphGenerator
{
public:
    GlyphGenerator();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setGlyph (Glyph glyph);
    void setSpeedFactor (float speedFactor);
    void setBandwidth (float bandwidth);
    void setFrequencyRange (float minFreq, float maxFreq);
    void setPanRange (float leftmostPan, float rightmostPan); // absolute leftmost is -1 and absolute rightmost is 1
    
    float freqFromYPos (float yPos);
    float panFromXPos (float xPos);
    
    float getCurrPlayingTime();
    
private:
    juce::dsp::ProcessSpec spec;
    std::optional<Glyph> glyph;
    
    NoiseGenerator noiseGenerator;
    float currTime = 0.0f;
    float timeInterval = 0.0f; // must be set in prepare
    
    // Variables
    float speedFactor = 1.0f;
    float bandwidth = 1.0f;
    
    // Constants
    float minFreq = 30.0f;
    float maxFreq = 15000.0f;
    float leftmostPan = -1.0f;
    float rightmostPan = 1.0f;
};
