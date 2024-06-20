/*
  ==============================================================================

    SetPointManager.h
    Created: 12 Jun 2024 10:41:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

class SetPointManager
{
public:
    SetPointManager (int numPoints)
    : setPointFreqs(initializeSetPoints(numPoints)) {}
    
    const std::vector<float>& getSetPoints() const { return setPointFreqs; }
    const float getFrequencyForIndex (int index) const
    {
        return setPointFreqs.at(index);
    }
    
    // Static funcs
    static std::vector<float> initializeSetPoints (int numPoints);
    
private:
    std::vector<float> setPointFreqs;
};
