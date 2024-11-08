/*
  ==============================================================================

    HiddenPattern.h
    Created: 7 Nov 2024 11:09:23am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SweepPattern.h"

// This represents a hidden pattern - i.e. a pattern sweeping through space, with other spatial noise blocking the pattern. For now, the confounding spatial noise is static and this only defines the path, letting the playback manage bandwidth/etc.
class HiddenPattern
{
public:
    HiddenPattern (std::vector<std::pair<float, float>> hiddenPath, std::vector<std::pair<float, float>> confoundingCoords, float hiddenBandwidth = 1.0f, float confoundingBandwidth = 2.0f, float cycleLengthInSeconds = 3.0f);
    
    SweepPattern getHiddenSweepPattern (float sampleRate);
    std::vector<SweepPattern> getConfoundingSweepPatterns (float sampleRate);
    
    float getHiddenBandwidth();
    float getConfoundingBandwidth();
    
private:
    std::pair<float, float> frequencyAndPanForCoords (std::pair<float, float>); // transforms a coordinate pair (with y in [-1, 1]) to frequency
    float MIN_FREQ = 20.0f;
    float MAX_FREQ = 15000.0f;
    
    std::vector<std::pair<float, float>> hiddenPath; // coordinate pairs defining the path of the hidden pattern
    std::vector<std::pair<float, float>> confoundingCoords; // coordinate pairs for confounding noise
    
    float cycleLengthInSeconds; // defines length of cycle in seconds for the entire hidden path
    float hiddenBandwidth;
    float confoundingBandwidth;
};
