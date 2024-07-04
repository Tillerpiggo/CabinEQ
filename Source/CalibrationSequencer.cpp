/*
  ==============================================================================

    CalibrationSequencer.cpp
    Created: 1 Jul 2024 3:34:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationSequencer.h"
#include <random>

CalibrationSequencer::CalibrationSequencer (const CalibratedSetPointManager& calibratedSetPointManager)
: calibratedSetPointManager (calibratedSetPointManager),
  referencePan (REFERENCE_PAN_WINDOW), referencePhase (REFERENCE_PHASE_WINDOW)
{
    
}

Question CalibrationSequencer::getNextQuestion()
{
    // TODO: Figure out what state we need to track to implement a basic calibration sequence
    // For now, just repeatedly ask for panning, then phase, then level of the 11 notes
    if (! referenceNoteHasBeenCalibrated())
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::bernoulli_distribution dist(0.5);
        bool coinFlip = dist(gen);
        
        // Calibrate pan or phase (choose randomly)
        if (coinFlip)
        {
            Note note1 (REFERENCE_GAIN_DB,
                        REFERENCE_FREQ,
                        referencePan.estimatedValue() - referencePan.getWindowSize(),
                        0.0f);
            
            Note note2 (REFERENCE_GAIN_DB,
                        REFERENCE_FREQ,
                        referencePan.estimatedValue() + referencePan.getWindowSize(),
                        0.0f);
            
            return Question (QuestionType::Pan, note1, note2);
        }
        else
        {
            return phaseQuestion (REFERENCE_FREQ, REFERENCE_GAIN_DB, referencePhase);
        }
    }
    else if (! amplitudesHaveBeenWindowed())
    {
        // Get random next amplitude to test
        auto [freq, amplitudeCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionAmplitude();
        
        Note note1 = referenceNote();
        Note note2 (freq, amplitudeCalibratedSetPoint.estimatedValue(), 0.0f, 0.0f);
        
        return Question (QuestionType::HigherThan, note1, note2);
    }
    else if (! phasesHaveBeenCalibratedPrecisely())
    {
        // Get random next phase to calibrate
        auto [freq, phaseCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionPhase();
        return phaseQuestion (freq, calibratedSetPointManager.amplitudeAt (freq), phaseCalibratedSetPoint);
    }
    else if (! pansHaveBeenCalibratedPrecisely())
    {
        // Get random next pan to calibrate
        auto [freq, panCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionPan();
        
        Note note1 = referenceNote();
        Note note2 (freq, calibratedSetPointManager.amplitudeAt (freq), panCalibratedSetPoint.estimatedValue(), 0.0f);
        
        return Question (QuestionType::Pan, note1, note2);
    }
    else if (! amplitudesHaveBeenCalibratedPrecisely())
    {
        // Get random next level to calibrate
        auto [freq, amplitudeCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionAmplitude();
        
        Note note1 = referenceNote();
        Note note2 (freq, amplitudeCalibratedSetPoint.estimatedValue(), 0.0f, 0.0f);
        
        return Question (QuestionType::Level, note1, note2);
    }
    else
    {
        return Question::defaultQuestion();
    }
}

Question CalibrationSequencer::phaseQuestion (const float frequency, const float gain, const CalibratedSetPoint phase) const
{
    float GAIN_INCREASE = 6.0f;
    float PAN = 0.0f;
    Note note1 (gain + GAIN_INCREASE, frequency, PAN, phase.estimatedValue() - phase.getWindowSize());
    Note note2 (gain + GAIN_INCREASE, frequency, PAN, phase.estimatedValue() + phase.getWindowSize());
    
    return Question (QuestionType::Phase, note1, note2);
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
