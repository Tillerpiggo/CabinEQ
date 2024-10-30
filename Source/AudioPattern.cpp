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

AudioPattern::AudioPattern (Type type, std::optional<SweepPattern> sweepPattern, std::optional<std::vector<NoiseNote>> noiseNotes, std::optional<float> relativeNoiseGain)
{
    this->type = type;
    this->sweepPattern = sweepPattern;
    this->noiseNotes = noiseNotes;
    this->relativeNoiseGain = relativeNoiseGain;
}

AudioPattern AudioPattern::glyph (SweepPattern sweepPattern)
{
    return AudioPattern (Type::glyph, sweepPattern, std::nullopt, std::nullopt);
}

AudioPattern AudioPattern::melodic (std::vector<NoiseNote> noiseNotes, float relativeNoiseGain)
{
    return AudioPattern (Type::melodic, std::nullopt, noiseNotes, relativeNoiseGain);
}

void AudioPattern::applyToSequencer (GlyphGenerator& glyphGenerator)
{
    if (type != Type::glyph)
    {
        glyphGenerator.mute();
        return;
    }
    
    glyphGenerator.setGlyph (Glyph (sweepPattern.value()));
}

void AudioPattern::applyToSequencer (MelodicNoiseSequencer& melodicNoiseSequencer)
{
    if (type != Type::melodic)
    {
        melodicNoiseSequencer.mute();
        return;
    }
    
    melodicNoiseSequencer.setPattern (noiseNotes.value(), relativeNoiseGain.value());
}
