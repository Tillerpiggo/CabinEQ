/*
  ==============================================================================

    Question.cpp
    Created: 2 Jul 2024 7:21:14pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Question.h"

Question::Question (QuestionType type, Note note1, Note note2) : type (type), note1 (note1), note2 (note2) {}

const QuestionType Question::getType() const
{
    return type;
}

const float Question::controlledFrequency() const
{
    return note2.frequency;
}

Question Question::defaultQuestion()
{
    Note note1 (1000.0f, 0.0f, 0.0f, 0.0f);
    Note note2 (440.0f, 0.0f, 0.0f, 0.0f);
    
    return Question (QuestionType::Level, note1, note2);
}
