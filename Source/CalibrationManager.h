/*
  ==============================================================================

    CalibrationManager.h
    Created: 22 Jun 2024 11:08:05am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "Sequencer.h"
#include "CalibratedSetPoint.h"
#include "CalibrationSequence.h"
#include "Curve.h"

class CalibrationManagerDelegate
{
    virtual ~CalibrationManagerDelegate() = default;
    virtual void choseOption (CalibrationChoice choice) = 0;
};

class CalibrationManager
{
public:
    CalibrationManager() {}
    
    void setDelegate (CalibrationManagerDelegate* delegate);
    void chooseOption (CalibrationChoice choice);
    float getNextSample();
    
private:
    CalibrationManagerDelegate* delegate;
    Sequencer sequencer;
    
    CalibrationSequence calibrationSequence;
    std::map<float, CalibratedSetPoint> setPoints;
    float currentSetPointFreq = -1.0; // not in the map by default; should crash if accessed early
    Curve curve;
    
    void changeMelodyTo (const Melody& melody);
};

