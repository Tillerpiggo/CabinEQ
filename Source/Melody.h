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
    Melody() : noteIdx (0), notes (std::vector<Note>()) {}
    
    Note getCurrentNote () { return notes.at (noteIdx) };
    Note getNextNote ();
    std::vector<Note> notes;
    
private:
    int noteIdx; // index of the next note to be played
};
