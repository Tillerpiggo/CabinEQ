/*
  ==============================================================================

    SpatialQualityStep.h
    Created: 1 Nov 2024 2:03:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Glyph.h"

// This represents a headphone audio test for spatial properties. It includes information to play the main test, as well as variations of the test or multiple distinct test audios to "pass" the test.
class SpatialStep
{
public:
    SpatialStep()
    {}
    
    void addStage (Glyph stage)
    {
        stages.push_back (stage);
    }
    
    void addStage (MelodicNotes melodicNotes)
    {
        stages.push_back (Glyph (melodicNotes));
    }
    
    void addStage (std::vector<SweepPattern> sweepPatterns)
    {
        stages.push_back (Glyph (sweepPatterns));
    }
    
    void addShape (std::vector<std::pair<float, float>> shapeCorners, float sampleRate, float bandwidth = 1.0f)
    {
        // Add a pattern shape
        Glyph cornerGlyph = Glyph::shapeFromCorners (shapeCorners, bandwidth);
        
        // Add a pattern sweep
        Glyph shapeGlyph = Glyph ({ SweepPattern (shapeCorners, 2.0f, sampleRate) });
        
        stages.push_back (cornerGlyph);
        stages.push_back (shapeGlyph);
    }
    
    const std::vector<Glyph>& getStages() const
    {
        return stages;
    }
    
    const Glyph& patternAtStage (int stageIdx) const
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
    std::vector<Glyph> stages; // each stage is a sweep pattern - to complete the step, you must be able to clearly hear all sweep patterns
};

