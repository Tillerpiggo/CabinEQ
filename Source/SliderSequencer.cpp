/*
  ==============================================================================

    SliderSequencer.cpp
    Created: 10 Jul 2024 1:05:40pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderSequencer.h"
#include <random>

void SliderSequencer::playInterval (float frequency, float amplitude, float pan, int noteLength, bool repeating)
{
    int noteDurationInSamples = noteLength;
    
    float amplitudeCompensationGain = std::pow (0.64, std::log2(frequency / 1000.0f));
    if (frequency < 60)
    {
        amplitudeCompensationGain /= std::pow (0.64, std::log2(frequency / 60.0f));
        amplitudeCompensationGain /= std::pow (0.5, std::log2(frequency / 60.0f));
    }
    if (frequency > 10000)
    {
        amplitudeCompensationGain *= std::pow (0.5, std::log2(frequency / 10000.0f));
    }
    
    
    float amplitudeCompensationDB = juce::Decibels::gainToDecibels (amplitudeCompensationGain);
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += amplitudeCompensationDB;
    
    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
    SequenceableNote note2 (Note (frequency, amplitude, pan, 0.0f), noteDurationInSamples);
    
    arbitrarySequencer.setNotes ({ note1, note2, note1 }, repeating);
}

void SliderSequencer::playTwoToneInterval (float frequency1, float amplitude1, float pan1,
                                           float frequency2, float amplitude2, float pan2)
{
    int noteDurationInSamples = 20000;
    
    std::vector<SequenceableNote> notes;
    for (int i = 0; i < 5; ++i)
    {
        SequenceableNote note1 (Note (frequency1, amplitude1, pan1, 0.0f), noteDurationInSamples + 10000 * i);
        SequenceableNote note2 (Note (frequency2, amplitude2, pan2, 0.0f), noteDurationInSamples + 10000 * i);
        
        for (int j = 0; j < 4; ++j)
        {
            notes.push_back (note1);
            notes.push_back (note2);
        }
    }
    
    arbitrarySequencer.setNotes (notes);
}

void SliderSequencer::playIntervalAndLastNote (float frequency, float amplitude, float pan,
                                               float lastFrequency, float lastAmplitude, float lastPan)
{
    int noteDurationInSamples = 10000;
    
    SequenceableNote refNote (referenceNote, noteDurationInSamples);
    SequenceableNote lastNote (Note (lastFrequency, lastAmplitude, lastPan, 0.0f), noteDurationInSamples);
    SequenceableNote currNote (Note (frequency, amplitude, pan, 0.0f), noteDurationInSamples);
    
    arbitrarySequencer.setNotes ({ currNote, refNote, lastNote, refNote });
}

void SliderSequencer::playComparisonFrequencies (float frequency, float amplitude, float pan,
                                std::vector<float> frequencies,
                                std::vector<float> amplitudes,
                                std::vector<float> pans)
{
    std::vector<SequenceableNote> notes;
    
    int noteDurationInSamples = 20000;
    SequenceableNote baseNote (Note (frequency, amplitude, pan, 0.0f), noteDurationInSamples);
    
    for (int i = 0; i < frequencies.size(); ++i)
    {
        notes.push_back (SequenceableNote (frequencies[i], amplitudes[i], pans[i], 0.0f, noteDurationInSamples, StereoGainEnvelope()));
        notes.push_back (baseNote);
    }
    
    arbitrarySequencer.setNotes (notes);
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

void SliderSequencer::playTuningNotes(std::vector<float> frequencies, std::vector<float> amplitudes, std::vector<float> pans)
{
    std::vector<SequenceableNote> notes;

    // Ensure we only iterate as many times as there are frequencies
    size_t numNotes = frequencies.size() * 4;
    size_t index = 0;

    for (int i = 0; i < numNotes; ++i)
    {
        // Get the current index and wrap around if necessary
        size_t currentIndex = index % frequencies.size();
        
        float frequency = frequencies.at(currentIndex);
        float amplitude = amplitudes.at(currentIndex);
        float pan = pans.at(currentIndex);
        
        // Create and add the note to the sequence
        SequenceableNote note(frequency, amplitude, pan, 0.0f, 20000, StereoGainEnvelope (5000));
        notes.push_back(note);

        // Increment the index
        index++;
    }

    arbitrarySequencer.setNotes(notes);
}

void SliderSequencer::playRandomNotes(std::vector<float> frequencies, std::vector<float> amplitudes, std::vector<float> pans)
{
    std::vector<SequenceableNote> notes;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(frequencies.size() - 1));
    
    size_t numNotes = frequencies.size() * 4;
    int lastIndex = -1;

    for (int i = 0; i < numNotes; ++i)
    {
        int index;
        do {
            index = dis(gen);
        } while (index == lastIndex);
        
        lastIndex = index;

        float frequency = frequencies.at(index);
        float amplitude = amplitudes.at(index);
        float pan = pans.at(index);
        
        // Create and add the note to the sequence
        SequenceableNote note(frequency, amplitude, pan, 0.0f, 20000, StereoGainEnvelope());
        notes.push_back(note);
    }
    
    arbitrarySequencer.setNotes(notes);
}

void SliderSequencer::changeAmplitudeOfNotesWithFrequency (float frequency, float newAmplitude)
{
    arbitrarySequencer.changeNoteGainWithFrequency (frequency, newAmplitude);
}

void SliderSequencer::changePanOfNotesWithFrequency (float frequency, float newPan)
{
    arbitrarySequencer.changeNotePanWithFrequency (frequency, newPan);
}

void SliderSequencer::setReferenceNote (float frequency, float amplitude)
{
//    referenceNote.frequency = frequency;
//    referenceNote.gain = amplitude;
}
