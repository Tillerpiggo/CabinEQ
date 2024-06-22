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
    void calibrateWith (Choice choice);
    double getNextSample();
    
private:
    CalibrationManagerDelegate* delegate;
    Sequencer sequencer;
    //Curve curve;
};

class CalibrationManagerDelegate
{
    virtual ~CalibrationManagerDelegate() = default;
    virtual void choseOption (CalibrationManager::Choice choice) = 0;
};
