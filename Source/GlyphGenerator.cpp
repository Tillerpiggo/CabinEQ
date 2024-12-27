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
//    noiseGenerator.setBandwidth (bandwidth);
    gainEnvelope.setEndEarlyInSamples (noteLenInSamples - 4000);
}

std::pair<float, float> GlyphGenerator::getNextSample()
{
    if (! glyph.has_value())
        return { 0.0f, 0.0f };
    
    // to do animated glyphs
    currTime += timeInterval * speedFactor;
    if (currTime >= 1.0f)
        currTime -= 1.0f;
    
    if (updateBandpassCounter > 1000)
    {
        auto [pos, progress] = glyph->positionAtTime (currTime);
        auto [xPos, yPos, _] = pos;
        noiseGenerator.setBandpass (freqFromYPos (yPos));
        noiseGenerator.setPan (panFromXPos (xPos));
        updateBandpassCounter = 0;
    }
    updateBandpassCounter++;
    return noiseGenerator.getNextSample();
    
    // to just play the vertices
//    std::pair<float, float> nextSample { 0.0f, 0.0f };
//    for (int i = 0; i < noiseGenerators.size(); ++i)
//    {
//        auto noiseSample = noiseGenerators[i].getNextSample();
//        float sampleOffset = noteLenInSamples * i;
//        float gain = gainEnvelope.gainAtSample ((static_cast<int> (sampleCount) + static_cast<int> (sampleOffset)) % (noteLenInSamples * numVertices), noteLenInSamples * numVertices);
//        nextSample.first += noiseSample.first * gain;
//        nextSample.second += noiseSample.second * gain;
//    }
//    
//    sampleCount += speedFactor;
//    if (sampleCount > noteLenInSamples * numVertices)
//    {
//        sampleCount = 0;
//    }
    
//    // Apply gain envelope based on progress
//    float envelope = 1.0f;
//    float len = 0.02;
//    if (progress < len)
//        envelope = progress / len;
//    if (progress > (1.0f - len))
//        envelope = (1.0f - progress) / len;
//    
//    nextSample.first *= envelope;
//    nextSample.second *= envelope;
    
//    return nextSample;
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
    auto vertices = glyph.getVertices();
    numVertices = (int) vertices.size();
    gainEnvelope.setEndEarlyInSamples (noteLenInSamples * numVertices - 2000 * numVertices);
    
    // Add needed genertors
    int numToAdd = static_cast<int> (vertices.size()) - static_cast<int> (noiseGenerators.size());
    for (int i = 0; i < numToAdd; ++i)
    {
        noiseGenerators.push_back (NoiseGenerator());
        noiseGenerators[noiseGenerators.size() - 1].prepare (spec);
    }
    
    updateNoiseGenerators();
}

void GlyphGenerator::updateNoiseGenerators()
{
    if (! glyph.has_value())
    {
        std::cerr << "Trying to update noise generators with nil glyph" << std::endl;
        return;
    }
    
    auto vertices = glyph->getVertices();
    // Set all noise generator positions
    for (int i = 0; i < noiseGenerators.size(); ++i)
    {
        if (i < vertices.size())
        {
            noiseGenerators[i].setBandwidth (bandwidth);
            noiseGenerators[i].setBandpass (fmin (freqFromYPos (vertices[i].y), 20000.0f));
            noiseGenerators[i].setPan (panFromXPos (vertices[i].x));
        }
        else
        {
            noiseGenerators[i].mute();
        }
    }
}

void GlyphGenerator::setSpeedFactor (float speedFactor)
{
    this->speedFactor = speedFactor;
}

void GlyphGenerator::setBandwidth (float bandwidth)
{
    this->bandwidth = bandwidth;
    updateNoiseGenerators();
}

void GlyphGenerator::setFrequencyRange (float minFreq, float maxFreq)
{
    this->minFreq = minFreq;
    this->maxFreq = maxFreq;
    updateNoiseGenerators();
}

void GlyphGenerator::setPanRange (float leftmostPan, float rightmostPan)
{
    this->leftmostPan = leftmostPan;
    this->rightmostPan = rightmostPan;
    updateNoiseGenerators();
}

void GlyphGenerator::setCenterPos (juce::Point<float> centerPos)
{
    this->centerPos = centerPos;
    updateNoiseGenerators();
}

void GlyphGenerator::setSizeFactor (float sizeFactor)
{
    this->sizeFactor = sizeFactor;
    updateNoiseGenerators();
}

float GlyphGenerator::freqFromYPos (float yPos)
{
    yPos *= sizeFactor;
    yPos += centerPos.y;
    float normalized = (yPos + 1.0f) / 2.0f;
    
    float logMinFreq = std::log(minFreq);
    float logMaxFreq = std::log(maxFreq);
    float logFreq = logMinFreq + normalized * (logMaxFreq - logMinFreq);
    
    return std::exp(logFreq);
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
