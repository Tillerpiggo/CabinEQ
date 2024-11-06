/*
  ==============================================================================

    PatternsStep.h
    Created: 6 Nov 2024 1:40:04am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "FauxMusicPattern.h"

// This represents a headphone audio test for faux music/multiple patterns. It includes information to play the patterns as well as how they change (although only the patterns themselves and the respective frequencies are included right now)
class PatternsStep
{
public:
    PatternsStep()
    {}
    
    void addStage (FauxMusicPattern stage)
    {
        std::cout << "pushing back stage" << std::endl;
        std::cout << "stage size: " << stage.getNumPatterns() << std::endl;
        stages.push_back (stage);
    }
    
    const std::vector<FauxMusicPattern>& getStages() const 
    {
        return stages;
    }
    
    const FauxMusicPattern& patternAtStage (int stageIdx) const
    {
        if (stageIdx < 0 || stageIdx >= stages.size())
            std::cerr << "patternAtStage called with stageIdx out of bounds in PatternsStep" << std::endl;
        
        return stages[stageIdx];
    }
    
    int getNumStages() const
    {
        return static_cast<int> (stages.size());
    }
    
private:
    std::vector<FauxMusicPattern> stages;
};
