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
#include "FractalStep.h"

// This class is a wrapper for a SpatialQualityStep and IntelligibilityQualityStep designed to make it easier to use them interchangeably.
class QualityStep
{
public:
    enum class Type
    {
        spatial,
        intelligibility,
        patterns,
        fractal
    };
    
    static QualityStep spatial (SpatialStep spatialStep)
    {
        return QualityStep (Type::spatial, spatialStep, std::nullopt, std::nullopt, std::nullopt);
    }
    
    static QualityStep intelligibility (IntelligibilityStep intelligibilityStep)
    {
        return QualityStep (Type::intelligibility, std::nullopt, intelligibilityStep, std::nullopt, std::nullopt);
    }
    
    static QualityStep patterns (PatternsStep patternsStep)
    {
        return QualityStep (Type::patterns, std::nullopt, std::nullopt, patternsStep, std::nullopt);
    }
    
    static QualityStep fractal (FractalStep fractalStep)
    {
        return QualityStep (Type::fractal, std::nullopt, std::nullopt, std::nullopt, fractalStep);
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
    
    const FractalPattern getFractalPatternAtStage (int stageIdx)
    {
        if (type != Type::fractal)
            std::cerr << "Calling getFractalPatternAtStage on non-spatial QualityStep" << std::endl;
        
        return fractalStep->patternAtStage (stageIdx);
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
            case Type::fractal:
                return fractalStep->getNumStages();
        }
    }
    
private:
    QualityStep (Type type, std::optional<SpatialStep> spatialStep, std::optional<IntelligibilityStep> intelligibilityStep, std::optional<PatternsStep> patternsStep, std::optional<FractalStep> fractalStep)
        : type (type), spatialStep (spatialStep), intelligibilityStep (intelligibilityStep), patternsStep (patternsStep), fractalStep (fractalStep)
    {
    }
    
    Type type;
    std::optional<SpatialStep> spatialStep;
    std::optional<IntelligibilityStep> intelligibilityStep;
    std::optional<PatternsStep> patternsStep;
    std::optional<FractalStep> fractalStep;
};
