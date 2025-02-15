/*
  ==============================================================================

    Checkerboard.cpp
    Created: 10 Feb 2025 3:19:14pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Checkerboard.h"

Checkerboard::Checkerboard()
    : title ("Unnamed Calibration"), numRows (2), numCols (2), polarity (true)
{
    calculateGrid();
}

Checkerboard::Checkerboard (std::string title, int numRows, int numCols, bool polarity)
    : title (title), numRows (numRows), numCols (numCols), polarity (polarity)
{
    calculateGrid();
}

std::string Checkerboard::getTitle() const
{
    return title;
}

std::pair<int, int> Checkerboard::getGridDimensions() const
{
    return { numRows, numCols };
}

bool Checkerboard::getPolarity() const
{
    return polarity;
}

int Checkerboard::getNumNoiseGenerators() const
{
    // Just count the number of 1's in grid
    int numNoiseGenerators = 0;
    for (int row = 0; row < numRows; ++row)
    {
        for (int col = 0; col < numCols; ++col)
        {
            if (grid[row][col])
                numNoiseGenerators++;
        }
    }
    return numNoiseGenerators;
}

std::vector<std::vector<bool>> Checkerboard::getGrid() const
{
    return grid;
}

//void Checkerboard::setSharpness (float sharpness)
//{
//    this->sharpness = sharpness;
//}

void Checkerboard::togglePolarity()
{
    polarity = ! polarity;
    calculateGrid();
}

void Checkerboard::calculateGrid()
{
    // Calculates and populates the grid based on current resolution and polarity
    grid.clear();
    
    bool colPolarity = polarity; // invert every other column, don't change the first column
    bool rowPolarity = false;
    for (int freqIdx = 0; freqIdx < numRows; freqIdx++)
    {
        grid.push_back (std::vector<bool>());
        for (int panIdx = 0; panIdx < numCols; panIdx++)
        {
            grid[freqIdx].push_back (rowPolarity ^ colPolarity);
            colPolarity = ! colPolarity;
            std::cout << "numRows: " << numRows << ", numCols: " << numCols << ", point: " << (rowPolarity ^ colPolarity) << std::endl;
        }
        colPolarity = polarity; // start at the same base polarity
        rowPolarity = ! rowPolarity; // make sure every other row is inverted
    }
}
