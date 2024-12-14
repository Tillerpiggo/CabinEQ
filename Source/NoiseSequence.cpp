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
}

void NoiseSequence::clearCoords()
{
    relativeCoords.clear();
}
