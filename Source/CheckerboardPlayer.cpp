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

void CheckerboardPlayer::setSoloSquareCoords (std::set<std::pair<int, int>> soloSquareCoords)
{
    this->soloSquareCoords = soloSquareCoords;
    std::cout << "set solo square coords" << std::endl;
    shouldUpdateNoiseGenerators = true;
}

void CheckerboardPlayer::updateNoiseGeneratorsIfNeeded()
{
    if (! shouldUpdateNoiseGenerators)
        return;
    
    noiseGenerators.clear();
    
    int resolution = checkerboard.getResolution();
    float sharpness = checkerboard.getSharpness();
    
//    // Clear noise generators and add new ones
//    if (! soloSquareCoords.empty()) // if solo'd, just play the one square
//    {
//        int lastIdx = 0;
//        for (const auto& soloSquareCoordPair : soloSquareCoords)
//        {
//            noiseGenerators.push_back (SquareGenerator());
//            noiseGenerators[lastIdx].prepare (spec);
//            noiseGenerators[lastIdx].setResolution (resolution);
//            noiseGenerators[lastIdx].setSharpness (sharpness);
//            noiseGenerators[lastIdx].setCheckerboardCoords (soloSquareCoordPair.first, soloSquareCoordPair.second);
//            lastIdx++;
//        }
//    }
//    else 
//    {
        auto grid = checkerboard.getGrid();
        int lastIdx = 0;
        for (int freqIdx = 0; freqIdx < resolution; ++freqIdx)
        {
            for (int panIdx = 0; panIdx < resolution; ++panIdx)
            {
                bool isSoloed = soloSquareCoords.empty() || (soloSquareCoords.find({ panIdx, freqIdx }) != soloSquareCoords.end());
                if (grid[panIdx][freqIdx] && isSoloed)
                {
                    noiseGenerators.push_back (SquareGenerator());
                    noiseGenerators[lastIdx].prepare (spec);
                    noiseGenerators[lastIdx].setResolution (resolution);
                    noiseGenerators[lastIdx].setSharpness (sharpness);
                    noiseGenerators[lastIdx].setCheckerboardCoords (panIdx, freqIdx);
                    lastIdx++;
                }
            }
        }
//    }

    
    shouldUpdateNoiseGenerators = false;
}
