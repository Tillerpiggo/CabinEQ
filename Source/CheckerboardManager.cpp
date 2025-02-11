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

const Checkerboard CheckerboardManager::getCheckerboard()
{
    return checkerboard;
}

void CheckerboardManager::setResolution (int resolution)
{
    checkerboard.setResolution (resolution);
}

void CheckerboardManager::togglePolarity()
{
    checkerboard.togglePolarity();
}
