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
        
        nextSample.first += noiseSample.first * factor;
        nextSample.second += noiseSample.second * factor;
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
    std::cout << "minFreq set to " << minFreq << std::endl;
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
    
    int numNoiseSources = 0;
    for (const auto& glyph : glyphs)
    {
        numNoiseSources += glyph.getNumNoiseSources();
    }
    int numGenerators = (int) noiseGenerators.size();
    int numToAdd = numNoiseSources - numGenerators;
    
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
    int noiseGenIdx = 0;
    for (int i = 0; i < glyphs.size(); ++i)
    {
        // Set the appropriate bandpass filter for each noise generator, and implement helper function
//        auto [freqPan, vol] = getFreqPanVolFromGlyphAtTime (glyphs[i], currTime);
        auto noisePoints = getFreqPanVolsFromGlyphAtTime (glyphs[i], currTime);
//        auto [freq, pan] = freqPan;
        for (const auto& noisePoint : noisePoints)
        {
            auto [freqPan, vol] = noisePoint;
            auto [freq, pan] = freqPan;
            noiseGenerators[noiseGenIdx].setBandwidth (bandwidth);
            noiseGenerators[noiseGenIdx].setBandpass (freq);
            noiseGenerators[noiseGenIdx].setPan (pan);
            noiseGenerators[noiseGenIdx].setVolumeGain (vol);
            
            noiseGenIdx++;
        }
    }
}

float GlyphGridPlayer::scaleToBarkBandwidth (float bandwidth, float centerFrequency)
{
    centerFrequency = fmax (centerFrequency, minFreq);
    float erb = 24.7f * (4.37f * centerFrequency / 1000.0f + 1.0f);
    float erbInOctaves = log2 (1.0f + erb / centerFrequency);
    
    if (erbScalingEnabled)
        return bandwidth * erbInOctaves * 2.5f;
    else
        return bandwidth;
//    return bandwidth;// * erbInOctaves * 2.5f;
//    float lowerFrequency = centerFrequency / pow (2.0f, bandwidth);
//    float upperFrequency = centerFrequency * pow (2.0f, bandwidth);
//    float lowerBarkFrequency = barkToHz
//    float lowerFrequency = centerFrequency - criticalBandwidth / 2.0f;
//    float upperFrequency = centerFrequency + criticalBandwidth / 2.0f;
//    float octaves = log2 (upperFrequency / lowerFrequency);
//    return bandwidth * octaves;
}

float GlyphGridPlayer::barkToHz (float bark)
{
    return 600.0f * sinh(bark / 6.0);
}

void GlyphGridPlayer::setBarkScaling (bool barkScalingEnabled)
{
    this->barkScalingEnabled = barkScalingEnabled;
}

void GlyphGridPlayer::setERBScaling (bool erbScalingEnbaled)
{
    this->erbScalingEnabled = erbScalingEnbaled;
}

void GlyphGridPlayer::setIsCascading (bool isCascading)
{
    for (auto& glyph : glyphs)
        glyph.setIsCascading (isCascading);
    shouldAddRemoveNoiseGenerators = true;
    shouldUpdateNoiseGenerators = true;
}

void GlyphGridPlayer::setDensity (int density)
{
    for (auto& glyph : glyphs)
        glyph.setDensity (density);
    shouldAddRemoveNoiseGenerators = true;
    shouldUpdateNoiseGenerators = true;
}

void GlyphGridPlayer::setStrokeOverlap (float strokeOverlap)
{
    for (auto& glyph : glyphs)
        glyph.setStrokeOverlap (strokeOverlap);
    shouldAddRemoveNoiseGenerators = true;
    shouldUpdateNoiseGenerators = true;
}

void GlyphGridPlayer::setDotOverlap (float dotOverlap)
{
    for (auto& glyph : glyphs)
        glyph.setDotOverlap (dotOverlap);
    shouldAddRemoveNoiseGenerators = true;
    shouldUpdateNoiseGenerators = true;
}

void GlyphGridPlayer::setRampLength (float rampLength)
{
    for (auto& glyph : glyphs)
        glyph.setRampLength (rampLength);
    shouldAddRemoveNoiseGenerators = true;
    shouldUpdateNoiseGenerators = true;
}

