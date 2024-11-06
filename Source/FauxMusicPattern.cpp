/*
  ==============================================================================

    FauxMusicPattern.cpp
    Created: 6 Nov 2024 1:51:20am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "FauxMusicPattern.h"

FauxMusicPattern::FauxMusicPattern (std::vector<Pattern> patterns, std::vector<std::pair<float, float>> freqRanges)
    : patterns (patterns), freqRanges (freqRanges)
{
}


const std::vector<Pattern>& FauxMusicPattern::getPatterns() const
{
    return patterns;
}

const std::vector<std::pair<float, float>>& FauxMusicPattern::getFreqRanges() const
{
    return freqRanges;
}

const int FauxMusicPattern::getNumPatterns() const
{
    return static_cast<int> (patterns.size());
}
