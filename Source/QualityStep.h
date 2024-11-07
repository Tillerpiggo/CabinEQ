/*
  ==============================================================================

    QualityStep.h
    Created: 30 Oct 2024 1:17:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "SpatialStep.h"
#include "IntelligibilityStep.h"
#include "PatternsStep.h"

// This class is a wrapper for a SpatialQualityStep and IntelligibilityQualityStep designed to make it easier to use them interchangeably.
class QualityStep
{
public:
    enum class Type
    {
        spatial,
        intelligibility,
        patterns
    };
    
    static QualityStep spatial (SpatialStep spatialStep)
    {
        return QualityStep (Type::spatial, spatialStep, std::nullopt, std::nullopt);
    }
    
    static QualityStep intelligibility (IntelligibilityStep intelligibilityStep)
    {
        return QualityStep (Type::intelligibility, std::nullopt, intelligibilityStep, std::nullopt);
    }
    
    static QualityStep patterns (PatternsStep patternsStep)
    {
        return QualityStep (Type::patterns, std::nullopt, std::nullopt, patternsStep);
    }
    
    const Type getType() const
    {
        return type;
    }
    
    const Glyph getSpatialPatternAtStage (int stageIdx)
    {
        if (type != Type::spatial)
            std::cerr << "Calling getSpatialPatternAtStage on non-spatial QualityStep" << std::endl;
        
        return spatialStep->patternAtStage (stageIdx);
    }
    
    const MelodicNotes getIntelligibilityPatternAtStage (int stageIdx)
    {
        if (type != Type::intelligibility)
            std::cerr << "Calling getIntelligibilityPatternAtStage on non-intelligibility QualityStep" << std::endl;
        
        return intelligibilityStep->patternAtStage (stageIdx);
    }
    
    const HiddenPattern getPatternAtStage (int stageIdx)
    {
        if (type != Type::patterns)
            std::cerr << "Calling getPatternsAtStage on non-patterns QualityStep" << std::endl;
        
        return patternsStep->patternAtStage (stageIdx);
    }
    
    int getNumStages()
    {
        switch (type)
        {
            case Type::spatial:
                return spatialStep->getNumStages();
            case Type::intelligibility:
                return intelligibilityStep->getNumStages();
            case Type::patterns:
                return patternsStep->getNumStages();
        }
    }
    
private:
    QualityStep (Type type, std::optional<SpatialStep> spatialStep, std::optional<IntelligibilityStep> intelligibilityStep, std::optional<PatternsStep> patternsStep)
        : type (type), spatialStep (spatialStep), intelligibilityStep (intelligibilityStep), patternsStep (patternsStep)
    {
    }
    
    Type type;
    std::optional<SpatialStep> spatialStep;
    std::optional<IntelligibilityStep> intelligibilityStep;
    std::optional<PatternsStep> patternsStep;
};
