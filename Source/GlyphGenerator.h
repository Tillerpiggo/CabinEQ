/*
  ==============================================================================

    GlyphGenerator.h
    Created: 29 Oct 2024 4:45:36pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "NoiseSweepGenerator.h"
#include "SpatialPatternGenerator.h"
#include "Glyph.h"
#include "PinkNoise.h"
#include "SequencerListener.h"

// Allows the easy playing of spatial glyphs. For now, it just plays SweepPatterns, since Glyphs are a wrapper for SweepPattern, but this class will handle more sophisticated sequencing of such patterns in the future.
class GlyphGenerator
{
public:
    GlyphGenerator();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setGlyph (Glyph glyph);
    void setSpeedFactor (float speedFactor);
    void setFreqFactor (float freqFactor);
    void mute();
    
    void setListener (SequencerListener* listener);
    
    std::optional<float> getCurrPlayingFreq();
    
private:
    void preparePointGenerators();
    void prepareSpatialPatternGenerators();
    
    juce::dsp::ProcessSpec spec;
    
    std::optional<Glyph> glyph;
    bool isMuted = false;
    
    PinkNoise pinkNoiseCenter;
    PinkNoise pinkNoiseLeft;
    PinkNoise pinkNoiseRight;
    NoiseSweepGenerator noiseSweepGenerator;
    SpatialPatternGenerator spatialPatternGenerator;
    SequencerListener* listener;
    
    std::vector<NoiseSweepGenerator> pointGenerators;
    std::vector<SpatialPatternGenerator> spatialPatternGenerators;
    
    float speedFactor = 1.0f;
};
