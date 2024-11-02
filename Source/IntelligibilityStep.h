/*
  ==============================================================================

    IntelligibilityQualityStep.h
    Created: 1 Nov 2024 2:03:35pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MelodicNotes.h"

// This represents a headphone audio test for intelligibility. It includes information to play the main test, a difficulty gradient for the test, as well as variations of the test or multiple distinct test audios to "pass" the test.
class IntelligibilityStep
{
public:
    IntelligibilityStep()
    {}
    
    void addStage (MelodicNotes stage)
    {
        stages.push_back (stage);
    }
    
    void setSineVolume (float sineVolumeDb)
    {
        this->sineWaveDbAboveNoise = sineVolumeDb;
    }
    
    const std::vector<MelodicNotes>& getStages() const
    {
        return stages;
    }
    
    const MelodicNotes& patternAtStage (int stageIdx) const
    {
        if (stageIdx < 0 || stageIdx >= stages.size())
            std::cerr << "patternAtStage called with stageIdx out of bounds in SpatialStep" << std::endl;
        
        return stages[stageIdx];
    }
    
    int getNumStages() const
    {
        return stages.size();
    }
    
private:
    float sineWaveDbAboveNoise = 3.0f;
    std::vector<MelodicNotes> stages; // each stage is a melody - to complete the step, pass all stages
    
    
};
