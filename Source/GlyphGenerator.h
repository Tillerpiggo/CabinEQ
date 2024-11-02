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
#include "Glyph.h"
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
    void mute();
    
    void setListener (SequencerListener* listener);
    
private:
    std::optional<Glyph> glyph;
    bool isMuted = false;
    
    NoiseSweepGenerator noiseSweepGenerator;
    SequencerListener* listener;
    
    float speedFactor = 1.0f;
};
