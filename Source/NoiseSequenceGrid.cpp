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
    noiseSequences.push_back (noiseSequence);
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

std::vector<NoiseSequence> NoiseSequenceGrid::getNoiseSequences()
{
    return noiseSequences;
}

std::pair<int, int> NoiseSequenceGrid::getNumRowsAndNumCols()
{
    return { numRows, numCols };
}

int NoiseSequenceGrid::getNextAvailableId()
{
    // Get a sorted list of ids
    std::vector<int> ids;
    for (const auto& noiseSequence : noiseSequences)
        ids.push_back (noiseSequence.getId());
    std::sort (ids.begin(), ids.end());
    
    // Find the next available id
    int nextId = 0;
    for (const int id : ids)
    {
        if (nextId == id)
            nextId++;
    }
    return nextId;
}

// Increase/reduce horizontal/vertical dimensions by adding rows/cols in between existing rows
// so 3 -> 5 -> 9 when scaling up twice
void NoiseSequenceGrid::scaleUpHorizontal()
{
    numCols += (numCols - 1);
    for (auto& noiseSequence : noiseSequences)
        noiseSequence.scaleUpHorizontal();
}

void NoiseSequenceGrid::scaleDownHorizontal()
{
    numCols = (numCols / 2) + 1;
    for (auto& noiseSequence : noiseSequences)
        noiseSequence.scaleDownHorizontal();
}

void NoiseSequenceGrid::scaleUpVertical()
{
    numRows += (numRows - 1);
    for (auto& noiseSequence : noiseSequences)
        noiseSequence.scaleUpVertical();
}

void NoiseSequenceGrid::scaleDownVertical()
{
    numRows = (numRows / 2) + 1;
    for (auto& noiseSequence : noiseSequences)
        noiseSequence.scaleDownVertical();
}
