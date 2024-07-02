/*
  ==============================================================================

    CalibrationSequencer.h
    Created: 1 Jul 2024 3:34:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "CalibratedSetPointManager.h"
#include "Note.h"
#include "Question.h"

class CalibrationSequencer
{
public:
    CalibrationSequencer (const CalibratedSetPointManager& calibrationSetPointManager);
    Question getNextQuestion();
    
private:
    const CalibratedSetPointManager& calibrationSetPointManager;
    
    const bool referenceNoteHasBeenCalibrated() const;
    const bool amplitudesHaveBeenWindowed() const;
    const bool phasesHaveBeenCalibratedPrecisely() const;
    const bool pansHaveBeenCalibratedPrecisely() const;
    const bool amplitudesHaveBeenCalibratedPrecisely() const;
    
    CalibratedSetPoint getRandomLowPrecisionSetPoint (const std::vector<CalibratedSetPoint>& calibratedSetPoints) const; // gets random set point of all those w/ least precision
    
    Note referenceNote();
    CalibratedSetPoint referencePan;
    CalibratedSetPoint referencePhase;
    
};
