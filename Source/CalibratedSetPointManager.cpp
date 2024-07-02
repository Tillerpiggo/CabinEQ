/*
  ==============================================================================

    CalibratedSetPointManager.cpp
    Created: 1 Jul 2024 3:34:15pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibratedSetPointManager.h"

CalibratedSetPointManager::CalibratedSetPointManager() 
: curve(frequencies, 
        amplitudes,
        std::make_shared<std::vector<CalibratedSetPoint>> (pans),
        std::make_shared<std::vector<CalibratedSetPoint>> (phases))
{
    // TODO: Construct the first batch of set points...
}
