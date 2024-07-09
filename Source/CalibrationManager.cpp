/*
  ==============================================================================

    CalibrationManager.cpp
    Created: 1 Jul 2024 3:34:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationManager.h"

CalibrationManager::CalibrationManager() 
: calibrationSequencer (calibratedSetPointManager), currentQuestion (Question::defaultQuestion()) 
{
    goToNextQuestion();
    questionSequencer.setListener (this);
}

const std::pair<float, float> CalibrationManager::getNextSample()
{
    return questionSequencer.getNextSample();
}

void CalibrationManager::setSampleRate (float newSampleRate)
{
    questionSequencer.setSampleRate (newSampleRate);
}

void CalibrationManager::calibrateWith (const CalibrationChoice choice)
{
    calibratedSetPointManager.calibrateWith (currentQuestion, choice);
    goToNextQuestion();
}

void CalibrationManager::changeReferencePanTo (float newReferencePan)
{
    calibratedSetPointManager.changeReferencePanTo (newReferencePan);
}

const Curve& CalibrationManager::getCurve() const
{
    return calibratedSetPointManager.getCurve();
}

const Question& CalibrationManager::getCurrentQuestion() const
{
    return currentQuestion;
}

const bool CalibrationManager::isPlayingFirstNote() const
{
    // TODO: Fix isPlayingFirstNote
    return questionSequencer.isPlayingFirstNote();
}

void CalibrationManager::setFletcherMunsonCompensation (float calibrationDB, float calibrationFactor, float fmDB)
{
    calibratedSetPointManager.setFletcherMunsonCompensation (calibrationDB, calibrationFactor, fmDB);
}

void CalibrationManager::sequenceDidFinish()
{
    calibratedSetPointManager.calibrateWith (currentQuestion, CalibrationChoice::NoPreference);
    currentQuestion.decreaseTempo();
    questionSequencer.setQuestion (currentQuestion);
}

void CalibrationManager::goToNextQuestion()
{
    currentQuestion = calibrationSequencer.getNextQuestion();
    questionSequencer.setQuestion (currentQuestion);
}
