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
class OldGlyph
{
public:
    enum class Type
    {
        sweep,
        pattern,
        grid, // a bunch of (frequency, pan) points
        spatialPatterns
    };
    
    OldGlyph (SweepPattern sweepPattern)
        : type (Type::sweep), sweepPattern (sweepPattern)
    {}
    
    OldGlyph (MelodicNotes melodicNotes)
        : type (Type::pattern), melodicNotes (melodicNotes)
    {}
    
    OldGlyph (std::vector<SweepPattern> sweepPatterns)
        : type (Type::grid), sweepPatterns (sweepPatterns)
    {}
    
    OldGlyph (std::vector<MelodicNotes> spatialPatterns)
        : type (Type::spatialPatterns), spatialPatterns (spatialPatterns)
    {}
    
    static OldGlyph shapeFromCorners (std::vector<std::pair<float, float>> corners, float bandwidth = 1.0f)
    {
        std::vector<MelodicNotes> spatialPatternsFromCorners;
        for (int i = 0; i < corners.size(); ++i)
        {
            MelodicNotes newNotes =
            MelodicNotes::withFreqs ({ corners[i].first }).withPan (corners[i].second).withCyclingBandwidths ({ bandwidth, 0.0f }).withNoteDurationInSeconds (0.1 + 0.02 * i);
            spatialPatternsFromCorners.push_back (newNotes);
        }
        return OldGlyph (spatialPatternsFromCorners);
    }
    
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
    
    const std::vector<SweepPattern> getSweepPatterns() const
    {
        return sweepPatterns;
    }
    
    const std::vector<MelodicNotes> getSpatialPatterns()
    {
        return spatialPatterns;
    }
    
    std::optional<SweepPattern> getPitchSweepPattern() const
    {
        return pitchSweepPattern;
    }
    
    void setPitchPattern (SweepPattern pitchSweepPattern)
    {
        this->pitchSweepPattern = pitchSweepPattern;
    }
    
    float getNextPitch()
    {
        return pitchSweepPattern->getNextFrequencyAndPan().first;
    }
    
    OldGlyph withPan (float pan)
    {
        std::vector<MelodicNotes> newSpatialPatterns;
        for (const auto& spatialPattern : spatialPatterns)
        {
            newSpatialPatterns.push_back (spatialPattern.withPan (pan));
        }
        OldGlyph newGlyph = OldGlyph (newSpatialPatterns);
        if (pitchSweepPattern.has_value())
            newGlyph.setPitchPattern (pitchSweepPattern.value());
        
        return newGlyph;
    }
    
private:
    Type type;
    std::optional<SweepPattern> sweepPattern;
    std::optional<MelodicNotes> melodicNotes;
    std::vector<SweepPattern> sweepPatterns;
    std::vector<MelodicNotes> spatialPatterns;
    std::optional<SweepPattern> pitchSweepPattern;
};
