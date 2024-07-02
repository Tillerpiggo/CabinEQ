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
    Question (QuestionType type, Note note1, Note note2)
    : type (type), note1 (note1), note2 (note2) {}
    
    const QuestionType getType() const;
private:
    QuestionType type;
    Note note1;
    Note note2;
};
