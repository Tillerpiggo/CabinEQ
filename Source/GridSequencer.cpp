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
    
    // TODO
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
    
    // Add needed generators
    for (int i = 0; i < numToAdd; ++i)
    {
        noiseGenerators.push_back (NoiseGenerator());
        noiseGenerators[i + numGenerators].prepare (spec); // prepare the generator we just added
        gainEnvelopes.push_back (TimeGainEnvelope (0.05f, 0.05f));
    }
    
    // Remove unneeded generators
    if (numToAdd < 0)
    {
        noiseGenerators.erase (noiseGenerators.end() + numToAdd, noiseGenerators.end());
        gainEnvelopes.erase (gainEnvelopes.end() + numToAdd, gainEnvelopes.end());
    }
    
    updateNoiseGenerators();
}

void GridSequencer::updateNoiseGenerators()
{
    // Update the frequency/bandwidth of all noise generators (assumes # generators = # sequences in grid)
    auto playingCoords = grid->getNormalizedPlayingCoordsAtTime (currTime);
    for (int i = 0; i < playingCoords.size(); ++i)
    {
        // set the appropriate bandpass filter for each noise generator, and implement helper function getFreqAndPanForNormalizedCoords to help with this
        auto [freq, pan] = getFreqAndPanForNormalizedCoords (playingCoords[i]);
        noiseGenerators[i].setBandwidth (bandwidth);
        noiseGenerators[i].setBandpass (freq);
        noiseGenerators[i].setPan (pan);
    }
}

std::pair<float, float> GridSequencer::getFreqAndPanForNormalizedCoords (std::pair<float, float> normalizedCoords)
{
    auto [normalizedX, normalizedY] = normalizedCoords;
    
    // Calculate pan
    float pan = normalizedX;
    
    // Calculate freq
    float freq = normalizedY;
    float normalizedFreq = (freq + 1.0f) / 2.0f; // move into range [0, 1]
    
    float logMinFreq = std::log (minFreq);
    float logMaxFreq = std::log (maxFreq);
    float logFreq = logMinFreq + normalizedFreq * (logMaxFreq - logMinFreq);
    freq = std::exp (logFreq);
    
    return { freq, pan };
}
