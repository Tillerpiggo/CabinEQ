/*
  ==============================================================================

    Melody.cpp
    Created: 20 Jun 2024 12:31:31pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Melody.h"

Note Melody::getNextNote()
{
    Note nextNote = notes.at (noteIdx);
    
    // Wrap around melody to loop it
    if (noteIdx >= notes.size())
    {
        noteIdx = 0;
    }
    
    return nextNote;
}
