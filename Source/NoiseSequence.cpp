/*
  ==============================================================================

    NoiseSequence.cpp
    Created: 13 Dec 2024 9:02:37pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "NoiseSequence.h"

NoiseSequence::NoiseSequence (std::pair<int, int> origin)
    : origin (origin)
{
    
}

std::vector<std::pair<int, int>> NoiseSequence::getAbsoluteCoords()
{
    std::vector<std::pair<int, int>> absoluteCoords;
    absoluteCoords.push_back (origin);
    for (const auto& relativeCoordPair : relativeCoords)
    {
        std::pair<int, int> absoluteCoordPair = { relativeCoordPair.first + origin.first,
            relativeCoordPair.second + origin.second };
        absoluteCoords.push_back (absoluteCoordPair);
    }
    return absoluteCoords;
}

std::vector<bool> NoiseSequence::getHits()
{
    return hits;
}

std::pair<int, int> NoiseSequence::getOrigin()
{
    return origin;
}

bool NoiseSequence::hasOriginAt (std::pair<int, int> point)
{
    return origin.first == point.first && origin.second == point.second;
}

void NoiseSequence::setOrigin (std::pair<int, int> newOrigin)
{
    this->origin = newOrigin;
}

void NoiseSequence::addCoords (std::pair<int, int> newCoords)
{
    std::pair<int, int> newCoordsRelative = newCoords;
    newCoordsRelative.first -= origin.first;
    newCoordsRelative.second -= origin.second;
    relativeCoords.push_back (newCoordsRelative);
    hits.push_back (true);
}

void NoiseSequence::addCoordSequence (std::vector<std::pair<int, int>> coordSequence)
{
    for (const auto& coords : coordSequence)
    {
        relativeCoords.push_back ({ coords.first - origin.first, coords.second - origin.second });
        hits.push_back (true);
    }
}

bool NoiseSequence::toggleCoords (std::pair<int, int> coords)
{
    if (hasOriginAt (coords))
    {
        hits[0] = ! hits[0];
        return true;
    }
    
    for (int i = 0; i < relativeCoords.size(); ++i)
    {
        auto point = relativeCoords[i];
        if (coords.first == point.first && coords.second == point.second)
        {
            hits[i + 1] = ! hits[i + 1];
            return true;
        }
    }
    
    return false;
}

void NoiseSequence::clearCoords()
{
    relativeCoords.clear();
    hits.clear();
}
