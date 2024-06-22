/*
  ==============================================================================

    NoteSequence.cpp
    Created: 22 Jun 2024 11:08:29am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "NoteSequence.h"

Note NoteSequence::getNextNote()
{
    Note nextNote = notes.at (noteIdx);
    noteIdx++;
    
    // Wrap around to loop sequence
    if (noteIdx >= notes.size())
    {
        noteIdx = 0;
    }
    
    return nextNote;
}

void NoteSequence::addNote (Note note)
{
    notes.push_back (note);
}
