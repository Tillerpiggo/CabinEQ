/*
  ==============================================================================

    Checkerboard.cpp
    Created: 10 Feb 2025 3:19:14pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Checkerboard.h"

Checkerboard::Checkerboard()
    : resolution (2), polarity (true)
{
    calculateGrid();
}

Checkerboard::Checkerboard (int resolution, bool polarity)
    : resolution (resolution), polarity (polarity)
{
    calculateGrid();
}

int Checkerboard::getResolution() const
{
    return resolution;
}

bool Checkerboard::getPolarity() const
{
    return polarity;
}

int Checkerboard::getNumNoiseGenerators() const
{
    // Just count the number of 1's in grid
    int numNoiseGenerators = 0;
    for (int i = 0; i < resolution; ++i)
    {
        for (int j = 0; j < resolution; ++j)
        {
            if (grid[i][j])
                numNoiseGenerators++;
        }
    }
    return numNoiseGenerators;
}

std::vector<std::vector<bool>> Checkerboard::getGrid() const
{
    return grid;
}

void Checkerboard::setResolution (int resolution)
{
    this->resolution = resolution;
    calculateGrid();
}


void Checkerboard::togglePolarity()
{
    polarity = ! polarity;
    calculateGrid();
}

void Checkerboard::calculateGrid()
{
    // Calculates and populates the grid based on current resolution and polarity
    grid.clear();
    
    bool colPolarity = false; // invert every other column, don't change the first column
    bool rowPolarity = polarity;
    for (int panIdx = 0; panIdx < resolution; panIdx++)
    {
        grid.push_back (std::vector<bool>());
        for (int freqIdx = 0; freqIdx < resolution; freqIdx++)
        {
            grid[panIdx].push_back (rowPolarity ^ colPolarity);
            rowPolarity = ! rowPolarity;
        }
        rowPolarity = polarity; // start at the same base polarity
        colPolarity = ! colPolarity; // make sure every other column is inverted
    }
}
