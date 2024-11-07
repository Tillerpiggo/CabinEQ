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
        case Glyph::Type::spatialPatterns:
            nextSample = { 0.0f, 0.0f };
            for (auto& spatialGenerator : spatialPatternGenerators)
            {
                auto spatialSample = spatialGenerator->getNextSample();
                nextSample.first += spatialSample.first;
                nextSample.second += spatialSample.second;
            }
            break;
    }
    
    if (glyph->getPitchSweepPattern().has_value())
    {
        setFreqFactor (glyph->getNextPitch());
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
    std::cout << "setting glyph" << std::endl;
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
        case Glyph::Type::spatialPatterns:
            prepareSpatialPatternGenerators();
            break;
    }
    
    isMuted = false;
    
    std::cout << "finished setting glyph" << std::endl;
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
//    spatialPatternGenerator.setFreqFactor (freqFactor);
//    
//    for (auto& generator : spatialPatternGenerators)
//        generator->setFreqFactor (freqFactor);
    
//    for (auto& generator : pointGenerators)
//        generator->setFreqFactor (freqFactor);
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
            return std::nullopt;
        case Glyph::Type::spatialPatterns:
            return std::nullopt;
    }
}

void GlyphGenerator::preparePointGenerators()
{
    // add + prepare all point generators
    size_t numToAdd = glyph->getSweepPatterns().size() - pointGenerators.size();
    for (int i = 0; i < numToAdd; ++i)
        pointGenerators.push_back (NoiseSweepGenerator());
    
    for (auto& pointGenerator : pointGenerators)
        pointGenerator.prepare (spec);
    
    auto& sweepPatterns = glyph->getSweepPatterns();
    for (int i = 0; i < pointGenerators.size(); ++i)
    {
        pointGenerators[i].setSweepPattern (sweepPatterns[i]);
        pointGenerators[i].setBandwidth (0.25f);
    }
}

void GlyphGenerator::prepareSpatialPatternGenerators()
{
    // add + prepare all spatial pattern generators
    std::cout << "glyph->getSpatialPatterns().size(): " << glyph->getSpatialPatterns().size() << "Spatialsize: " << spatialPatternGenerators.size() << std::endl;
    
    int numToAdd = glyph->getSpatialPatterns().size() - spatialPatternGenerators.size();
    for (int i = 0; i < numToAdd; ++i)
    {
        std::cout << "adding generator" << std::endl;
        spatialPatternGenerators.push_back (std::make_unique<SpatialPatternGenerator>());
        spatialPatternGenerators[spatialPatternGenerators.size() - 1]->prepare (spec);
    }
    
    
    std::cout << "Spatialsize: " << spatialPatternGenerators.size() << std::endl;
    
    auto& patterns = glyph->getSpatialPatterns();
    for (int i = 0; i < spatialPatternGenerators.size(); ++i)
    {
        if (i < patterns.size())
            spatialPatternGenerators[i]->setPattern (patterns[i].noiseNotes());
        else
            spatialPatternGenerators[i]->mute();
    }
}
