/*
  ==============================================================================

    Melody.h
    Created: 20 Jun 2024 12:31:31pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Note.h"

class Melody
{
public:
    Melody() : notes (std::vector<Note>()), noteIdx (0)  {}
    
    Note getCurrentNote () { return notes.at (noteIdx); }
    Note getNextNote ();
    std::vector<Note> notes;
    
    void addNote (Note note);
    
private:
    int noteIdx; // index of the next note to be played
};
