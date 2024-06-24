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
#include "IntervalSequencer.h"

class RandomCalibrationManager
{
public:
    RandomCalibrationManager ();
    
    const float getNextSample() const;
    void setSampleRate (float newSampleRate);
    void setCurrGain (float gainInDecibels);
    void setGainAtIdx (int i, float gainInDecibels);
    const Curve& getCurve() const;
    
    void updateSequencer();
    
    int goToNextInterval(); // returns the corresponding idx used by SetPointManager, -1 if no next
    int goToPrevInterval(); // returns the corresponding idx used by SetPointManager, -1 if no prev
    
private:
    IntervalSequencer sequencer;
    SetPointManager setPointManager;
    Curve curve;
    
    std::vector<int> intervalOrder; // idx -> SetPointManager idx
    int currIntervalIdx;
};
