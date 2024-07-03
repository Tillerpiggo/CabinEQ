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
}

const std::pair<float, float> CalibrationManager::getNextSample()
{
    return calibrationIntervalSequencer.getNextSample();
}

void CalibrationManager::setSampleRate (float newSampleRate)
{
    calibrationIntervalSequencer.setSampleRate (newSampleRate);
}

void CalibrationManager::calibrateWith (const CalibrationChoice choice)
{
    calibratedSetPointManager.calibrateWith (currentQuestion, choice);
    goToNextQuestion();
}

const Curve& CalibrationManager::getCurve() const
{
    return calibratedSetPointManager.getCurve();
}

const Question& CalibrationManager::getCurrentQuestion()
{
    return currentQuestion;
}

const bool CalibrationManager::isPlayingFirstNote()
{
    return calibrationIntervalSequencer.isPlayingFirstNote();
}

void CalibrationManager::goToNextQuestion()
{
    currentQuestion = calibrationSequencer.getNextQuestion();
    calibrationIntervalSequencer.setNotes (currentQuestion.getNote1(), currentQuestion.getNote2());
}
