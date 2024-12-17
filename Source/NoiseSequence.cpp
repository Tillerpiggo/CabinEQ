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
}

NoiseSequence::NoiseSequence (std::vector<std::pair<int, int>> coords, int id)
    : id (id)
{
    this->coords = coords;
    hits = std::vector<bool> (coords.size(), true);
}

std::vector<std::pair<int, int>> NoiseSequence::getAbsoluteCoords() const
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
    
    std::cout << "rowsToMove: " << rowsToMove << std::endl;
    std::cout << "colsToMove: " << colsToMove << std::endl;
    
    // Assume this is being done properly - this method doesn't check if the origin is "out of bounds" in any sense
    for (int i = 0; i < coords.size(); ++i)
    {
        coords[i].first += rowsToMove;
        coords[i].second += colsToMove;
    }
}

void NoiseSequence::addCoords (std::pair<int, int> newCoords)
{
    coords.push_back (newCoords);
    hits.push_back (true);
}

void NoiseSequence::addCoordSequence (std::vector<std::pair<int, int>> coordSequence)
{
    for (const auto& newCoords : coordSequence)
    {
        coords.push_back (newCoords);
        hits.push_back (true);
    }
}

void NoiseSequence::removeLastCoords()
{
    // Never remove the origin
    if (coords.size() <= 1)
        return;
    
    coords.pop_back();
}

bool NoiseSequence::toggleCoords (std::pair<int, int> point)
{
    for (int i = 0; i < coords.size(); ++i)
    {
        if (coords[i].first == point.first && coords[i].second == point.second)
        {
            hits[i] = ! hits[i];
            return true;
        }
    }
    
    return false;
}

void NoiseSequence::clearCoords()
{
    // TBH i don't think this method should be called
    coords.clear();
    hits.clear();
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
}

//void NoiseSequence::scaleUpHorizontal()
//{
//    // First, convert the coordinates to the scaled coordinates
//    origin.first *= 2;
//    for (auto& coords : relativeCoords)
//        coords.first *= 2;
//    
//    // Then, add in extra coordinates to fill in any gaps
//    std::vector<std::pair<int, int>> newRelativeCoords;
//    std::vector<bool> newHits;
//    for (int i = 0; i < relativeCoords.size() - 1; ++i)
//    {
//        newRelativeCoords.push_back (relativeCoords[i]);
//        newHits.push_back (hits[i]);
//        if (relativeCoords[i].first != relativeCoords[i + 1].first)
//        {
//            int avgCol = (relativeCoords[i].first + relativeCoords[i + 1].first) / 2;
//            newRelativeCoords.push_back ({ avgCol, relativeCoords[i].second });
//            newHits.push_back (false);
//        }
//    }
//    newRelativeCoords.push_back (relativeCoords[relativeCoords.size() - 1]);
//    
//    // Set the new values
//    relativeCoords = newRelativeCoords;
//    hits = newHits;
//}

//void NoiseSequence::scaleDownHorizontal()
//{
//    // Let's just not worry about this case for now
//    
////    // First, remove all odd # col indices and corresponding hits
////    std::vector<std::pair<int, int>> newRelativeCoords;
////
////    // Then, scale down current coordinates/columns
//}
//
//void NoiseSequence::scaleUpVertical()
//{
//    // First, convert the coordinates to the scaled coordinates
//    origin.second *= 2;
//    for (auto& coords : relativeCoords)
//        coords.second *= 2;
//    
//    // Then, add in extra coordinates to fill in any vertical gaps
//    std::vector<std::pair<int, int>> newRelativeCoords;
//    std::vector<bool> newHits;
//    for (int i = 0; i < relativeCoords.size() - 1; ++i)
//    {
//        newRelativeCoords.push_back (relativeCoords[i]);
//        newHits.push_back (hits[i]);
//        if (relativeCoords[i].second != relativeCoords[i + 1].second)
//        {
//            int avgRow = (relativeCoords[i].second + relativeCoords[i + 1].second) / 2;
//            newRelativeCoords.push_back ({ avgRow, relativeCoords[i].second });
//            newHits.push_back (false);
//        }
//    }
//    newRelativeCoords.push_back (relativeCoords[relativeCoords.size() - 1]);
//    
//    // Set the new values
//    relativeCoords = newRelativeCoords;
//    hits = newHits;
//}

//void NoiseSequence::scaleDownVertical()
//{
//    // Let's just not worry about this case for now
//}
