/*
  ==============================================================================

    SliderSetPointManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SetPointManager.h"

SetPointManager::SetPointManager()
{
    // Generate NUM_PTS evenly spaced frequencies from 20 to 15000khz
    frequencies.reserve(NUM_PTS / 3);
    float startFreq = 50.0f;
    float endFreq = 950.0f;

    float logStart = std::log10(startFreq);
    float logEnd = std::log10(endFreq);
    float step = (logEnd - logStart) / (NUM_PTS / 3 - 1);

    for (int i = 0; i < NUM_PTS / 3; ++i) {
        double logFreq = logStart + i * step;
        frequencies.push_back(std::pow(10, logFreq));
    }
    
    frequencies.reserve(NUM_PTS / 3);
    startFreq = 1050.0f;
    endFreq = 7500.0f;

    logStart = std::log10(startFreq);
    logEnd = std::log10(endFreq);
    step = (logEnd - logStart) / (NUM_PTS / 3 - 1);

    for (int i = 0; i < NUM_PTS / 3; ++i) {
        double logFreq = logStart + i * step;
        frequencies.push_back(std::pow(10, logFreq));
    }
    
    frequencies.reserve(NUM_PTS / 3);
    startFreq = 8000.0f;
    endFreq = 18000.0f;

    logStart = std::log10(startFreq);
    logEnd = std::log10(endFreq);
    step = (logEnd - logStart) / (NUM_PTS / 3 - 1);

    for (int i = 0; i < NUM_PTS / 3; ++i) {
        double logFreq = logStart + i * step;
        frequencies.push_back(std::pow(10, logFreq));
    }
    
    for (size_t i = 0; i < frequencies.size(); ++i)
    {
        amplitudes.push_back(0.0f);
        pans.push_back(0.0f);
    }
}
