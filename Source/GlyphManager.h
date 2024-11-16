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
    
    void addGlyphs (std::vector<Glyph> newGlyphs);
    void addGlyph (Glyph glyph);
    
    void setSizeFactor (float sizeFactor);
    void setCenterPos (juce::Point<float> centerPos); // if this would render it out of bounds, it just clips it.
    float getSizeFactor() const;
    juce::Point<float> getCenterPos() const;
    
private:
    bool isInBounds() const;
    void moveGlyphWithinBounds(); // changes centerPos so that glyph is still in bounds
    std::pair<float, float> getXBounds() const; // returns min x and max x for the current size
    std::pair<float, float> getYBounds() const; // returns min y and max y for the current size
    
    std::vector<Glyph> glyphs;
    int glyphIdx = 0;
    float sizeFactor = 1.0f; // can be in (0, 1], with 1 being full-sized
    juce::Point<float> centerPos = { 0.0f, 0.0f }; // can be x in [-1, 1], y in [-1, 1]
};
