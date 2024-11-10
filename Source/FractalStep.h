/*
  ==============================================================================

    FractalStep.h
    Created: 9 Nov 2024 11:31:50pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "FractalPattern.h"

// This represents a headphone audio test for fractal patterns. It just includes the fractal pattern, and different stages with different fractal patterns
class FractalStep
{
public:
    FractalStep()
    {}
    
    void addStage (FractalPattern stage)
    {
        stages.push_back (stage);
    }
    
    const std::vector<FractalPattern>& getStages() const
    {
        return stages;
    }
    
    const FractalPattern& patternAtStage (int stageIdx) const
    {
        if (stageIdx < 0 || stageIdx >= stages.size())
            std::cerr << "patternAtstage called with stageIdx out of bounds in FractalStep" << std::endl;
        
        return stages[stageIdx];
    }
    
    int getNumStages() const
    {
        return static_cast<int> (stages.size());
    }
    
private:
    std::vector<FractalPattern> stages;
};
