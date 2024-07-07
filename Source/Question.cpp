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

const Note& Question::getNote1() const
{
    return note1;
}

const Note& Question::getNote2() const
{
    return note2;
}

const std::string Question::lowerText() const
{
    switch (type)
    {
        case QuestionType::Level:
            return "This one's louder!";
        case QuestionType::Pan:
            return "Left";
        case QuestionType::Phase:
            return "First More Sharp"; // ?? might need to change
        case QuestionType::LowerThan:
            return "Decrease Volume";
        case QuestionType::HigherThan:
            return "Increase Volume";
        case QuestionType::ReferencePan:
            return "First More Centered";
        case QuestionType::ReferencePhase:
            return "First More Sharp";
    }
}

const std::string Question::higherText() const
{
    switch (type)
    {
        case QuestionType::Level:
            return "This one's louder!";
        case QuestionType::Pan:
            return "Right";
        case QuestionType::Phase:
            return "Second More Sharp"; // ?? same as above, might need to change
        case QuestionType::LowerThan:
            [[fallthrough]];
        case QuestionType::HigherThan:
            return "Submit";
        case QuestionType::ReferencePan:
            return "Second More Centered";
        case QuestionType::ReferencePhase:
            return "Second More Sharp";
    }
}

void Question::increaseControlledNoteVolume()
{
    note2.gain += 3.0f;
}

void Question::decreaseControlledNoteVolume()
{
    note2.gain -= 3.0f;
}
