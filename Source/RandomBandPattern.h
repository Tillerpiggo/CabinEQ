/*
  ==============================================================================

    RandomBandPattern.h
    Created: 11 Nov 2024 6:57:13pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandProfile.h"
#include <random>

// This class manages a random pattern of some number of peaks/dips with different bandwidths that can be transistioned between along a clear sliding scale
class RandomBandPattern
{
public:
    RandomBandPattern (int numPositions, int numBands);
    std::vector<Band> getBandsAtTime (float time);
    
private:
    std::vector<std::vector<Band>> randomPositions;
    int numBands;
};
