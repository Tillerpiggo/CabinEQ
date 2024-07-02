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
    frequencies = { 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240 };
    
    for (float freq : frequencies)
    {
        amplitudes.push_back (CalibratedSetPoint (12.0f));
        pans.push_back (CalibratedSetPoint (3.0f));
        phases.push_back (CalibratedSetPoint (3.14f));
    }
}
