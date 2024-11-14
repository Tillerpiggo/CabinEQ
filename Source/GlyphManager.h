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
    Glyph getCurrGlyph();
    void goToNext();
    void goToPrev();
    bool hasNext();
    bool hasPrev();
    
    void addGlyph (Glyph glyph);
    
private:
    std::vector<Glyph> glyphs;
    int glyphIdx = 0;
};
