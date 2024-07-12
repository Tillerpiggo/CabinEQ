/*
  ==============================================================================

    SliderSequencer.cpp
    Created: 10 Jul 2024 1:05:40pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderSequencer.h"
#include <random>

void SliderSequencer::playInterval (float frequency, float amplitude, float pan)
{
    int noteDurationInSamples = 10000;
    
    SequenceableNote note1 (referenceNote, noteDurationInSamples);
    SequenceableNote note2 (Note (frequency, amplitude, pan, 0.0f), noteDurationInSamples);
    
    arbitrarySequencer.setNotes ({ note1, note2 });
}

void SliderSequencer::changeControlledAmplitude (float newAmplitude)
{
    // change arbitrarysequencer to be able to dynamically change volume of note at index
    arbitrarySequencer.changeNoteGainAtIdx (1, newAmplitude);
}

void SliderSequencer::changeControlledPan (float newPan)
{
    arbitrarySequencer.changeNotePanAtIdx (1, newPan);
}

void SliderSequencer::playRandomNotes (std::vector<float> frequencies, std::vector<float> amplitudes, std::vector<float> pans)
{
    std::vector<SequenceableNote> notes;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int> (frequencies.size() - 1));
    
    size_t numNotes = frequencies.size() * 4;

    for (int i = 0; i < numNotes; ++i)
    {
        int index = dis(gen);
        float frequency = frequencies.at (index);
        float amplitude = amplitudes.at (index);
        float pan = pans.at (index);
        
        // Create and add the note to the sequence
        SequenceableNote note (frequency, amplitude, pan, 0.0f, 20000, StereoGainEnvelope());
        SequenceableNote spacingNote (frequency, 0.0f, 0.0f, 0.0f, 10000,
                                      StereoGainEnvelope (StereoGainEnvelopeType::SILENT));
        notes.push_back (note);
        notes.push_back (spacingNote);
    }
    
    arbitrarySequencer.setNotes (notes);
}

void SliderSequencer::changeAmplitudeOfNotesWithFrequency (float frequency, float newAmplitude)
{
    arbitrarySequencer.changeNoteGainWithFrequency (frequency, newAmplitude);
}

void SliderSequencer::changePanOfNotesWithFrequency (float frequency, float newPan)
{
    arbitrarySequencer.changeNotePanWithFrequency (frequency, newPan);
}
