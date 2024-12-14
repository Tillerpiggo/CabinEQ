/*
  ==============================================================================

    NoiseSequence.h
    Created: 13 Dec 2024 9:02:37pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This class represents a noise sequence on a grid, including the pattern to be played and the sequence of coordinates
class NoiseSequence
{
public:
    NoiseSequence (std::pair<int, int> origin);
    
    std::vector<std::pair<int, int>> getAbsoluteCoords(); // returns all coords, including the origin, in absolute terms, with (0, 0) as the bottom left corner
    
    void setOrigin (std::pair<int, int> newOrigin);
    void addCoords (std::pair<int, int> newCoords); // given in absolute terms, translated to be relative
    void clearCoords(); // clears relative coords but keeps origin
    
private:
    std::pair<int, int> origin; // the starting point
    std::vector<std::pair<int, int>> relativeCoords; // all the next coordinates, relative to the origin
};
