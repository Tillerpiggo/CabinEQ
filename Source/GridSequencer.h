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
#include "TimeGainEnvelope.h"

// Allows the playing of a noise sequence grid. Keeps track of the current playing indices for each sequence, and handles iterating forward with time
class GridSequencer
{
public:
    GridSequencer();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    void setNoiseGrid (NoiseSequenceGrid noiseSequenceGrid);
    void setMinFreq (float newMinFreq);
    void setMaxFreq (float newMaxFreq);
    void setBandwidth (float bandwidth);
    void setSpeedFactor (float speedFactor);
    
    float getCurrTime();
    std::vector<float> getCurrPlayingFreqs();
    
private:
    void updateNoiseGenerators();
    std::pair<float, float> getFreqAndPanForNormalizedCoords (std::pair<float, float> normalizedCoords);
    
    std::optional<NoiseSequenceGrid> grid;
    juce::dsp::ProcessSpec spec;
    
    std::vector<NoiseGenerator> noiseGenerators;
    std::vector<TimeGainEnvelope> gainEnvelopes;
    
    float currTime = 0.0f; // time from 0 to 1
    float timeInterval = 0.0f; // must be set in prepare
    float speedFactor = 1.0f;
    
    // Constants
    float minFreq = 50.0f;
    float maxFreq = 12000.0f;
    float leftmostPan = -1.0f;
    float rightmostPan = 1.0f;
    float bandwidth = 1.0f;
    float envelopeDuration = 0.01f;
    
    bool shouldAddRemoveNoiseGenerators;
    bool shouldUpdateNoiseGenerators;
    int bandwidthCounter = 1;
    int timeCounter = 1;
};
