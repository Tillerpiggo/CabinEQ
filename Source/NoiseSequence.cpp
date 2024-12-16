/*
  ==============================================================================

    NoiseSequence.cpp
    Created: 13 Dec 2024 9:02:37pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "NoiseSequence.h"

NoiseSequence::NoiseSequence (std::pair<int, int> origin, int id)
    : origin (origin), id (id)
{
    hits.push_back (true);
}

NoiseSequence::NoiseSequence (std::vector<std::pair<int, int>> coords, int id)
    : id (id)
{
    origin = coords[0];
    hits.push_back (true);
    for (int i = 1; i < coords.size(); ++i)
    {
        relativeCoords.push_back ({ coords[i].first - origin.first, coords[i].second - origin.second });
        hits.push_back (true);
    }
}

std::vector<std::pair<int, int>> NoiseSequence::getAbsoluteCoords() const
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

std::vector<bool> NoiseSequence::getHits() const
{
    return hits;
}

std::pair<int, int> NoiseSequence::getOrigin() const
{
    return origin;
}

int NoiseSequence::getId() const
{
    return id;
}

bool NoiseSequence::hasOriginAt (std::pair<int, int> point) const
{
    return origin.first == point.first && origin.second == point.second;
}

bool NoiseSequence::getIsEnabled() const
{
    return isEnabled;
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

void NoiseSequence::removeLastCoords()
{
    relativeCoords.pop_back();
}

bool NoiseSequence::toggleCoords (std::pair<int, int> coords)
{
    std::cout << "toggling coords" << std::endl;
    if (hasOriginAt (coords))
    {
        hits[0] = ! hits[0];
        return true;
    }
    
    for (int i = 0; i < relativeCoords.size(); ++i)
    {
        auto point = relativeCoords[i];
        point.first += origin.first;
        point.second += origin.second;
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

void NoiseSequence::scaleUpHorizontal()
{
    // First, convert the coordinates to the scaled coordinates
    origin.first *= 2;
    for (auto& coords : relativeCoords)
        coords.first *= 2;
    
    // Then, add in extra coordinates to fill in any gaps
    std::vector<std::pair<int, int>> newRelativeCoords;
    std::vector<bool> newHits;
    for (int i = 0; i < relativeCoords.size() - 1; ++i)
    {
        newRelativeCoords.push_back (relativeCoords[i]);
        newHits.push_back (hits[i]);
        if (relativeCoords[i].first != relativeCoords[i + 1].first)
        {
            int avgCol = (relativeCoords[i].first + relativeCoords[i + 1].first) / 2;
            newRelativeCoords.push_back ({ avgCol, relativeCoords[i].second });
            newHits.push_back (false);
        }
    }
    newRelativeCoords.push_back (relativeCoords[relativeCoords.size() - 1]);
    
    // Set the new values
    relativeCoords = newRelativeCoords;
    hits = newHits;
}

void NoiseSequence::scaleDownHorizontal()
{
    // Let's just not worry about this case for now
    
//    // First, remove all odd # col indices and corresponding hits
//    std::vector<std::pair<int, int>> newRelativeCoords;
//
//    // Then, scale down current coordinates/columns
}

void NoiseSequence::scaleUpVertical()
{
    // First, convert the coordinates to the scaled coordinates
    origin.second *= 2;
    for (auto& coords : relativeCoords)
        coords.second *= 2;
    
    // Then, add in extra coordinates to fill in any vertical gaps
    std::vector<std::pair<int, int>> newRelativeCoords;
    std::vector<bool> newHits;
    for (int i = 0; i < relativeCoords.size() - 1; ++i)
    {
        newRelativeCoords.push_back (relativeCoords[i]);
        newHits.push_back (hits[i]);
        if (relativeCoords[i].second != relativeCoords[i + 1].second)
        {
            int avgRow = (relativeCoords[i].second + relativeCoords[i + 1].second) / 2;
            newRelativeCoords.push_back ({ avgRow, relativeCoords[i].second });
            newHits.push_back (false);
        }
    }
    newRelativeCoords.push_back (relativeCoords[relativeCoords.size() - 1]);
    
    // Set the new values
    relativeCoords = newRelativeCoords;
    hits = newHits;
}

void NoiseSequence::scaleDownVertical()
{
    // Let's just not worry about this case for now
}
