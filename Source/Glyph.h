/*
  ==============================================================================

    Glyph.h
    Created: 30 Oct 2024 2:13:51pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "SweepPattern.h"
#include "MelodicNotes.h"

// Data structure for defining spatial patterns for playback via GlyphGenerator. For now, it is just a wrapper around SweepPattern
class Glyph
{
public:
    enum class Type
    {
        sweep,
        pattern,
        grid // a bunch of (frequency, pan) points
    };
    
    Glyph (SweepPattern sweepPattern)
        : type (Type::sweep), sweepPattern (sweepPattern)
    {}
    
    Glyph (MelodicNotes melodicNotes)
        : type (Type::pattern), melodicNotes (melodicNotes)
    {}
    
    Glyph (std::vector<std::pair<float, float>> points)
        : type (Type::grid), points (points)
    {}
    
    const Type getType() const
    {
        return type;
    }
    
    const std::optional<SweepPattern> getSweepPattern() const
    {
        return sweepPattern;
    }
    
    const std::optional<std::vector<NoiseNote>> getSpatialPattern()
    {
        if (! melodicNotes.has_value())
            return std::nullopt;
        return melodicNotes->noiseNotes();
    }
    
    const std::vector<std::pair<float, float>> getPoints() const
    {
        return points;
    }
    
private:
    Type type;
    std::optional<SweepPattern> sweepPattern;
    std::optional<MelodicNotes> melodicNotes;
    std::vector<std::pair<float, float>> points;
};
