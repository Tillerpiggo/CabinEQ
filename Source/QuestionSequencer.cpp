/*
  ==============================================================================

    QuestionSequencer.cpp
    Created: 5 Jul 2024 4:52:44pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "QuestionSequencer.h"

QuestionSequencer::QuestionSequencer() {}

std::pair<float, float> QuestionSequencer::getNextSample()
{
    if (! currQuestion.has_value())
    {
        std::cerr << "WARNING: Get next sample called on QuestionSequencer before Question was set" << std::endl;
        return { 0.0f, 0.0f };
    }
    
    auto [leftSample, rightSample] = arbitrarySequencer.getNextSample();
    std::cout << "leftsample: " << leftSample << ", rightSample: " << rightSample << std::endl;
    
    return { leftSample, rightSample };
}

void QuestionSequencer::setSampleRate (float newSampleRate)
{
    arbitrarySequencer.setSampleRate (newSampleRate);
}

void QuestionSequencer::setQuestion (Question question)
{
    currQuestion = question;
    arbitrarySequencer.setNotes (notesForQuestion (currQuestion.value()));
}

const std::vector<SequenceableNote> QuestionSequencer::notesForQuestion (const Question& question) const
{
    std::vector<SequenceableNote> notes;
    switch (question.getType())
    {
        case QuestionType::Level:
        {
            // Simple two-tone test
            int noteDurationInSamples = 40000;
            
            notes.push_back (SequenceableNote (question.getNote1(),
                                               noteDurationInSamples));
            notes.push_back (SequenceableNote (question.getNote2(),
                                               noteDurationInSamples));
            return notes;
        }
        case QuestionType::ReferencePan:
            [[fallthrough]];
        case QuestionType::Pan:
        {
            // Hard left, hard right, center, then controlled tone
            // Simple two-tone test
            int noteDurationInSamples = 20000;
            int leftRightIntroNoteDurationInSamples = 1000;
            int rampDurationInSamples = 300;
            
            notes.push_back (SequenceableNote (question.getNote1(),
                                               leftRightIntroNoteDurationInSamples,
                                               StereoGainEnvelope (StereoGainEnvelopeType::HARD_LEFT,
                                                                   rampDurationInSamples)));
            notes.push_back (SequenceableNote (question.getNote1(),
                                               leftRightIntroNoteDurationInSamples,
                                               StereoGainEnvelope (StereoGainEnvelopeType::HARD_RIGHT,
                                                                   rampDurationInSamples)));
            notes.push_back (SequenceableNote (question.getNote1(),
                                               noteDurationInSamples,
                                               StereoGainEnvelope (rampDurationInSamples)));
            notes.push_back (SequenceableNote (question.getNote2(),
                                               noteDurationInSamples,
                                               StereoGainEnvelope (rampDurationInSamples)));
            return notes;
        }
        case QuestionType::ReferencePhase:
            [[fallthrough]];
        case QuestionType::Phase:
        {
            int noteDurationInSamples = 70000;
            int spaceBetweenNotesInSamples = 20000;
            int rightDelayInSamples = 0; // I don't think this is really helping much
            int rampDurationInSamples = 2000;
            
            SequenceableNote silentNote (question.getNote1(),
                                         spaceBetweenNotesInSamples,
                                         StereoGainEnvelope (StereoGainEnvelopeType::SILENT,
                                                             rampDurationInSamples));
            
            notes.push_back (SequenceableNote (question.getNote1(),
                                               noteDurationInSamples,
                                               StereoGainEnvelope (rampDurationInSamples,
                                                                   rightDelayInSamples)));
            notes.push_back (silentNote);
            notes.push_back (SequenceableNote (question.getNote2(),
                                               noteDurationInSamples,
                                               StereoGainEnvelope (rampDurationInSamples,
                                                                   rightDelayInSamples)));
            notes.push_back (silentNote);
            
            return notes;
        }
        case QuestionType::HigherThan:
        {
            // Simple two-tone test
            int noteDurationInSamples = 15000;
            
            notes.push_back (SequenceableNote (question.getNote1(),
                                               noteDurationInSamples));
            notes.push_back (SequenceableNote (question.getNote2(),
                                               noteDurationInSamples));
            return notes;
        }
    }
}
