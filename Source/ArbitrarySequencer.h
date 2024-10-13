///*
//  ==============================================================================
//
//    ArbitrarySequencer.h
//    Created: 5 Jul 2024 4:43:07pm
//    Author:  Tyler Gee
//
//  ==============================================================================
//*/
//
//#pragma once
//
//#include <JuceHeader.h>
//#include "SequenceableNote.h"
//#include "SineWaveGenerator.h"
//#include "PitchedGenerator.h"
//
//class SequencerListener
//{
//public:
//    virtual ~SequencerListener() = default;
//    virtual void sequenceDidFinish() = 0;
//};
//
//class ArbitrarySequencer
//{
//public:
//    ArbitrarySequencer (std::unique_ptr<PitchedGenerator> pitchedGenerator);
//    
//    std::pair<float, float> getNextSample();
//    bool isPlayingFirstNote() const;
//    float currentlyPlayingFrequency() const;
//    
//    void setSampleRate (float newSampleRate);
//    void setNotes (const std::vector<SequenceableNote>& notes, bool repeating = true);
//    void setNotes (const std::vector<float>& notesInSemitones, float freq, Curve& amplCurve, bool repeating = true);
//    void setNotesForSpatialCalibration (const std::vector<SequenceableNote>& notes, bool repeating = true); // plays each note repeated, from 5 different angles (hard right, soft right, center, soft left, hard left)
//    void setListener (SequencerListener* newListener);
//    
//    void updateNotes (const std::vector<SequenceableNote>& newNotes);
//    void updateNotes (const std::vector<float>& newNotesInSemitones, float freq, Curve& amplCurve);
//    void updateNotesForSpatialCalibration (const std::vector<SequenceableNote>& newNotes);
//    
//    void changeNoteAtIdx (int idx, Note newNote);
//    void changeNoteAtIdx (int idx, SequenceableNote newNote);
//    void changeNoteGainAtIdx (int idx, float noteGain);
//    void changeNotePanAtIdx (int idx, float notePan);
//    void changeNoteGainWithFrequency (float frequency, float noteGain);
//    void changeNotePanWithFrequency (float frequency, float notePan);
//    
//private:
//    void goToNextNote();
//    const SequenceableNote& getCurrNote() const;
//    void notifyListener();
//    std::vector<SequenceableNote> getNotesForSpatialCalibration (const std::vector<SequenceableNote>& notes);
//    
//    std::unique_ptr<PitchedGenerator> pitchedGenerator;
//    std::vector<SequenceableNote> notes;
//    
//    int currNoteIdx;
//    int numSamplesNoteHasBeenPlaying;
//    SequencerListener* listener;
//    
//    bool isRepeating = true;
//};
