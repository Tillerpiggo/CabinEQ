/*
  ==============================================================================

    CalibrationSequencer.cpp
    Created: 1 Jul 2024 3:34:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationSequencer.h"

CalibrationSequencer::CalibrationSequencer (const CalibratedSetPointManager& calibratedSetPointManager)
: calibratedSetPointManager (calibratedSetPointManager),
  referencePan (REFERENCE_PAN_WINDOW), referencePhase (REFERENCE_PHASE_WINDOW)
{
    
}

Question CalibrationSequencer::getNextQuestion()
{
    // TODO: Figure out what state we need to track to implement a basic calibration sequence
    // For now, just repeatedly ask for panning, then phase, then level of the 11 notes
    
    
}

bool CalibrationSequencer::referenceNoteHasBeenCalibrated() const
{
    return referencePan.precision() < 0.02 && referencePhase.precision() < 0.01;
}

bool CalibrationSequencer::amplitudesHaveBeenWindowed() const
{
    return calibratedSetPointManager.amplitudesHaveBeenWindowed();
}

bool CalibrationSequencer::phasesHaveBeenCalibratedPrecisely() const
{
    return calibratedSetPointManager.phasesHaveBeenCalibratedWithPrecision (PHASE_PRECISION);
}

bool CalibrationSequencer::pansHaveBeenCalibratedPrecisely() const
{
    return calibratedSetPointManager.phasesHaveBeenCalibratedWithPrecision (PAN_PRECISION);
}

bool CalibrationSequencer::amplitudesHaveBeenCalibratedPrecisely() const
{
    return calibratedSetPointManager.phasesHaveBeenCalibratedWithPrecision (AMPLITUDE_PRECISION);
}

Note CalibrationSequencer::referenceNote()
{
    return Note (REFERENCE_FREQ, 
                 REFERENCE_GAIN_DB,
                 referencePan.estimatedValue(), 
                 referencePhase.estimatedValue());
}
