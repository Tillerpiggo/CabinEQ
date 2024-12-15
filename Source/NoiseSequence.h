/*
  ==============================================================================

    NoiseSequence.h
    Created: 13 Dec 2024 9:02:37pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This class represents a noise sequence on a grid, including the pattern to be played and the sequence of coordinates. A noise sequence follows a contiguous non-overlapping directional path, but some spaces can be empty/not played
class NoiseSequence
{
public:
    NoiseSequence (std::pair<int, int> origin);
    
    std::vector<std::pair<int, int>> getAbsoluteCoords(); // returns all coords, including the origin, in absolute terms, with (0, 0) as the top left corner
    std::vector<bool> getHits();
    std::pair<int, int> getOrigin();
    bool hasOriginAt (std::pair<int, int> point);
    
    void setOrigin (std::pair<int, int> newOrigin);
    void addCoords (std::pair<int, int> newCoords); // given in absolute terms, translated to be relative
    void addCoordSequence (std::vector<std::pair<int, int>> coordSequence); // appends the (absolute) coordinate sequence, translated into relative coords
    bool toggleCoords (std::pair<int, int> coords); // toggles whether these coords are hit/not hit. Returns true if the coords are actually in this sequence, and false if the coords are not in this sequence.
    void clearCoords(); // clears relative coords but keeps origin
    
    void scaleUpHorizontal();
    void scaleDownHorizontal();
    void scaleUpVertical();
    void scaleDownVertical();
    
private:
    std::pair<int, int> origin; // the starting point - [colIdx, rowIdx] pair
    std::vector<std::pair<int, int>> relativeCoords; // all the next coordinates, relative to the origin - [colIdx, rowIdx] pair
    std::vector<bool> hits; // in the order of coords, which ones are "hits". The first hit - whether the origin is on - determines whether the sequence as whole is on. If off, the entire sequence is muted/disabled.
    float tempo; // 1 = normal speed, 1/2 = half speed, etc.
};
