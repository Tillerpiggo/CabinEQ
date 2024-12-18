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

void NoiseSequenceGrid::moveSequence (int id, std::pair<int, int> newOrigin)
{
    // Assume there is a sequence at origin, and that the sequence can be moved to newOrigin
    
    // Find the sequence
    for (int i = 0; i < noiseSequences.size(); ++i)
    {
        if (noiseSequences[i].getId() == id)
        {
            noiseSequences[i].moveOriginTo (newOrigin);
        }
    }
}

void NoiseSequenceGrid::toggleCoords (std::pair<int, int> coords)
{
    for (auto& noiseSequence : noiseSequences)
        if (noiseSequence.toggleCoords (coords))
            return;
}

const std::vector<NoiseSequence>& NoiseSequenceGrid::getNoiseSequences()
{
    return noiseSequences;
}

std::vector<std::pair<float, float>> NoiseSequenceGrid::getNormalizedPlayingCoordsAtTime (float time)
{
    std::vector<std::pair<int, int>> playingCoords;
    for (const auto& sequence : noiseSequences)
    {
        playingCoords.push_back (sequence.getPlayingCoordsAtTime (time));
    }
    
    std::vector<std::pair<float, float>> normalizedPlayingCoords;
    for (const auto& coords : playingCoords)
    {
        normalizedPlayingCoords.push_back (getNormalizedCoordsFor (coords));
    }
    
    return normalizedPlayingCoords;
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

std::pair<float, float> NoiseSequenceGrid::getNormalizedCoordsFor (std::pair<int, int> coords)
{
    // First normalize x and y to [0, 1] (flip x and y because .first refers to rows and .second refers to columns
    float normalizedY = (((float) coords.first)) / ((float) numCols - 1.0f);
    float normalizedX = (((float) coords.second)) / ((float) numRows - 1.0f);
    
    // Then convert to [-1, 1]
    normalizedX = normalizedX * 2.0f - 1.0f;
    normalizedY = normalizedY * 2.0f - 1.0f;
    
    std::pair<float, float> normalizedCoords = { normalizedX, normalizedY };
    return normalizedCoords;
}
