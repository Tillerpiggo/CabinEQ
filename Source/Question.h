/*
  ==============================================================================

    QuestionType.h
    Created: 1 Jul 2024 3:35:29pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "Note.h"

enum class QuestionType
{
    Level,
    Pan,
    Phase
};

class Question
{
public:
    Question (QuestionType type, Note note1, Note note2);
    
    const QuestionType getType() const;
    const float controlledFrequency() const; // return the frequency the user is calibrating/controlling w/ this questi
    static Question defaultQuestion(); // placeholder question because C++ is annoying with initialization
    
    const Note& getNote1() const;
    const Note& getNote2() const;
    const std::string lowerText() const; // text for the lowerPreferred button based on QuestionType
    const std::string higherText() const; // text for the higherPreferred button based on QuestionType
    
private:
    QuestionType type;
    Note note1;
    Note note2;
};
