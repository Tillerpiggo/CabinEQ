/*
  ==============================================================================

    GridSequencer.cpp
    Created: 16 Dec 2024 7:40:08pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "GridSequencer.h"

GridSequencer::GridSequencer()
{
    
}

std::pair<float, float> GridSequencer::getNextSample()
{
    if (! grid.has_value())
        return { 0.0f, 0.0f };
    
    if (shouldAddRemoveNoiseGenerators)
    {
        int numSequences = (int) grid->getNoiseSequences().size();
        int numGenerators = (int) noiseGenerators.size();
        int numToAdd = numSequences - numGenerators;
        
        // Add needed generators
        for (int i = 0; i < numToAdd; ++i)
        {
            noiseGenerators.push_back (NoiseGenerator());
            noiseGenerators[i + numGenerators].prepare (spec); // prepare the generator we just added
            gainEnvelopes.push_back (TimeGainEnvelope (envelopeDuration * speedFactor, envelopeDuration * speedFactor));
        }
        
        // Remove unneeded generators
        if (numToAdd < 0)
        {
            noiseGenerators.erase (noiseGenerators.end() + numToAdd, noiseGenerators.end());
            gainEnvelopes.erase (gainEnvelopes.end() + numToAdd, gainEnvelopes.end());
        }
    }
    
    if (shouldUpdateNoiseGenerators)
        updateNoiseGenerators();
    
    std::pair<float, float> nextSample { 0.0f, 0.0f };
    for (int i = 0; i < noiseGenerators.size(); ++i)
    {
        auto noiseSample = noiseGenerators[i].getNextSample();
        float gain = gainEnvelopes[i].gainAtTime (currTime, grid->getNoiseSequences()[i].getNoteDurationInTime());
        nextSample.first += noiseSample.first * gain;
        nextSample.second += noiseSample.second * gain;
    }
    
    currTime += timeInterval * speedFactor;
    if (currTime > 1.0f)
    {
        currTime -= 1.0f;
        isPerpendicular = ! isPerpendicular;
    }
    
//    // Repeat with 4 different bandwidths
//    currTime += timeInterval * speedFactor;
//    if (currTime >= (float) timeCounter * grid->getNoiseSequences()[0].getNoteDurationInTime() / speedFactor)
//    {
//        currTime -= 1.0f * grid->getNoiseSequences()[0].getNoteDurationInTime() / speedFactor;
//        setBandwidth (bandwidth * 1.5f);
//        bandwidthCounter++;
//    }
//    if (bandwidthCounter >= 4)
//    {
//        bandwidthCounter = 1;
//        timeCounter++;
//        setBandwidth (bandwidth / std::pow (1.5f, 3.0f));
//        currTime += 1.0f * grid->getNoiseSequences()[0].getNoteDurationInTime() / speedFactor;
//    }
//    if (currTime > 1.0f)
//    {
//        timeCounter = 1;
//        bandwidthCounter = 1;
//        currTime -= 1.0f;
//    }
    
    return nextSample;
}

void GridSequencer::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    timeInterval = 1.0f / (spec.sampleRate * 3.0f); // make time interval 3 seconds
}

void GridSequencer::setNoiseGrid (NoiseSequenceGrid noiseSequenceGrid)
{
    this->grid = noiseSequenceGrid;
    
    
    // Make sure there are the right # of noise generators
    int numSequences = (int) grid->getNoiseSequences().size();
    int numGenerators = (int) noiseGenerators.size();
    int numToAdd = numSequences - numGenerators;
    
    if (numToAdd != 0)
    {
        shouldAddRemoveNoiseGenerators = true;
    }
    shouldUpdateNoiseGenerators = true;
}

void GridSequencer::setMinFreq (float newMinFreq)
{
    this->minFreq = newMinFreq;
    shouldUpdateNoiseGenerators = true;
//    updateNoiseGenerators();
}

void GridSequencer::setMaxFreq (float newMaxFreq)
{
    this->maxFreq = newMaxFreq;
    shouldUpdateNoiseGenerators = true;
//    updateNoiseGenerators();
}

void GridSequencer::setBandwidth (float bandwidth)
{
    this->bandwidth = bandwidth;
    shouldUpdateNoiseGenerators = true;
}

void GridSequencer::setSpeedFactor (float speedFactor)
{
    this->speedFactor = speedFactor;
    shouldUpdateNoiseGenerators = true;
}

float GridSequencer::getCurrTime()
{
    return currTime;
}

std::vector<float> GridSequencer::getCurrPlayingFreqs()
{
    if (! grid.has_value())
        return {};
    
    
    auto playingCoords = grid->getNormalizedPlayingCoordsAtTime (currTime);
    std::vector<float> playingFreqs;
    for (int i = 0; i < playingCoords.size(); ++i)
        playingFreqs.push_back (getFreqAndPanForNormalizedCoords (playingCoords[i]).first);
    
    return playingFreqs;
}

void GridSequencer::updateNoiseGenerators()
{
//    bandwidth = 7.0f / grid->getNumRowsAndNumCols().first;
    
    // Update the frequency/bandwidth of all noise generators (assumes # generators = # sequences in grid)
    auto playingCoords = grid->getNormalizedPlayingCoordsAtTime (currTime);
    for (int i = 0; i < playingCoords.size(); ++i)
    {
        // set the appropriate bandpass filter for each noise generator, and implement helper function getFreqAndPanForNormalizedCoords to help with this
        auto [freq, pan] = getFreqAndPanForNormalizedCoords (playingCoords[i]);
        noiseGenerators[i].setBandwidth (bandwidth);
        noiseGenerators[i].setBandpass (freq);
        noiseGenerators[i].setPan (pan);
        gainEnvelopes[i].setStartDurationInSeconds (envelopeDuration * speedFactor);
        gainEnvelopes[i].setEndDurationInSeconds (envelopeDuration * speedFactor);
    }
}

std::pair<float, float> GridSequencer::getFreqAndPanForNormalizedCoords (std::pair<float, float> normalizedCoords)
{
    auto [normalizedX, normalizedY] = normalizedCoords;
    
//    if (isPerpendicular)
//        std::swap (normalizedX, normalizedY);
    
    // Calculate pan
    float pan = normalizedX;
    
    // Calculate freq
    float freq = -normalizedY; // flip upside down because this seems to work
    float normalizedFreq = (freq + 1.0f) / 2.0f; // move into range [0, 1]
    
    float logMinFreq = std::log (minFreq);
    float logMaxFreq = std::log (maxFreq);
    float logFreq = logMinFreq + normalizedFreq * (logMaxFreq - logMinFreq);
    freq = std::exp (logFreq);
    
    return { freq, pan };
}
