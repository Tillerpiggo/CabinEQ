/*
  ==============================================================================

    SliderSetPointManager.h
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SetPoint.h"
#include "Curve.h"

/// This class manages reading and writing data in ClearEQ. Adding/removing profiles, changing set points, and a useful interface for getting relevant data is handled here.
class SetPointManager
{
public:
    SetPointManager();
    
    const std::vector<SetPoint>& getSetPoints() { return setPoints; }
    const int getNumPoints() const { return static_cast<int> (setPoints.size()); }
    
    void setSetPointAt (int idx, SetPoint setPoint);
    void setAmplitudeAt (int idx, float newAmplitude);
    void setPanAt (int idx, float newPan);
    
    const Curve& getCurve() const;
    
    int indexForFrequency (float frequency) const
    {
        for (int i = 0; i < setPoints.size(); ++i)
        {
            if (frequency == setPoints.at (i).frequency)
            {
                return i;
            }
        }
        
        return -1;
    }
    
    static const int NUM_PTS = 60;
    
private:
    std::vector<SetPoint> setPoints;
    Curve curve;
};
