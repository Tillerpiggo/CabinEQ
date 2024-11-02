/*
  ==============================================================================

    GlyphGenerator.cpp
    Created: 29 Oct 2024 4:45:36pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "GlyphGenerator.h"

GlyphGenerator::GlyphGenerator()
{
    
}

std::pair<float, float> GlyphGenerator::getNextSample()
{
    if (isMuted || ! glyph.has_value())
        return { 0.0f, 0.0f };
    
    return noiseSweepGenerator.getNextSample();
}

void GlyphGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    noiseSweepGenerator.prepare (spec);
}

void GlyphGenerator::setGlyph (Glyph glyph)
{
    this->glyph = glyph;
    noiseSweepGenerator.setSweepPattern (glyph.getSweepPattern());
    isMuted = false;
}

void GlyphGenerator::mute()
{
    isMuted = true;
}

void GlyphGenerator::setListener (SequencerListener* listener)
{
    noiseSweepGenerator.setListener (listener);
}
