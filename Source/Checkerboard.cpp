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
    
}

Checkerboard::Checkerboard (int resolution, bool polarity)
    : resolution (resolution), polarity (polarity)
{
}

int Checkerboard::getResolution()
{
    return resolution;
}

bool Checkerboard::getPolarity()
{
    return polarity;
}

int Checkerboard::getNumNoiseGenerators()
{
    // TODO: test this function
    
    // Compute num noise generators with dumb brute force
    int numNoiseGenerators = 0;
    bool colPolarity = false; // invert every other column, don't change the first column
    bool rowPolarity = polarity;
    for (int panIdx = 0; panIdx < resolution; panIdx++)
    {
        for (int freqIdx = 0; freqIdx < resolution; freqIdx++)
        {
            if (rowPolarity ^ colPolarity)
                numNoiseGenerators++;
            rowPolarity = ! rowPolarity;
        }
        rowPolarity = polarity; // start at the same base polarity
        colPolarity = ! colPolarity; // make sure every other column is inverted
    }
    
    return numNoiseGenerators;
}


void Checkerboard::togglePolarity()
{
    polarity = ! polarity;
}
