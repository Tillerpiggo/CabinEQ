/*
  ==============================================================================

    QuestionSequencer.h
    Created: 5 Jul 2024 4:52:44pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ArbitrarySequencer.h"
#include "Question.h"

class QuestionSequencer
{
public:
    QuestionSequencer();
    
    std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void setQuestion (Question question);
    
private:
    const std::vector<SequenceableNote> notesForQuestion (const Question& question) const;
    
    std::optional<Question> currQuestion;
    ArbitrarySequencer arbitrarySequencer;
};
