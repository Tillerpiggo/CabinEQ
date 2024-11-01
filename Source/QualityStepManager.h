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

class QualityStepManager
{
public:
    QualityStepManager (std::vector<QualityStep> steps);
    
    QualityStep getCurrStep();
    void goToNextStep();
    void goToPrevStep();
    
private:
    std::vector<QualityStep> steps;
    int stepIdx = 0;
};
