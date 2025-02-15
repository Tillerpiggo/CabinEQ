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

std::vector<float> CheckerboardPlayer::getCurrSolodFreqs()
{
    // Just return the solod freqs without thinking too hard about it
    std::set<float> solodFreqs;
//    for (i)
}

float CheckerboardPlayer::getBandwidth()
{
    // TODO: implement
    return 1.0f;
}

void CheckerboardPlayer::updateNoiseGeneratorsIfNeeded()
{
    if (! shouldUpdateNoiseGenerators)
        return;
    
    noiseGenerators.clear();
    
//    int resolution = checkerboard.getResolution();
    auto [numRows, numCols] = checkerboard.getGridDimensions();
//    float sharpness = checkerboard.getSharpness();
    
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
        for (int freqIdx = 0; freqIdx < numRows; ++freqIdx)
        {
            for (int panIdx = 0; panIdx < numCols; ++panIdx)
            {
                bool isSoloed = soloSquareCoords.empty() || (soloSquareCoords.find({ freqIdx, panIdx }) != soloSquareCoords.end());
                if (grid[freqIdx][panIdx] && isSoloed)
                {
                    noiseGenerators.push_back (SquareGenerator());
                    noiseGenerators[lastIdx].prepare (spec);
                    noiseGenerators[lastIdx].setGridDimensions (numRows, numCols);
                    noiseGenerators[lastIdx].setCheckerboardCoords (freqIdx, panIdx);
                    lastIdx++;
                }
            }
        }
//    }

    
    shouldUpdateNoiseGenerators = false;
}
