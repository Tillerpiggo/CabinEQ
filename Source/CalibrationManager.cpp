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
    return questionSequencer.getNextSample();
}

void CalibrationManager::setSampleRate (float newSampleRate)
{
    questionSequencer.setSampleRate (newSampleRate);
}

void CalibrationManager::calibrateWith (const CalibrationChoice choice)
{
    std::cout << "Calibrating with choice: " << std::endl;
    if ((currentQuestion.getType() == QuestionType::HigherThan ||
        currentQuestion.getType() == QuestionType::LowerThan) &&
        choice == CalibrationChoice::LowerPreferred)
    {
        if (currentQuestion.getType() == QuestionType::HigherThan)
        {
            currentQuestion.increaseControlledNoteVolume();
        }
        else
        {
            currentQuestion.decreaseControlledNoteVolume();
        }
        questionSequencer.setQuestion (currentQuestion);
    }
    else
    {
        calibratedSetPointManager.calibrateWith (currentQuestion, choice);
        goToNextQuestion();
    }
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
    return true;//questionSequencer.isPlayingFirstNote();
}

void CalibrationManager::goToNextQuestion()
{
    currentQuestion = calibrationSequencer.getNextQuestion();
    questionSequencer.setQuestion (currentQuestion);
}