////std::pair<std::pair<float, float>, float> GlyphGridPlayer::getFreqPanVolFromGlyphAtTime (const Glyph& glyph, float time)
////{
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
////}

std::vector<std::pair<std::pair<float, float>, float>> GlyphGridPlayer::getFreqPanVolsFromGlyphAtTime (const Glyph& glyph, float time)
{
    std::vector<std::pair<std::pair<float, float>, float>> freqPanVols;
    auto noisePoints = glyph.positionsAtTime (time);

    // Calculate coords
    auto [horizontalSizeFactor, verticalSizeFactor] = glyph.getSizeFactor();
    
    for (const auto& noisePoint : noisePoints)
    {
        float x = noisePoint.x * horizontalSizeFactor + glyph.getCenterPos().x;
        float y = noisePoint.y * verticalSizeFactor + glyph.getCenterPos().y;
        
        // Calculate freq
        float freq = y;
        float normalizedFreq = (freq + 1.0f) / 2.0f;
        float logMinFreq = std::log (minFreq);
        float logMaxFreq = std::log (maxFreq);
        float logFreq = logMinFreq + normalizedFreq * (logMaxFreq - logMinFreq);
        freq = std::exp (logFreq);
        
        // Calculate pan
        float pan = x;
        
        // Calculate vol
//        float vol = 1.0f;
        float vol = noisePoint.vol;
        
        freqPanVols.push_back ({{ freq, pan }, vol });
    }
    
    return freqPanVols;
    
}

std::pair<std::pair<float, float>, float> GlyphGridPlayer::getFreqPanVolFromGlyphAtTime (const Glyph& glyph, float time)
{
    // Calculate coords
    auto normalizedCoords = glyph.positionAtTime (time);
    auto [horizontalSizeFactor, verticalSizeFactor] = glyph.getSizeFactor();
    float x = normalizedCoords.x * horizontalSizeFactor + glyph.getCenterPos().x;
    float y = normalizedCoords.y * verticalSizeFactor + glyph.getCenterPos().y;
    
    // Calculate pan
    float pan = x;
    
    // Calculate freq using Bark scaling
    float freq = y; // Use y-coordinate for frequency
    float normalizedFreq = (freq + 1.0f) / 2.0f; // Normalize y to [0, 1]

    // Map normalized frequency to Bark scale (0 to 24 Barks)
    float barkMin = 1.0f; // Minimum Bark value (~60hz)
    float barkMax = 24.0f; // Maximum Bark value
    float barkFreq = barkMin + normalizedFreq * (barkMax - barkMin); // Map to Bark scale

    // Convert Bark to Hz (inverse mapping of Bark scale)
    float hzFreq = barkToHz (barkFreq);/*600.0f * (std::exp(barkFreq / 6.0f) - std::exp(-barkFreq / 6.0f));*/
    
    float logMinFreq = std::log (minFreq);
    float logMaxFreq = std::log (maxFreq);
    float logFreq = logMinFreq + normalizedFreq * (logMaxFreq - logMinFreq);
    freq = std::exp (logFreq);
    
    // Calculate vol
    float vol = normalizedCoords.vol;

    if (barkScalingEnabled)
        return {{ hzFreq, pan }, vol };
    else
        return {{ freq, pan }, vol };
}

std::vector<float> GlyphGridPlayer::getCurrPlayingFreqs()
{
    std::vector<float> playingFreqs;
    for (const auto& glyph : glyphs)
    {
        auto freqPanVols = getFreqPanVolsFromGlyphAtTime (glyph, currTime);
        for (const auto& freqPanVol : freqPanVols)
            playingFreqs.push_back (freqPanVol.first.first);
    }
    return playingFreqs;
}

std::vector<std::pair<float, float>> GlyphGridPlayer::getCurrPlayingFreqsAndVols()
{
    std::vector<std::pair<float, float>> playingFreqsAndVols;
    for (const auto& glyph : glyphs)
    {
        auto freqPanVols = getFreqPanVolsFromGlyphAtTime (glyph, currTime);
        for (const auto& freqPanVol : freqPanVols)
            playingFreqsAndVols.push_back ({ freqPanVol.first.first, freqPanVol.second });
    }
    return playingFreqsAndVols;
}

float GlyphGridPlayer::volToDB (float vol)
{
//    return (1.0f - vol) * -20.0f;
}
