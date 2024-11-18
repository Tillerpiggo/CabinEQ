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
    
    auto [pos, progress] = glyph->positionAtTime (currTime);
    auto [xPos, yPos] = pos;
    currTime += timeInterval * speedFactor;
    if (currTime >= 1.0f)
        currTime -= 1.0f;
    noiseGenerator.setBandpass (freqFromYPos (yPos));
    noiseGenerator.setPan (panFromXPos (xPos));
    
    auto nextSample = noiseGenerator.getNextSample();
    
    // Apply gain envelope based on progress
    float envelope = 1.0f;
    float len = 0.02;
    if (progress < len)
        envelope = progress / len;
    if (progress > (1.0f - len))
        envelope = (1.0f - progress) / len;
    
    nextSample.first *= envelope;
    nextSample.second *= envelope;
    
    return nextSample;
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

void GlyphGenerator::setCenterPos (juce::Point<float> centerPos)
{
    this->centerPos = centerPos;
}

void GlyphGenerator::setSizeFactor (float sizeFactor)
{
    this->sizeFactor = sizeFactor;
}

//float GlyphGenerator::freqFromYPos (float yPos)
//{
//    yPos *= sizeFactor;
//    yPos += centerPos.y;
//    float normalized = (yPos + 1.0f) / 2.0f;
//    
//    float logMinFreq = std::log(minFreq);
//    float logMaxFreq = std::log(maxFreq);
//    float logFreq = logMinFreq + normalized * (logMaxFreq - logMinFreq);
//    
//    return std::exp(logFreq);
//}

float GlyphGenerator::freqFromYPos (float yPos)
{
    yPos *= sizeFactor;
    yPos += centerPos.y;
    float normalized = (yPos + 1.0f) / 2.0f;
    
    // Convert minFreq and maxFreq to Bark scale
    float barkMinFreq = 13.0f * std::atan(0.00076f * minFreq) + 3.5f * std::atan(std::pow(minFreq / 7500.0f, 2));
    float barkMaxFreq = 13.0f * std::atan(0.00076f * maxFreq) + 3.5f * std::atan(std::pow(maxFreq / 7500.0f, 2));
    
    // Interpolate in the Bark scale
    float barkFreq = barkMinFreq + normalized * (barkMaxFreq - barkMinFreq);
    
    // Convert back from Bark to frequency
    float freq = 650.0f * std::sinh(barkFreq / 7.0f);
    
    return freq;
}

float GlyphGenerator::panFromXPos (float xPos)
{
    xPos *= sizeFactor;
    xPos += centerPos.x;
    return xPos;
}

float GlyphGenerator::getCurrPlayingTime()
{
    return currTime;
}
