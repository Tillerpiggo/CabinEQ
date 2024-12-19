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
    NoiseSequence (std::pair<int, int> origin, int id);
    NoiseSequence (std::vector<std::pair<int, int>> coords, int id);
    
    std::vector<std::pair<int, int>> getCoords() const; // returns all coords, including the origin, in absolute terms, with (0, 0) as the top left corner
    std::vector<bool> getHits() const;
    std::pair<int, int> getOrigin() const;
    std::pair<int, int> getPlayingCoordsAtTime (float time) const; // returns the coords that should be playing at that time, given time is from [0, 1] and everything in the sequence is evenly spread over the time (with it being sped up if tempo is sped up)
    float getNoteDurationInTime() const; // returns note duration as a fraction of cycle time
    int getId() const;
    bool hasOriginAt (std::pair<int, int> point) const;
    bool getIsEnabled() const;
    
    void moveOriginTo (std::pair<int, int> newOrigin);
    void addCoords (std::pair<int, int> newCoords); // given in absolute terms, translated to be relative
    void addCoordSequence (std::vector<std::pair<int, int>> coordSequence); // appends the (absolute) coordinate sequence, translated into relative coords
    void removeLastCoords(); // removes the last coords in relative coords
    bool toggleCoords (std::pair<int, int> coords); // toggles whether these coords are hit/not hit. Returns true if the coords are actually in this sequence, and false if the coords are not in this sequence.
    void clearCoords(); // clears relative coords but keeps origin
    
    void scaleUp();
    void scaleDown();
    
private:
    void updatePlayingCoords();
    
    std::vector<std::pair<int, int>> coords; // the starting point - [colIdx, rowIdx] pair
    std::vector<std::pair<int, int>> playingCoords;
    std::vector<bool> hits; // in the order of coords, which ones are "hits".
    bool isEnabled = true;
    float tempo = 1.0f; // 1 = normal speed, 2 = double speed, etc.
    int id;
};
