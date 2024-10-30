/*
  ==============================================================================

    Glyph.h
    Created: 30 Oct 2024 2:13:51pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

// Data structure for defining spatial patterns for playback via GlyphGenerator. For now, it is just a wrapper around SweepPattern

#include "SweepPattern.h"

class Glyph
{
public:
    Glyph (SweepPattern sweepPattern)
        : sweepPattern (sweepPattern)
    {}
    
    const SweepPattern& getSweepPattern() const;
    
private:
    SweepPattern sweepPattern;
};
