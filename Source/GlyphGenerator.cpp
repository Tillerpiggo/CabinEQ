/*
  ==============================================================================

    GlyphGenerator.cpp
    Created: 13 Nov 2024 11:13:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "GlyphGenerator.h"

GlyphGenerator::GlyphGenerator()
{
    noiseGenerator.setBandwidth (bandwidth);
}

std::pair<float, float> GlyphGenerator::getNextSample()
{
    if (! glyph.has_value())
        return { 0.0f, 0.0f };
    
    auto [xPos, yPos] = glyph->positionAtTime (currTime);
    currTime += timeInterval * speedFactor;
    if (currTime >= 1.0f)
        currTime -= 1.0f;
    noiseGenerator.setBandpass (freqFromYPos (yPos));
    noiseGenerator.setPan (panFromXPos (xPos));
    
    return noiseGenerator.getNextSample();
}

void GlyphGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    noiseGenerator.prepare (spec);
    
    timeInterval = 1.0f / (spec.sampleRate * 3.0f); // make time interval 3 seconds
}

void GlyphGenerator::setGlyph (Glyph glyph)
{
    this->glyph = glyph;
}

void GlyphGenerator::setSpeedFactor (float speedFactor)
{
    this->speedFactor = speedFactor;
}

void GlyphGenerator::setBandwidth (float bandwidth)
{
    this->bandwidth = bandwidth;
    noiseGenerator.setBandwidth (bandwidth);
}

void GlyphGenerator::setFrequencyRange (float minFreq, float maxFreq)
{
    this->minFreq = minFreq;
    this->maxFreq = maxFreq;
}

void GlyphGenerator::setPanRange (float leftmostPan, float rightmostPan)
{
    this->leftmostPan = leftmostPan;
    this->rightmostPan = rightmostPan;
}

float GlyphGenerator::freqFromYPos (float yPos)
{
    float normalized = (yPos + 1) / 2.0f;
    
    float logMinFreq = std::log(minFreq);
    float logMaxFreq = std::log(maxFreq);
    float logFreq = logMinFreq + normalized * (logMaxFreq - logMinFreq);
    
    return std::exp(logFreq);
}

float GlyphGenerator::panFromXPos (float xPos)
{
    return xPos;
}

float GlyphGenerator::getCurrPlayingTime()
{
    return currTime;
}
