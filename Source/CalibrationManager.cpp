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
    currentQuestion = calibrationSequencer.getNextQuestion();
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
    switch (currentQuestionType())
    {
        case QuestionType::Level:
            
            break;
        case QuestionType::Pan:
            break;
        case QuestionType::Phase:
            break;
    }
}

const Curve& CalibrationManager::getCurve() const
{
    return calibratedSetPointManager.getCurve();
}

const QuestionType CalibrationManager::currentQuestionType()
{
    return currentQuestion.getType();
}

const bool CalibrationManager::isPlayingFirstNote()
{
    return calibrationIntervalSequencer.isPlayingFirstNote();
}
