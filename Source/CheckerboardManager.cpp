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

int CheckerboardManager::getNumCheckerboards()
{
    return static_cast<int> (checkerboards.size());
}

std::string CheckerboardManager::getNameAtIdx (int idx)
{
    if (idx < 0 || idx >= getNumCheckerboards())
    {
        std::cerr << "called getNameAtIdx in CheckerboardManager with out of bounds idx  (idx = " << idx << ", numCheckerboards = " << getNumCheckerboards() << std::endl;
        return "ERROR";
    }
    
    return checkerboards[idx].getTitle();
}

int CheckerboardManager::getSelectedRow()
{
    return currIdx;
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

void CheckerboardManager::selectIdx (int idx)
{
    if (idx < 0 || idx >= getNumCheckerboards())
    {
        std::cerr << "selectIdx called in CheckerboardManager with idx out of bounds (idx = " << idx << ", numCheckerboards = " << getNumCheckerboards() << std::endl;
    }
    
    currIdx = idx;
}
