/*
  ==============================================================================

    QualityStepManager.cpp
    Created: 31 Oct 2024 5:04:17pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "QualityStepManager.h"


QualityStepManager::QualityStepManager()
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

void QualityStepManager::addSpatialStep (SpatialStep spatialStep)
{
    steps.push_back (QualityStep::spatial (spatialStep));
}

void QualityStepManager::addIntelligibilityStep (IntelligibilityStep intelligibilityStep)
{
    steps.push_back (QualityStep::intelligibility (intelligibilityStep));
}

void QualityStepManager::addPatternsStep (PatternsStep patternsStep)
{
    steps.push_back (QualityStep::patterns (patternsStep));
}
