/*
  ==============================================================================

    AudioPattern.h
    Created: 30 Oct 2024 1:31:08pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "GlyphGenerator.h"
#include "MelodicNoiseSequencer.h"
#include "Glyph.h"

// This represents an audio pattern consisting of either a sweep pattern that can be fed into a GlyphGenerator, or a sequence of noise notes that can be fed into a MelodicNoiseGenerator as well as information about the level of confounding noise.
class AudioPattern
{
public:
    enum class Type
    {
        glyph, // essentially spatial
        melodic, // essentially intelligibility
        none // essentially silent
    };
    
    AudioPattern(); // creates a blank audio pattern where nothing is played
    AudioPattern (Type type, std::optional<SweepPattern> sweepPattern, std::optional<std::vector<NoiseNote>> noiseNotes, std::optional<float> relativeNoiseGain);
    static AudioPattern glyph (SweepPattern sweepPattern);
    static AudioPattern melodic (std::vector<NoiseNote> noiseNotes, float relativeNoiseGain);
    
    void applyToSequencer (GlyphGenerator& glyphGenerator);
    void applyToSequencer (MelodicNoiseSequencer& melodicNoiseSequencer);
    
private:
    Type type;
    std::optional<SweepPattern> sweepPattern;
    std::optional<std::vector<NoiseNote>> noiseNotes;
    std::optional<float> relativeNoiseGain;
};
