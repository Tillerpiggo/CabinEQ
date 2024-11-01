/*
  ==============================================================================

    QualityStepManager.cpp
    Created: 31 Oct 2024 5:04:17pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "QualityStepManager.h"


QualityStepManager::QualityStepManager (std::vector<QualityStep> steps)
    : steps (steps)
{
    
}

QualityStep QualityStepManager::getCurrStep()
{
    return steps[stepIdx];
}

void QualityStepManager::goToNextStep()
{
    if (stepIdx < steps.size() - 1)
        stepIdx++;
}

void QualityStepManager::goToPrevStep()
{
    if (stepIdx > 0)
        stepIdx--;
}
