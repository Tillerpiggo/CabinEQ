/*
  ==============================================================================

    GlyphGridPlayer.cpp
    Created: 23 Dec 2024 3:58:50pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "GlyphGridPlayer.h"

GlyphGridPlayer::GlyphGridPlayer()
{
    
}

std::pair<float, float> GlyphGridPlayer::getNextSample()
{
    // Update if needed
    updateBandpassCounter++;
    if (updateBandpassCounter > samplesUntilBandpassUpdate)
    {
        addRemoveNoiseGeneratorsIfNeeded();
        updateNoiseGeneratorsIfNeeded();
        updateBandpassCounter = 0;
    }
    
    // Calculate sample value
    std::pair<float, float> nextSample { 0.0f, 0.0f };
    for (int i = 0; i < noiseGenerators.size(); ++i)
    {
        auto noiseSample = noiseGenerators[i].getNextSample();
        float factor = 1.0f;
        nextSample.first += noiseSample.first * factor;// / static_cast<float> (i + 1);
        nextSample.second += noiseSample.second * factor;// / static_cast<float> (i + 1);
    }
    
    // Increment time
    currTime += timeInterval * speedFactor;
    if (currTime >= 1.0f)
        currTime -= 1.0f;
    
    return nextSample;
}

void GlyphGridPlayer::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    this->timeInterval = 1.0f / (spec.sampleRate * 2.0f); // make time interval 2 seconds
}

void GlyphGridPlayer::setGlyphs (std::vector<Glyph> glyphs)
{
    this->glyphs = glyphs;
    shouldAddRemoveNoiseGenerators = true;
    shouldUpdateNoiseGenerators = true;
}

void GlyphGridPlayer::setSpeedFactor (float speedFactor)
{
    this->speedFactor = speedFactor;
}

void GlyphGridPlayer::setBandwidth (float bandwidth)
{
    this->bandwidth = bandwidth;
    shouldUpdateNoiseGenerators = true;
}

void GlyphGridPlayer::setFrequencyRange (float minFreq, float maxFreq)
{
    this->minFreq = minFreq;
    this->maxFreq = maxFreq;
    shouldUpdateNoiseGenerators = true;
}

void GlyphGridPlayer::setMinFreq (float minFreq)
{
    this->minFreq = minFreq;
    shouldUpdateNoiseGenerators = true;
}

void GlyphGridPlayer::setMaxFreq (float maxFreq)
{
    this->maxFreq = maxFreq;
    shouldUpdateNoiseGenerators = true;
}

void GlyphGridPlayer::setPanRange (float leftmostPan, float rightmostPan)
{
    this->leftmostPan = leftmostPan;
    this->rightmostPan = rightmostPan;
    shouldUpdateNoiseGenerators = true;
}

float GlyphGridPlayer::getCurrPlayingTime()
{
    return currTime;
}

void GlyphGridPlayer::addRemoveNoiseGeneratorsIfNeeded()
{
    if (! shouldAddRemoveNoiseGenerators)
        return;
    
    int numGlyphs = (int) glyphs.size();
    int numGenerators = (int) noiseGenerators.size();
    int numToAdd = numGlyphs - numGenerators;
    
    // Add needed generators
    for (int i = 0; i < numToAdd; ++i)
    {
        noiseGenerators.push_back (NoiseGenerator());
        noiseGenerators[i + numGenerators].prepare (spec); // prepare the generator we just added
    }
    
    // Remove unneeded generators
    if (numToAdd < 0)
    {
        noiseGenerators.erase (noiseGenerators.end() + numToAdd, noiseGenerators.end());
    }
}

void GlyphGridPlayer::updateNoiseGeneratorsIfNeeded()
{
    if (! shouldUpdateNoiseGenerators)
        return;
    
    // Update the bandpass of all noise generators
    for (int i = 0; i < glyphs.size(); ++i)
    {
        // Set the appropriate bandpass filter for each noise generator, and implement helper function
        auto [freqPan, vol] = getFreqPanVolFromGlyphAtTime (glyphs[i], currTime);
        auto [freq, pan] = freqPan;
        noiseGenerators[i].setBandwidth (scaleToBarkBandwidth (bandwidth, freq));
        noiseGenerators[i].setBandpass (freq);
        noiseGenerators[i].setPan (pan);
        noiseGenerators[i].setVolumeDB (volToDB (glyphs[i].getVolume()) + volToDB (vol));
    }
}

float GlyphGridPlayer::scaleToBarkBandwidth (float bandwidth, float centerFrequency)
{
    float criticalBandwidth = 25.0 + 75.0 * pow(1.0 + 1.4 * pow (centerFrequency / 1000.0, 2.0), 0.69);
    float actualBandwidth = centerFrequency * (pow (2.0, 1.0 / 2.0) - pow (2.0, -1.0 / 2.0));
    std::cout << "ratio: " << criticalBandwidth / actualBandwidth << std::endl;
    return bandwidth * fmin (criticalBandwidth / actualBandwidth, 1.0f);
}

float GlyphGridPlayer::barkToHz (float hz)
{
    return 600.0f * sinh(hz / 6.0);
}

//std::pair<std::pair<float, float>, float> GlyphGridPlayer::getFreqPanVolFromGlyphAtTime (const Glyph& glyph, float time)
//{
//    // Calculate coords
//    auto normalizedCoords = glyph.positionAtTime (time).first;
//    float x = normalizedCoords.x * glyph.getSizeFactor() + glyph.getCenterPos().x;
//    float y = normalizedCoords.y * glyph.getSizeFactor() + glyph.getCenterPos().y;
//    
//    // Calculate pan
//    float pan = x;
//    
//    // Calculate freq
//    float freq = y;
//    float normalizedFreq = (freq + 1.0f) / 2.0f;
//    
//    float logMinFreq = std::log (minFreq);
//    float logMaxFreq = std::log (maxFreq);
//    float logFreq = logMinFreq + normalizedFreq * (logMaxFreq - logMinFreq);
//    freq = std::exp (logFreq);
//    
//    // Calculate vol
//    float vol = normalizedCoords.vol;
//    
//    return {{ freq, pan }, vol };
//}

std::pair<std::pair<float, float>, float> GlyphGridPlayer::getFreqPanVolFromGlyphAtTime (const Glyph& glyph, float time)
{
    // Calculate coords
    auto normalizedCoords = glyph.positionAtTime (time).first;
    float x = normalizedCoords.x * glyph.getSizeFactor() + glyph.getCenterPos().x;
    float y = normalizedCoords.y * glyph.getSizeFactor() + glyph.getCenterPos().y;
    
    // Calculate pan
    float pan = x;
    
    // Calculate freq using Bark scaling
    float freq = y; // Use y-coordinate for frequency
    float normalizedFreq = (freq + 1.0f) / 2.0f; // Normalize y to [0, 1]

    // Map normalized frequency to Bark scale (0 to 24 Barks)
    float barkMin = 0.0f; // Minimum Bark value
    float barkMax = 24.0f; // Maximum Bark value
    float barkFreq = barkMin + normalizedFreq * (barkMax - barkMin); // Map to Bark scale

    // Convert Bark to Hz (inverse mapping of Bark scale)
    float hzFreq = barkToHz (barkFreq);/*600.0f * (std::exp(barkFreq / 6.0f) - std::exp(-barkFreq / 6.0f));*/

    // Calculate vol
    float vol = normalizedCoords.vol;

    return {{ hzFreq, pan }, vol };
}

std::vector<float> GlyphGridPlayer::getCurrPlayingFreqs()
{
    std::vector<float> playingFreqs;
    for (const auto& glyph : glyphs)
    {
        playingFreqs.push_back (getFreqPanVolFromGlyphAtTime (glyph, currTime).first.first);
    }
    return playingFreqs;
}

float GlyphGridPlayer::volToDB (float vol)
{
    return (1.0f - vol) * -20.0f;
}
//
//std::pair<float, float> GlyphGridPlayer::getFreqAndPanFromNormalizedCoords (juce::Point<float> coords)
//{
//    // Calculate pan
//    float pan = coords.x;
//    
//    // Calculate freq
//    float freq = coords.y; // flip upside down because y is weird
//    float normalizedFreq = (freq + 1.0f) / 2.0f;
//    
//    float logMinFreq = std::log (minFreq);
//    float logMaxFreq = std::log (maxFreq);
//    float logFreq = logMinFreq + normalizedFreq * (logMaxFreq - logMinFreq);
//    freq = std::exp (logFreq);
//    
//    return { freq, pan };
//}
