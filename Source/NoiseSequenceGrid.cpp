/*
  ==============================================================================

    NoiseSequenceGrid.cpp
    Created: 14 Dec 2024 11:31:05am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "NoiseSequenceGrid.h"

NoiseSequenceGrid::NoiseSequenceGrid (int numRows, int numCols)
    : numRows (numRows), numCols (numCols)
{
    
}

void NoiseSequenceGrid::addSequence (NoiseSequence noiseSequence)
{
    
}

void NoiseSequenceGrid::removeSequence (std::pair<int, int> origin)
{
    for (int i = 0; i < noiseSequences.size(); ++i)
    {
        if (noiseSequences[i].hasOriginAt (origin))
        {
            noiseSequences.erase (noiseSequences.begin() + i);
            break;
        }
    }
}

void NoiseSequenceGrid::moveSequence (std::pair<int, int> origin, std::pair<int, int> newOrigin)
{
    // Assume there is a sequence at origin, and that the sequence can be moved to newOrigin
    
    // Find the sequence
    for (int i = 0; i < noiseSequences.size(); ++i)
    {
        if (noiseSequences[i].hasOriginAt (origin))
        {
            noiseSequences[i].setOrigin (origin);
        }
    }
}

void NoiseSequenceGrid::toggleCoords (std::pair<int, int> coords)
{
    for (auto& noiseSequence : noiseSequences)
        if (noiseSequence.toggleCoords (coords))
            return;
}

// Increase/reduce horizontal/vertical dimensions by adding rows/cols in between existing rows
// so 3 -> 5 -> 9 when scaling up twice
void NoiseSequenceGrid::scaleUpHorizontal();
void NoiseSequenceGrid::scaleDownHorizontal();
void NoiseSequenceGrid::scaleUpVertical();
void NoiseSequenceGrid::scaleDownVertical();
