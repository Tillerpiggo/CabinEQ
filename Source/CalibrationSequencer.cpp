/*
  ==============================================================================

    CalibrationSequencer.cpp
    Created: 1 Jul 2024 3:34:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationSequencer.h"

CalibrationSequencer::CalibrationSequencer (const CalibratedSetPointManager& calibrationSetPointManager)
: calibrationSetPointManager (calibrationSetPointManager),
  referencePan (3.0f), referencePhase (3.14f)
{
    
}

Question CalibrationSequencer::getNextQuestion()
{
    // TODO: Figure out what state we need to track to implement a basic calibration sequence
    // For now, just repeatedly ask for panning, then phase, then level of the 11 notes
    
}
