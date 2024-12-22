/*
  ==============================================================================

    NoiseSequence.cpp
    Created: 13 Dec 2024 9:02:37pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "NoiseSequence.h"

NoiseSequence::NoiseSequence (std::pair<int, int> origin, int id)
    : id (id)
{
    coords.push_back (origin);
    hits.push_back (true);
    updatePlayingCoords();
}

NoiseSequence::NoiseSequence (std::vector<std::pair<int, int>> coords, int id)
    : id (id)
{
    this->coords = coords;
    hits = std::vector<bool> (coords.size(), true);
    updatePlayingCoords();
}

std::vector<std::pair<int, int>> NoiseSequence::getCoords() const
{
    return coords;
}

std::vector<bool> NoiseSequence::getHits() const
{
    return hits;
}

std::pair<int, int> NoiseSequence::getOrigin() const
{
    return coords[0];
}

std::pair<int, int> NoiseSequence::getPlayingCoordsAtTime (float time) const
{
    float spedUpTime = fmin (fmod (time * tempo, 1.0f), 0.99);
    int playingIdx = floor (spedUpTime * (playingCoords.size()));
    return playingCoords[playingIdx];
}

//std::pair<float, float> NoiseSequence::getPlayingCoordsAtTime (float time) const
//{
//    // Scale and wrap the time based on tempo
//    float spedUpTime = fmin(fmod(time * tempo, 1.0f), 0.99f);
//
//    // Calculate the scaled index
//    float scaledIndex = spedUpTime * (playingCoords.size() - 1);
//
//    // Get the indices for interpolation
//    int indexA = static_cast<int>(scaledIndex); // Integer part
//    int indexB = indexA + 1; // Next index
//
//    // Ensure indexB wraps around if it exceeds the size
//    if (indexB >= playingCoords.size()) {
//        indexB = 0;
//    }
//
//    // Get the fractional part of the scaled index
//    float t = scaledIndex - indexA;
//
//    // Retrieve the coordinates for both indices
//    auto coordA = playingCoords[indexA];
//    auto coordB = playingCoords[indexB];
//
//    // Linearly interpolate between the two coordinates
//    float interpolatedX = coordA.first + t * (coordB.first - coordA.first);
//    float interpolatedY = coordA.second + t * (coordB.second - coordA.second);
//
//    return {interpolatedX, interpolatedY};
//}

float NoiseSequence::getNoteDurationInTime() const
{
    int numPlayingCoords = (int) playingCoords.size();
    return 1.0f / ((float) tempo * (float) numPlayingCoords);
}

int NoiseSequence::getId() const
{
    return id;
}

bool NoiseSequence::hasOriginAt (std::pair<int, int> point) const
{
    auto origin = coords[0];
    return origin.first == point.first && origin.second == point.second;
}

bool NoiseSequence::getIsEnabled() const
{
    return isEnabled;
}

void NoiseSequence::moveOriginTo (std::pair<int, int> newOrigin)
{
    auto origin = getOrigin();
    int rowsToMove = newOrigin.first - origin.first;
    int colsToMove = newOrigin.second - origin.second;
    
    // Assume this is being done properly - this method doesn't check if the origin is "out of bounds" in any sense
    for (int i = 0; i < coords.size(); ++i)
    {
        coords[i].first += rowsToMove;
        coords[i].second += colsToMove;
    }
    updatePlayingCoords();
}

void NoiseSequence::addCoords (std::pair<int, int> newCoords)
{
    coords.push_back (newCoords);
    hits.push_back (true);
    updatePlayingCoords();
}

void NoiseSequence::addCoordSequence (std::vector<std::pair<int, int>> coordSequence)
{
    for (const auto& newCoords : coordSequence)
    {
        coords.push_back (newCoords);
        hits.push_back (true);
    }
    updatePlayingCoords();
}

void NoiseSequence::removeLastCoords()
{
    // Never remove the origin
    if (coords.size() <= 1)
        return;
    
    coords.pop_back();
    hits.pop_back();
    updatePlayingCoords();
}

bool NoiseSequence::toggleCoords (std::pair<int, int> point)
{
    for (int i = 0; i < coords.size(); ++i)
    {
        if (coords[i].first == point.first && coords[i].second == point.second)
        {
            hits[i] = ! hits[i];
            updatePlayingCoords();
            return true;
        }
    }
    
    updatePlayingCoords();
    return false;
}

void NoiseSequence::clearCoords()
{
    // TBH i don't think this method should be called
    coords.clear();
    hits.clear();
    updatePlayingCoords();
}

void NoiseSequence::scaleUp()
{
    // First, convert the coordinates to the scaled coordinates
    for (int i = 0; i < coords.size(); ++i)
    {
        coords[i].first *= 2;
        coords[i].second *= 2;
    }
    
    // Then, add in extra coordinates to fill in any gaps
    std::vector<std::pair<int, int>> newCoords;
    std::vector<bool> newHits;
    
    for (int i = 0; i < coords.size(); ++i)
    {
        newCoords.push_back (coords[i]);
        newHits.push_back (hits[i]);
        
        // Figure out the in-between coordinates to push back
        int inBetweenRow;
        int inBetweenCol;
        if (coords[i].first == coords[i + 1].first)
            inBetweenRow = coords[i].first;
        else
            inBetweenRow = (coords[i].first + coords[i + 1].first) / 2;
        
        if (coords[i].second == coords[i + 1].second)
            inBetweenCol = coords[i].second;
        else
            inBetweenCol = (coords[i].second + coords[i + 1].second) / 2;
        
        if (i < coords.size() - 1)
        {
            newCoords.push_back ({ inBetweenRow, inBetweenCol });
            newHits.push_back (false);
        }
    }
    
    coords = newCoords;
    hits = newHits;
    updatePlayingCoords();
}

void NoiseSequence::scaleDown()
{
    // First, round down the origin to the nearest position, and move everything else along with it
    auto origin = getOrigin();
    int rowsToMove = origin.first % 2;
    int colsToMove = origin.second % 2;
    moveOriginTo ({ origin.first - rowsToMove, origin.second - colsToMove });
    
    // Simply remove any in-between coordinates as you try to scale down
    std::vector<std::pair<int, int>> newCoords;
    std::vector<bool> newHits;
    for (int i = 0; i < coords.size(); ++i)
    {
        if (coords[i].first % 2 == 0 && coords[i].second % 2 == 0)
        {
            newCoords.push_back ({ coords[i].first / 2, coords[i].second / 2 });
            newHits.push_back (hits[i]);
        }
    }
    
    coords = newCoords;
    hits = newHits;
    updatePlayingCoords();
}

//void NoiseSequence::updatePlayingCoords()
//{
//    std::vector<std::pair<int, int>> updatedPlayingCoords;
//    for (int i = 0; i < coords.size(); ++i)
//    {
//        if (hits[i])
//            updatedPlayingCoords.push_back(coords[i]);
//    }
//    
//    // Order from left to right like reading
//    std::sort(updatedPlayingCoords.begin(), updatedPlayingCoords.end(),
//              [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
//                  if (a.first == b.first) // Compare x-coordinates first
//                      return a.second < b.second; // If x-coordinates are equal, compare y-coordinates
//                  return a.first < b.first; // Otherwise, sort by x-coordinate
//              });
//
//    this->playingCoords = updatedPlayingCoords;
//}

void NoiseSequence::updatePlayingCoords()
{
    std::vector<std::pair<int, int>> updatedPlayingCoords;
    for (int i = 0; i < coords.size(); ++i)
    {
        if (hits[i])
            updatedPlayingCoords.push_back (coords[i]);
    }
    
    // Order from left to right like reading
    
    
    this->playingCoords = updatedPlayingCoords;
}
