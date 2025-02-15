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
