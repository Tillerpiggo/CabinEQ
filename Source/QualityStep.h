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

// This class is a wrapper for a SpatialQualityStep and IntelligibilityQualityStep designed to make it easier to use them interchangeably.
class QualityStep
{
public:
    enum class Type
    {
        spatial,
        intelligibility
    };
    
    static QualityStep spatial (SpatialStep spatialStep)
    {
        return QualityStep (Type::spatial, spatialStep, std::nullopt);
    }
    
    static QualityStep intelligibility (IntelligibilityStep intelligibilityStep)
    {
        return QualityStep (Type::intelligibility, std::nullopt, intelligibilityStep);
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
    
private:
    QualityStep (Type type, std::optional<SpatialStep> spatialStep, std::optional<IntelligibilityStep> intelligibilityStep)
        : type (type), spatialStep (spatialStep), intelligibilityStep (intelligibilityStep)
    {
    }
    
    Type type;
    std::optional<SpatialStep> spatialStep;
    std::optional<IntelligibilityStep> intelligibilityStep;
};

//// This represents a single headphone test audio. It includes information to play the test, show the reference (what you're supposed to be listening for), and adjust the difficulty, for both spatial and intelligiblity based tests.
//class QualityStep
//{
//public:
//    enum class Type
//    {
//        spatial,
//        intelligibility
//    };
//
//    QualityStep (Type type, AudioPattern testPattern, DifficultyRange difficultyRange)
//        : type (type), testPattern (testPattern), difficultyRange (difficultyRange)
//    {}
//
//    const AudioPattern& getAudioPattern() const
//    {
//        return testPattern;
//    }
//
//    const DifficultyRange& getDifficultyRange() const
//    {
//        return difficultyRange;
//    }
//
//private:
//    Type type;
//    AudioPattern testPattern;
//    DifficultyRange difficultyRange;
//};
