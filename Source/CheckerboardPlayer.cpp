/*
  ==============================================================================

    CheckerboardPlayer.cpp
    Created: 10 Feb 2025 3:18:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CheckerboardPlayer.h"

CheckerboardPlayer::CheckerboardPlayer()
{
    
}

std::pair<float, float> CheckerboardPlayer::getNextSample()
{
    updateNoiseGeneratorsIfNeeded();
    
    // Compute next sample
    float leftSample = 0.0f;
    float rightSample = 0.0f;
    for (int i = 0; i < noiseGenerators.size(); ++i)
    {
        auto noiseSample = noiseGenerators[i].getNextSample();
        leftSample += noiseSample.first;
        rightSample += noiseSample.second;
    }
    
    return { leftSample, rightSample };
}

void CheckerboardPlayer::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
}

void CheckerboardPlayer::setCheckerboard (Checkerboard checkerboard)
{
    this->checkerboard = checkerboard;
    shouldUpdateNoiseGenerators = true;
}

void CheckerboardPlayer::updateNoiseGeneratorsIfNeeded()
{
    if (! shouldUpdateNoiseGenerators)
        return;
    
    noiseGenerators.clear();
    
    // Clear noise generators and add new ones
    int numNoiseGenerators = checkerboard.getNumNoiseGenerators();
    int resolution = checkerboard.getResolution();
    for (int freqIdx = 0; freqIdx < resolution; ++freqIdx)
    {
        for (int panIdx = 0; panIdx < resolution; ++panIdx)
        {
            int lastIdx = freqIdx * resolution + panIdx;
            noiseGenerators.push_back (SquareGenerator());
            noiseGenerators[lastIdx].prepare (spec);
            noiseGenerators[lastIdx].setResolution (resolution);
            noiseGenerators[lastIdx].setCheckerboardCoords (panIdx, freqIdx);
            
        }
    }
    
    shouldUpdateNoiseGenerators = false;
}
