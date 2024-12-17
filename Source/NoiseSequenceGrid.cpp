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
    updateNextAvailableId();
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
    updateNextAvailableId();
}

void NoiseSequenceGrid::moveSequence (std::pair<int, int> origin, std::pair<int, int> newOrigin)
{
    // Assume there is a sequence at origin, and that the sequence can be moved to newOrigin
    
    // Find the sequence
    for (int i = 0; i < noiseSequences.size(); ++i)
    {
        if (noiseSequences[i].hasOriginAt (origin))
        {
            noiseSequences[i].moveOriginTo (origin);
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
    return nextAvailableId;
}

int NoiseSequenceGrid::getSequenceIdAtCoords (std::pair<int, int> coords)
{
    for (const auto& sequence : noiseSequences)
        if (sequence.hasOriginAt (coords))
            return sequence.getId();
    
    return -1;
}

// Increase/reduce horizontal/vertical dimensions by adding rows/cols in between existing rows
// so 3 -> 5 -> 9 when scaling up twice
void NoiseSequenceGrid::scaleUpGrid()
{
    numRows += (numRows - 1);
    numCols += (numCols - 1);
    for (auto& noiseSequence : noiseSequences)
        noiseSequence.scaleUp();
}

void NoiseSequenceGrid::scaleDownGrid()
{
    numRows = (numRows / 2) + 1;
    numCols = (numCols / 2) + 1;
    for (auto& noiseSequence : noiseSequences)
        noiseSequence.scaleDown();
}

void NoiseSequenceGrid::updateNextAvailableId()
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
    
    nextAvailableId = nextId;
}
