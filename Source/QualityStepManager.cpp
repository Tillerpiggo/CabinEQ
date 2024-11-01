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
//    // Let's hard code the steps in here
//    std::vector<QualityStep> initialSteps;
//    
//    // Add a super mario pattern
//    MelodicNotes superMarioMelody =
//    MelodicNotes::withMelodicPattern ({ 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, { 4 - 48, 4 - 24, 4, 0 + 24, 4 + 48, 7 + 24, -5 }, 10000.0f, 1.0f, { 1.0f })
//        .withNoteDurationInSeconds (0.1f)
//        .withTransposition(-18);
//    AudioPattern superMarioPattern = AudioPattern::intelligibility(superMarioMelody.noiseNotes());
//    
//    initialSteps.push_back (QualityStep (QualityStep::Type::intelligibility, superMarioPattern, DifficultyRange (0.5f, 2.0f, 5.0f, 2.0f)));
//    
//    steps = initialSteps;
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

QualityStep QualityStepManager::addSpatialStep (SpatialStep spatialStep)
{
    steps.push_back (QualityStep::spatial (spatialStep));
}

QualityStep QualityStepManager::addIntelligibilityStep (IntelligibilityStep intelligibilityStep)
{
    steps.push_back (QualityStep::intelligibility (intelligibilityStep));
}
