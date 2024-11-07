/*
  ==============================================================================

    AudioPattern.cpp
    Created: 30 Oct 2024 1:41:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "AudioPattern.h"

AudioPattern::AudioPattern()
    : type (Type::none)
{
}

AudioPattern::AudioPattern (Type type, std::optional<SweepPattern> sweepPattern, std::optional<std::vector<NoiseNote>> noiseNotes)
{
    this->type = type;
    this->sweepPattern = sweepPattern;
    this->noiseNotes = noiseNotes;
}

AudioPattern AudioPattern::spatial (SweepPattern sweepPattern)
{
    return AudioPattern (Type::spatial, sweepPattern, std::nullopt);
}

AudioPattern AudioPattern::intelligibility (std::vector<NoiseNote> noiseNotes)
{
    return AudioPattern (Type::intelligibility, std::nullopt, noiseNotes);
}

void AudioPattern::applyToSequencer (GlyphGenerator& glyphGenerator)
{
    if (type != Type::spatial)
    {
        glyphGenerator.mute();
        return;
    }
    
//    glyphGenerator.setGlyph (Glyph (sweepPattern.value()));
}

void AudioPattern::applyToSequencer (MelodicNoiseSequencer& melodicNoiseSequencer)
{
    if (type != Type::intelligibility)
    {
        melodicNoiseSequencer.mute();
        return;
    }
    
    melodicNoiseSequencer.setPattern (noiseNotes.value());
}
