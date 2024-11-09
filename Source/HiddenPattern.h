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
#include "MelodicNotes.h"

// This represents a hidden pattern - i.e. a pattern sweeping through space, with other spatial noise blocking the pattern. For now, the confounding spatial noise is static and this only defines the path, letting the playback manage bandwidth/etc.
class HiddenPattern
{
public:
    HiddenPattern (std::vector<bool> hits, 
                   std::vector<std::pair<float, float>> hiddenPath,
                   std::vector<std::pair<float, float>> confoundingCoords,
                   float hiddenBandwidth = 2.0f,
                   float confoundingBandwidth = 1.0f,
                   float cycleLengthInSeconds = 3.0f);
    
    HiddenPattern (std::vector<bool> hits,
                   std::vector<std::vector<int>> pattern2D,
                   float hiddenBandwidth = 2.0f,
                   float confoundingBandwidth = 1.0f,
                   float cycleLengthInSeconds = 3.0f);
    
    static HiddenPattern alongPath (std::vector<std::pair<float, float>> path, int numHits);
    static std::pair<float, float> GetPositionAlongPath(const std::vector<std::pair<float, float>>& path, float t);
    static std::pair<float, float> GetDirectionAlongPath(const std::vector<std::pair<float, float>>& path, float t);
    
    MelodicNotes getMelodicPattern();
    SweepPattern getHiddenSweepPattern (float sampleRate);
    std::vector<SweepPattern> getConfoundingSweepPatterns (float sampleRate);
    
    float getHiddenBandwidth();
    float getConfoundingBandwidth();
    
    static std::vector<std::pair<float, float>> GenerateHiddenPath(const std::vector<std::vector<int>>& pattern2D);
    static std::vector<std::pair<float, float>> GenerateConfoundingCoords(const std::vector<std::vector<int>>& pattern2D);
    
private:
    std::pair<float, float> frequencyAndPanForCoords (std::pair<float, float>); // transforms a coordinate pair (with y in [-1, 1]) to frequency
    float MIN_FREQ = 160.0f;
    float MAX_FREQ = 12000.0f;
    
    std::vector<bool> hits;
    std::vector<std::pair<float, float>> hiddenPath; // coordinate pairs defining the path of the hidden pattern
    std::vector<std::pair<float, float>> confoundingCoords; // coordinate pairs for confounding noise
    
    float cycleLengthInSeconds; // defines length of cycle in seconds for the entire hidden path
    float hiddenBandwidth;
    float confoundingBandwidth;
};
