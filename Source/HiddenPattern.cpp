/*
  ==============================================================================

    HiddenPattern.cpp
    Created: 7 Nov 2024 11:09:23am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "HiddenPattern.h"

HiddenPattern::HiddenPattern (std::vector<bool> hits, std::vector<std::pair<float, float>> hiddenPath, std::vector<std::pair<float, float>> confoundingCoords, float hiddenBandwidth, float confoundingBandwidth, float cycleLengthInSeconds)
    : hits (hits), hiddenPath (hiddenPath), confoundingCoords (confoundingCoords), cycleLengthInSeconds (cycleLengthInSeconds), hiddenBandwidth (hiddenBandwidth), confoundingBandwidth (confoundingBandwidth)
{}

HiddenPattern::HiddenPattern(std::vector<bool> hits,
                             std::vector<std::vector<int>> pattern2D,
                             float hiddenBandwidth,
                             float confoundingBandwidth,
                             float cycleLengthInSeconds)
    : HiddenPattern(hits,
                    GenerateHiddenPath(pattern2D),
                    GenerateConfoundingCoords(pattern2D),
                    hiddenBandwidth,
                    confoundingBandwidth,
                    cycleLengthInSeconds) {}

// Helper functions to generate hiddenPath and confoundingCoords
std::vector<std::pair<float, float>> HiddenPattern::GenerateHiddenPath(const std::vector<std::vector<int>>& pattern2D)
{
    std::vector<std::pair<int, std::pair<float, float>>> orderedPoints;
    int numRows = pattern2D.size();
    int numCols = pattern2D.empty() ? 0 : pattern2D[0].size();

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {
            int value = pattern2D[r][c];
            if (value > 0) {
                float x = (numCols == 1) ? 0.0f : -1.0f + 2.0f * c / (numCols - 1);
                float y = (numRows == 1) ? 0.0f : -1.0f + 2.0f * r / (numRows - 1);
                orderedPoints.emplace_back(value, std::make_pair(x, y));
            }
        }
    }
    std::sort(orderedPoints.begin(), orderedPoints.end(),
              [](const auto& a, const auto& b){ return a.first < b.first; });

    std::vector<std::pair<float, float>> hiddenPath;
    for (const auto& p : orderedPoints) {
        hiddenPath.push_back(p.second);
    }
    return hiddenPath;
}

std::vector<std::pair<float, float>> HiddenPattern::GenerateConfoundingCoords(const std::vector<std::vector<int>>& pattern2D)
{
    std::vector<std::pair<float, float>> confoundingCoords;
    int numRows = pattern2D.size();
    int numCols = pattern2D.empty() ? 0 : pattern2D[0].size();

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {
            int value = pattern2D[r][c];
            if (value == 0) {
                float x = (numCols == 1) ? 0.0f : -1.0f + 2.0f * c / (numCols - 1);
                float y = (numRows == 1) ? 0.0f : -1.0f + 2.0f * r / (numRows - 1);
                confoundingCoords.emplace_back(x, y);
            }
        }
    }
    return confoundingCoords;
}

SweepPattern HiddenPattern::getHiddenSweepPattern (float sampleRate)
{
    std::vector<std::pair<float, float>> hiddenPathFreqsAndPans;
    for (const auto& point : hiddenPath)
    {
        hiddenPathFreqsAndPans.push_back (frequencyAndPanForCoords (point));
    }
    return SweepPattern (hiddenPathFreqsAndPans, cycleLengthInSeconds, sampleRate);
}

MelodicNotes HiddenPattern::getMelodicPattern()
{
    std::vector<float> freqs;
    std::vector<float> pans;
    for (const auto& point : hiddenPath)
    {
        auto [freq, pan] = frequencyAndPanForCoords (point);
        freqs.push_back (freq);
        pans.push_back (pan);
    }
    
    return MelodicNotes::withMelodicPattern (hits, freqs, hiddenBandwidth, pans).withNoteDurationInSeconds (0.1f);
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

float HiddenPattern::getHiddenBandwidth()
{
    return hiddenBandwidth;
}

float HiddenPattern::getConfoundingBandwidth()
{
    return confoundingBandwidth;
}

std::pair<float, float> HiddenPattern::frequencyAndPanForCoords (std::pair<float, float> coords)
{
    auto [x, y] = coords;
    
    float freq = MIN_FREQ * std::pow (MAX_FREQ / MIN_FREQ, (y + 1.0f) / 2.0f );
    float pan = x;

    return { freq, pan };
}
