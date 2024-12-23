/*
  ==============================================================================

    GlyphManager.h
    Created: 13 Nov 2024 11:45:39pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Glyph.h"

// A super simple class to manage a sequence of glyphs. Exists to hold any extra logic we might want in the future
class GlyphManager
{
public:
    GlyphManager (std::vector<Glyph> glyphs = {});
    
    void addGlyph (int archetypeId, juce::Point<float> centerPos);
    void addGlyph (ArchetypalGlyph archetype, juce::Point<float> centerPos, float sizeFactor);
    void removeGlyph (int glyphId);
    void addArchetypalGlyphs (std::vector<ArchetypalGlyph> newGlyphs);
    void addArchetypalGlyph (ArchetypalGlyph glyph);
    
    const std::vector<Glyph>& getGlyphs();
    const std::vector<ArchetypalGlyph>& getArchetypalGlyphs();
    
private:
    void updateNextAvailableId();
    
    std::vector<Glyph> glyphs; // active glyphs that can be used to play sound
    std::vector<ArchetypalGlyph> archetypalGlyphs; // glyphs that can be copied and made active
    int nextAvailableId = 0;
};
