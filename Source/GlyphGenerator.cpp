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
    noiseSweepGenerator.setBandwidth (0.5f);
}

std::pair<float, float> GlyphGenerator::getNextSample()
{
    if (isMuted || ! glyph.has_value())
        return { 0.0f, 0.0f };
    
    std::pair<float, float> nextSample;
    switch (glyph->getType())
    {
        case Glyph::Type::sweep:
            nextSample = noiseSweepGenerator.getNextSample();
            break;
        case Glyph::Type::pattern:
            nextSample = spatialPatternGenerator.getNextSample();
            break;
    }
    
    float pinkNoiseCenterSample = pinkNoiseCenter.generate();
    float pinkNoiseLeftSample = pinkNoiseLeft.generate();
    float pinkNoiseRightSample = pinkNoiseRight.generate();
    
    nextSample.first += pinkNoiseCenterSample + pinkNoiseLeftSample * 2.0f;
    nextSample.second += pinkNoiseCenterSample + pinkNoiseRightSample * 2.0f;
    
    return nextSample;
}

void GlyphGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    noiseSweepGenerator.prepare (spec);
    spatialPatternGenerator.prepare (spec);
}

void GlyphGenerator::setGlyph (Glyph glyph)
{
    this->glyph = glyph;
    switch (glyph.getType())
    {
        case Glyph::Type::sweep:
            noiseSweepGenerator.setSweepPattern (glyph.getSweepPattern().value());
            break;
        case Glyph::Type::pattern:
            spatialPatternGenerator.setPattern (glyph.getSpatialPattern().value());
            break;
    }
    
    isMuted = false;
}

void GlyphGenerator::setSpeedFactor (float speedFactor)
{
    this->speedFactor = speedFactor;
    noiseSweepGenerator.setSpeedFactor (speedFactor);
    spatialPatternGenerator.setSpeedFactor (speedFactor);
}

void GlyphGenerator::setFreqFactor (float freqFactor)
{
//    noiseSweepGenerator.setFreqFactor (freqFactor);
    spatialPatternGenerator.setFreqFactor (freqFactor);
}

void GlyphGenerator::mute()
{
    isMuted = true;
}

void GlyphGenerator::setListener (SequencerListener* listener)
{
    noiseSweepGenerator.setListener (listener);
    spatialPatternGenerator.setListener (listener);
}

std::optional<float> GlyphGenerator::getCurrPlayingFreq()
{
    if (! glyph.has_value())
        return std::nullopt;
    
    switch (glyph->getType())
    {
        case Glyph::Type::sweep:
            return noiseSweepGenerator.getCurrPlayingFreq();
        case Glyph::Type::pattern:
            return spatialPatternGenerator.getCurrPlayingFreq();
    }
}
