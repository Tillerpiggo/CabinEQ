/*
  ==============================================================================

    FauxMusicPattern.h
    Created: 6 Nov 2024 1:51:20am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Pattern.h"

// This represents a full distribution of patterns of noise that should, ideally, fill up the spectrum. For now it's just a collection of hit patterns and 
class FauxMusicPattern
{
public:
    FauxMusicPattern (std::vector<Pattern> patterns, std::vector<std::pair<float, float>> freqRanges); // len(patterns) == len(freqRanges)
    
    FauxMusicPattern (std::vector<std::vector<bool>> hitPatterns, std::vector<std::pair<float, float>> freqRanges);
    
    const std::vector<Pattern>& getPatterns() const;
    const std::vector<std::pair<float, float>>& getFreqRanges() const;
    const int getNumPatterns() const;
private:
    std::vector<Pattern> patterns;
    std::vector<std::pair<float, float>> freqRanges;
};
