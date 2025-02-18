/*
  ==============================================================================

    CheckerboardManager.cpp
    Created: 10 Feb 2025 7:31:36pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CheckerboardManager.h"

CheckerboardManager::CheckerboardManager()
{
    
}

const Checkerboard CheckerboardManager::getCurrCheckerboard()
{
    return checkerboards[currIdx];
}

void CheckerboardManager::selectIdx (int idx)
{
    if (idx < 0 || idx >= checkerboards.size())
    {
        std::cerr << "selectIdx called with idx out of bounds (idx = " << idx << ", numCheckerboards = " << getNumCheckerboards() << std::endl;
    }
    
    currIdx = idx;
}

int CheckerboardManager::getNumCheckerboards()
{
    return static_cast<int> (checkerboards.size());
}

std::string CheckerboardManager::getNameAtIdx (int idx)
{
    if (idx < 0 || idx >= checkerboards.size())
    {
        std::cerr << "getNameAtIdx called with idx out of bounds (idx = " << idx << ", numCheckerboards = " << getNumCheckerboards() << std::endl;
        return "ERROR";
    }
    
    return checkerboards[idx].getTitle();
}

void CheckerboardManager::goToNext()
{
    if (hasNext())
        currIdx++;
}

void CheckerboardManager::goToPrev()
{
    if (hasPrev())
        currIdx--;
}

bool CheckerboardManager::hasNext()
{
    return currIdx < checkerboards.size() - 1;
}

bool CheckerboardManager::hasPrev()
{
    return currIdx > 0;
}

void CheckerboardManager::togglePolarity()
{
    checkerboards[currIdx].togglePolarity();
}
