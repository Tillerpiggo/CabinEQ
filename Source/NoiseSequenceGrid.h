/*
  ==============================================================================

    NoiseSequenceGrid.h
    Created: 14 Dec 2024 11:31:05am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "NoiseSequence.h"

// This represents a grid of noise sequences, including the dimensions of the grid, and details about each sequence
class NoiseSequenceGrid
{
public:
    NoiseSequenceGrid (int numRows = 3, int numCols = 3);
    
    void addSequence (NoiseSequence noiseSequence);
    void removeSequence (std::pair<int, int> origin); // removes the sequence with the matching origin, if there is one in this noise sequence grid
    void moveSequence (std::pair<int, int> origin, std::pair<int, int> newOrigin);
    void toggleCoords (std::pair<int, int> coords); // If the coords are within a sequence, toggles them to be on/off
    
    std::vector<NoiseSequence> getNoiseSequences();
    std::pair<int, int> getNumRowsAndNumCols();
    int getNextAvailableId(); // returns the next available id, if starting at 0, and basing it on the existing noise sequences
    
    // Increase/reduce horizontal/vertical dimensions by adding rows/cols in between existing rows
    // so 3 -> 5 -> 9 when scaling up twice
    void scaleUpGrid();
    void scaleDownGrid();
    
private:
    int numRows;
    int numCols;
    std::vector<NoiseSequence> noiseSequences;
};
