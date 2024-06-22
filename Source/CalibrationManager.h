/*
  ==============================================================================

    CalibrationManager.h
    Created: 22 Jun 2024 11:08:05am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "Curve.h"
#include "Sequencer.h"
#include "CalibratedSetPoint.h"
#include "CalibrationSequence.h"
#include "CalibratedSetPoint.h"

class CalibrationManagerDelegate;

class CalibrationManager
{
public:
    enum class Choice
    {
        LowerPreferred,
        HigherPreferred,
    };
    
    CalibrationManager() {}
    
    void setDelegate (CalibrationManagerDelegate* delegate);
    void chooseOption (Choice choice);
    double getNextSample();
    
private:
    CalibrationManagerDelegate* delegate;
    Sequencer sequencer;
    
    CalibrationSequence calibrationSequence;
    std::map<double, CalibratedSetPoint> setPoints;
    double currentSetPointFreq = -1; // not in the map by default; should crash if accessed early
    Curve curve;
    
    void changeMelodyTo (const Melody& melody);
};

class CalibrationManagerDelegate
{
    virtual ~CalibrationManagerDelegate() = default;
    virtual void choseOption (CalibrationManager::Choice choice) = 0;
};
