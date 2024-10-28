/*
  ==============================================================================

    ArbitrarySequencer.cpp
    Created: 5 Jul 2024 4:43:07pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ArbitrarySequencer.h"

ArbitrarySequencer::ArbitrarySequencer (std::unique_ptr<PitchedGenerator> pitchedGenerator)
    : pitchedGenerator (std::move (pitchedGenerator)), currNoteIdx(0), numSamplesNoteHasBeenPlaying(0), listener (nullptr)
{}

std::pair<float, float> ArbitrarySequencer::getNextSample()
{
    if (currNoteIdx < 0 || currNoteIdx >= notes.size()) return { 0.0f, 0.0f };
    
    SequenceableNote currNote = notes.at (currNoteIdx);
    
    auto [leftSample, rightSample] = pitchedGenerator->getNextSample();
    auto [leftGain, rightGain] = currNote.getGainAtSample (numSamplesNoteHasBeenPlaying);
    
    numSamplesNoteHasBeenPlaying++;
    if (numSamplesNoteHasBeenPlaying >= currNote.getDuration())
    {
        goToNextNote();
    }
    
    return { leftSample * leftGain, rightSample * rightGain };
}

bool ArbitrarySequencer::isPlayingFirstNote() const
{
    return currNoteIdx == 0;
}

float ArbitrarySequencer::currentlyPlayingFrequency() const
{
    if (currNoteIdx < 0 || currNoteIdx >= notes.size()) return -1;
    return notes.at (currNoteIdx).getFrequency();
}

void ArbitrarySequencer::setSampleRate (float newSampleRate)
{
    pitchedGenerator->setSampleRate (newSampleRate);
}

void ArbitrarySequencer::setNotes (const std::vector<SequenceableNote>& newNotes, bool repeating)
{
    notes = newNotes;
    currNoteIdx = 0;
    numSamplesNoteHasBeenPlaying = 0;
    
    pitchedGenerator->setNote (getCurrNote().getNote());
    
    this->isRepeating = repeating;
}

void ArbitrarySequencer::setNotes (const std::vector<float>& notesInSemitones, float freq, bool repeating)
{
    std::vector<SequenceableNote> sequenceableNotes;
    float semitoneRatio = std::pow (2.0f, 1.0f / 12.0f);
    for (const auto& semitones : notesInSemitones)
    {
        float noteFreq = freq * std::pow (semitoneRatio, semitones);
        sequenceableNotes.push_back (SequenceableNote (noteFreq, 0.0, 0.0, 0.0, 7500));
    }
    setNotes (sequenceableNotes, repeating);
}

void ArbitrarySequencer::setNotes (const std::vector<float>& notesInSemitones, float freq, int noteDurationInSamples, bool repeating)
{
    std::vector<SequenceableNote> sequenceableNotes;
    float semitoneRatio = std::pow (2.0f, 1.0f / 12.0f);
    for (const auto& semitones : notesInSemitones)
    {
        float noteFreq = freq * std::pow (semitoneRatio, semitones);
        sequenceableNotes.push_back (SequenceableNote (noteFreq, 0.0, 0.0, 0.0, noteDurationInSamples));
    }
    setNotes (sequenceableNotes, repeating);
}

void ArbitrarySequencer::setNotesForSpatialCalibration (const std::vector<SequenceableNote>& notes, bool repeating)
{
    auto spatialNotes = getNotesForSpatialCalibration (notes);
    setNotes (spatialNotes, repeating);
}

void ArbitrarySequencer::setListener(SequencerListener* newListener)
{
    listener = newListener;
}

void ArbitrarySequencer::updateNotes (const std::vector<SequenceableNote>& newNotes)
{
    for (int i = 0; i < newNotes.size(); ++i)
    {
        changeNoteAtIdx (i, newNotes[i]);
    }
}

void ArbitrarySequencer::updateNotes (const std::vector<float>& newNotesInSemitones, float freq)
{
    std::vector<SequenceableNote> sequenceableNotes;
    float semitoneRatio = std::pow (2.0f, 1.0f / 12.0f);
    for (const auto& semitones : newNotesInSemitones)
    {
        float noteFreq = freq * std::pow (semitoneRatio, semitones);
        sequenceableNotes.push_back (SequenceableNote (noteFreq, 0.0, 0.0, 0.0, 10000));
    }
    
    for (int i = 0; i < sequenceableNotes.size(); ++i)
    {
        changeNoteAtIdx (i, sequenceableNotes[i]);
    }
}

void ArbitrarySequencer::updateNotesForSpatialCalibration (const std::vector<SequenceableNote>& newNotes)
{
    auto spatialNotes = getNotesForSpatialCalibration (newNotes);
    for (int i = 0; i < spatialNotes.size(); ++i)
    {
        changeNoteAtIdx (i, spatialNotes[i]);
    }
}

void ArbitrarySequencer::changeNoteAtIdx (int idx, Note newNote)
{
    if (idx < 0 || idx >= notes.size())
    {
        std::cerr << "WARNING: changing note gain at idx out of bounds" << std::endl;
        return;
    }
    
    notes.at (idx).setFrequency (newNote.frequency);
    notes.at (idx).setAmplitude (newNote.amplitude);
    notes.at (idx).setPan (newNote.pan);
}

void ArbitrarySequencer::changeNoteAtIdx (int idx, SequenceableNote newNote)
{
    if (idx < 0 || idx >= notes.size())
    {
        std::cerr << "WARNING: changing note gain at idx out of bounds" << std::endl;
        return;
    }
    notes.at (idx).setFrequency (newNote.getFrequency());
    notes.at (idx).setAmplitude (newNote.getAmplitude());
    notes.at (idx).setPan (newNote.getPan());
    notes.at (idx).setDuration (newNote.getDuration());
}

void ArbitrarySequencer::changeNoteGainAtIdx (int idx, float noteGain)
{
    if (idx < 0 || idx >= notes.size())
    {
        std::cerr << "WARNING: changing note gain at idx out of bounds" << std::endl;
        return;
    }
    
    notes.at (idx).setAmplitude(noteGain);
    if (currNoteIdx == idx)
    {
        pitchedGenerator->setVolume (noteGain);
    }
}

void ArbitrarySequencer::changeNoteGainWithFrequency (float frequency, float noteGain)
{
    for (int i = 0; i < notes.size(); ++i)
    {
        if (notes.at (i).getFrequency() == frequency)
        {
            changeNoteGainAtIdx (i, noteGain);
        }
    }
}

float ArbitrarySequencer::getCurrFreq()
{
    return getCurrNote().getFrequency();
}

void ArbitrarySequencer::goToNextNote()
{
    numSamplesNoteHasBeenPlaying = 0;
    currNoteIdx++;
    
    if (currNoteIdx >= notes.size() && isRepeating)
    {
        currNoteIdx = 0;
        notifyListener();
    }
    
    Note nextNote = getCurrNote().getNote();
    pitchedGenerator->setNote (nextNote);
}

const SequenceableNote& ArbitrarySequencer::getCurrNote() const
{
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        std::cerr << "ERROR IN ArbitrarySequencer - getting curr note before assigning any notes" << std::endl;
    return notes.at (currNoteIdx);
}

void ArbitrarySequencer::notifyListener()
{
    if (listener != nullptr)
    {
        listener->sequenceDidFinish();
    }
}

std::vector<SequenceableNote> ArbitrarySequencer::getNotesForSpatialCalibration (const std::vector<SequenceableNote>& notes)
{
    std::vector<SequenceableNote> spatialNotes;
//    std::vector<float> pans { -1, -0.3, 0.3, 1 };
//    std::vector<float> pans { -1, -0.3, 0.3, 1 };
    std::vector<float> pans { -1, -0.5, 0, 0.5, 1 };
//    std::vector<float> pans { 1, 0.3, -0.3, -1 };
    std::vector<float> ampls { -6.0f, 0.0f, 6.0f, 0.0f };
//    std::vector<float> ampls { 6.0f, -30.0f, 6.0f, -30.0f };
//    std::vector<float> ampls;
//    for (int i = 0; i < 12; ++i)
//    {
//        ampls.push_back (i - 6.0f);
//    }
//    
//    for (int i = 0; i < 12; ++i)
//    {
//        ampls.push_back (6.0f - i);
//    }
    
    for (const auto& note : notes)
    {
//        for (const auto& ampl : ampls)
//        {
//            spatialNotes.emplace_back (note.withAmplitudeChange (ampl));
////            for (const auto& ampl : ampls)
////            {
////                spatialNotes.emplace_back (note.withAmplitudeChange (ampl).withPan (pan));
////            }
//        }
        
        for (const auto& pan : pans)
        {
            spatialNotes.emplace_back (note.withPan (pan));
        }
    }
    
//    for (const auto& ampl : ampls)
//    {
//
//        
//        for (const auto& note : notes)
//        {
//            spatialNotes.emplace_back ( note.withAmplitudeChange (ampl));
//        }
//    }
    
//    for (const auto& note : notes)
//    {
//        // Create a note for each pan
//        for (const auto& pan : pans)
//        {
//            spatialNotes.emplace_back (note.withPan (pan));
//        }
//    }
    
    
    return spatialNotes;
}
