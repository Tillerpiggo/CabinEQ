/*
  ==============================================================================

    RandomCalibrationManager.h
    Created: 23 Jun 2024 6:36:43pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SetPointManager.h"
#include "Curve.h"

class RandomCalibrationManager
{
public:
    RandomCalibrationManager () : curve (setPointManager) {}
    
    const float getNextSample() const;
    void setSampleRate (float newSampleRate);
    void setGainAtFreq (float freq, float gainInDecibels);
    void setGainAtIdx (int idx, float gainInDecibels);
    const Curve& getCurve() const;
    
    void goToNextInterval();
    void goToPrevInterval();
    
private:
    SetPointManager setPointManager;
    Curve curve;
};
