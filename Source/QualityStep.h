/*
  ==============================================================================

    QualityStep.h
    Created: 30 Oct 2024 1:17:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "AudioPattern.h"
#include "DifficultyRange.h"

// This represents a single headphone test audio. It includes information to play the test, show the reference (what you're supposed to be listening for), and adjust the difficulty, for both spatial and intelligiblity based tests.
class QualityStep
{
public:
    enum class Type
    {
        spatial,
        intelligibility
    };
    
    QualityStep (Type type, AudioPattern testPattern, AudioPattern referencePattern, DifficultyRange difficultyRange)
        : type (type), testPattern (testPattern), referencePattern (referencePattern), difficultyRange (difficultyRange)
    {}
    
    const AudioPattern& getTestPattern() const
    {
        return testPattern;
    }
    
    const AudioPattern& getReferencePattern() const
    {
        return referencePattern;
    }
    
    const DifficultyRange& getDifficultyRange() const
    {
        return difficultyRange;
    }
    
private:
    Type type;
    AudioPattern testPattern;
    AudioPattern referencePattern;
    DifficultyRange difficultyRange;
};
