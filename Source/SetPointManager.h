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
    SetPointManager ()
    : setPointFreqs (initializeSetPointFreqs(NUM_SET_POINTS)),
      setPointGains (std::vector<float> (0.0f, NUM_SET_POINTS)) {}
    
    const std::vector<float>& getSetPointFreqs() const { return setPointFreqs; }
    const std::vector<float>& getSetPointGains() const { return setPointGains; }
    const float getFrequencyForIndex (int index) const
    {
        return setPointFreqs.at(index);
    }
    
    // Static funcs
    static std::vector<float> initializeSetPointFreqs (int numPoints);
    static const int NUM_SET_POINTS = 50;
    
private:
    std::vector<float> setPointFreqs;
    std::vector<float> setPointGains;
};
