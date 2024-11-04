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
        case Glyph::Type::grid:
            nextSample = { 0.0f, 0.0f };
            for (auto& pointGenerator : pointGenerators)
            {
                auto pointSample = pointGenerator.getNextSample();
                nextSample.first += pointSample.first;
                nextSample.second += pointSample.second;
            }
            break;
    }
    
//    float pinkNoiseCenterSample = pinkNoiseCenter.generate();
//    float pinkNoiseLeftSample = pinkNoiseLeft.generate();
//    float pinkNoiseRightSample = pinkNoiseRight.generate();
//    
//    nextSample.first += pinkNoiseCenterSample + pinkNoiseLeftSample * 2.0f;
//    nextSample.second += pinkNoiseCenterSample + pinkNoiseRightSample * 2.0f;
    
    return nextSample;
}

void GlyphGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
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
        case Glyph::Type::grid:
            preparePointGenerators();
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
        case Glyph::Type::grid:
            return 1000.0f;
    }
}

void GlyphGenerator::preparePointGenerators()
{
    // add + prepare all point generators
    for (int i = 0; i < glyph->getPoints().size() - pointGenerators.size(); ++i)
        pointGenerators.push_back (NoiseSweepGenerator());
    
    for (auto& pointGenerator : pointGenerators)
        pointGenerator.prepare (spec);
    
    auto& points = glyph->getPoints();
    for (int i = 0; i < pointGenerators.size(); ++i)
    {
        std::pair<float, float> leftPoint = points[i];
        std::pair<float, float> rightPoint = points[i];
        leftPoint.second *= -1;
        rightPoint.second *= 1;
        pointGenerators[i].setSweepPattern (SweepPattern ({ leftPoint, rightPoint }, 1.0f + i * 0.1f, spec.sampleRate));
        pointGenerators[i].setBandwidth (0.5f);
    }
}
