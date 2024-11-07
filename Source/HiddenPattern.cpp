/*
  ==============================================================================

    HiddenPattern.cpp
    Created: 7 Nov 2024 11:09:23am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "HiddenPattern.h"

HiddenPattern::HiddenPattern (std::vector<std::pair<float, float>> hiddenPath, std::vector<std::pair<float, float>> confoundingCoords, float cycleLengthInSeconds)
    : hiddenPath (hiddenPath), confoundingCoords (confoundingCoords), cycleLengthInSeconds (cycleLengthInSeconds)
{}

SweepPattern HiddenPattern::getHiddenSweepPattern (float sampleRate)
{
    std::vector<std::pair<float, float>> hiddenPathFreqsAndPans;
    for (const auto& point : hiddenPath)
    {
        hiddenPathFreqsAndPans.push_back (frequencyAndPanForCoords (point));
    }
    return SweepPattern (hiddenPathFreqsAndPans, cycleLengthInSeconds, sampleRate);
}

std::vector<SweepPattern> HiddenPattern::getConfoundingSweepPatterns (float sampleRate)
{
    std::vector<SweepPattern> confoundingSweepPatterns;
    for (int i = 0; i < confoundingCoords.size(); ++i)
    {
        auto coords = frequencyAndPanForCoords (confoundingCoords[i]);
        confoundingSweepPatterns.push_back (SweepPattern ({ coords, coords }, cycleLengthInSeconds, sampleRate));
    }
    
    return confoundingSweepPatterns;
}

std::pair<float, float> HiddenPattern::frequencyAndPanForCoords (std::pair<float, float> coords)
{
    auto [x, y] = coords;
    
    float midpoint = MAX_FREQ / MIN_FREQ;
    float freq = 20.0f * std::pow (midpoint, (y + 1.0f) / 2.0f );
    float pan = x;

    return { freq, pan };
}
