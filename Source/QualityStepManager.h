/*
  ==============================================================================

    QualityStepManager.h
    Created: 31 Oct 2024 5:04:17pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "QualityStep.h"
#include "MelodicNotes.h"

class QualityStepManager
{
public:
    QualityStepManager();
    
    QualityStep getCurrStep();
    void goToNextStep();
    void goToPrevStep();
    
    void addSpatialStep (SpatialStep spatialStep);
    void addIntelligibilityStep (IntelligibilityStep intelligibilityStep);

private:
    std::vector<QualityStep> steps;
    int stepIdx = 0;
};
