/*
  ==============================================================================

    NoteSequence.h
    Created: 22 Jun 2024 11:08:29am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "Note.h"

class NoteSequence
{
public:
    NoteSequence() : notes (std::vector<Note>()), noteIdx (0)  {}
    
    Note getCurrentNote() { return notes.at (noteIdx); }
    Note getNextNote();
    int getNoteIdx() { return noteIdx; }
    std::vector<Note> notes;
    
    void addNote (Note note);
    
private:
    int noteIdx; // index of the next note to be played
};
