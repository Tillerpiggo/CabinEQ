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
    
    const std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void setCurrGain (float gainInDecibels);
    void setCurrPan (float balanceGainInDecibels);
    void setCurrPhase (float phaseInRadians);
    void setGainAtIdx (int i, float gainInDecibels);
    void setPanAtIdx (int i, float gainInDecibels);
    void setPhaseAtIdx (int i, float phaseInRadians);
    
    const Curve& getCurve() const { return frCurve; }
    
    void updateSequencer();
    
    int goToNextInterval(); // returns the corresponding idx used by SetPointManager, -1 if no next
    int goToPrevInterval(); // returns the corresponding idx used by SetPointManager, -1 if no prev
    
private:
    IntervalSequencer intervalSequencer;
    SetPointManager gainSetPointManager;
    SetPointManager panSetPointManager;
    SetPointManager phaseSetPointManager;
    Curve frCurve; // frequency response curve
    
    std::vector<int> intervalOrder; // idx -> SetPointManager idx
    int currIntervalIdx;
};
