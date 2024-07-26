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
    
    float amplitudeCompensationGain = std::pow (0.59, std::log2(frequency / 1000.0f));    
    float amplitudeCompensationDB = juce::Decibels::gainToDecibels (amplitudeCompensationGain);
    
    // Introduce custom slope for clarity
    const float referenceFrequency = 1000.0;
    float slope = 1.7f;
    float octaves = std::log2((frequency) / (referenceFrequency));
    float dbDifference = octaves * slope;
    
    amplitudeCompensationDB += dbDifference;
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += amplitudeCompensationDB;
    
    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
    SequenceableNote note2 (Note (frequency, amplitude, pan, 0.0f), noteDurationInSamples);
    SequenceableNote spacingNote (1000.0f, 0.0f, 0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope (StereoGainEnvelopeType::SILENT));
    
    arbitrarySequencer.setNotes ({ note1, note2 }, true);
}

void SliderSequencer::playInterval (EQNode eqNode, int noteLength, bool repeating)
{
    playInterval (eqNode.frequency, eqNode.amplitude, eqNode.pan, noteLength, repeating);
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

void SliderSequencer::changeAmplitudeOfNotesWithFrequency (float frequency, float newAmplitude)
{
    arbitrarySequencer.changeNoteGainWithFrequency (frequency, newAmplitude);
}

void SliderSequencer::changePanOfNotesWithFrequency (float frequency, float newPan)
{
    arbitrarySequencer.changeNotePanWithFrequency (frequency, newPan);
}

std::pair<float, float> SliderSequencer::getNextSample()
{
    return arbitrarySequencer.getNextSample();
}

float SliderSequencer::currentlyPlayingFrequency() const
{
    return arbitrarySequencer.currentlyPlayingFrequency();
}

void SliderSequencer::setSampleRate (float newSampleRate)
{
    arbitrarySequencer.setSampleRate (newSampleRate);
}
