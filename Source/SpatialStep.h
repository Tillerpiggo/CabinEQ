/*
  ==============================================================================

    SpatialQualityStep.h
    Created: 1 Nov 2024 2:03:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SweepPattern.h"

// This represents a headphone audio test for spatial properties. It includes information to play the main test, as well as variations of the test or multiple distinct test audios to "pass" the test.
class SpatialStep
{
public:
    SpatialStep();
    
    void addStage (SweepPattern stage)
    {
        stages.push_back (stage);
    }
    
    const std::vector<SweepPattern>& getStages() const
    {
        return stages;
    }
    
private:
    std::vector<SweepPattern> stages; // each stage is a sweep pattern - to complete the step, you must be able to clearly hear all sweep patterns
};

