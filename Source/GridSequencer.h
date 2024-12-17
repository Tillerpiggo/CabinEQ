/*
  ==============================================================================

    GridSequencer.h
    Created: 16 Dec 2024 7:40:08pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "NoiseSequenceGrid.h"
#include "NoiseGenerator.h"
#include "GainEnvelope.h"

// Allows the playing of a noise sequence grid. Keeps track of the current playing indices for each sequence, and handles iterating forward with time
class GridSequencer
{
public:
    GridSequencer();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setNoiseGrid (NoiseSequenceGrid noiseSequenceGrid);
    void updateNoiseGenerators();
    
private:
    std::optional<NoiseSequenceGrid> grid;
    
    std::vector<NoiseGenerator> noiseGenerators;
    std::vector<GainEnvelope> gainEnvelopes;
    
    float currTime = 0.0f; // time from 0 to 1
    float timeInterval = 0.0f; // must be set in prepare
    
    // Constants
    float minFreq = 50.0f;
    float maxFreq = 12000.0f;
    float leftmostPan = -1.0f;
    float rightmostPan = 1.0f;
};
