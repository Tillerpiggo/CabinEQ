/*
  ==============================================================================

    CheckerboardPlayer.h
    Created: 10 Feb 2025 3:18:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SquareGenerator.h"
#include "Checkerboard.h"

class CheckerboardPlayer
{
public:
    CheckerboardPlayer();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setCheckerboard (Checkerboard checkerboard);
    void setSoloSquareCoords (std::set<std::pair<int, int>> soloSquareCoords); // soloSquareCoords are [panIdx, freqIdx] - sets the coords of the currently "solod" square so that it only plays that square while muting everything else. If set to null, stops soloing the square
    
private:
    void updateNoiseGeneratorsIfNeeded(); // completely recaulcates noise generators if shouldUpdateNoiseGenerators = true
    
    juce::dsp::ProcessSpec spec;
    Checkerboard checkerboard;
    std::vector<SquareGenerator> noiseGenerators;
    
    std::set<std::pair<int, int>> soloSquareCoords;
    
    bool shouldUpdateNoiseGenerators = true;
    
};
