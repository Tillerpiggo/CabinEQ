/*
  ==============================================================================

    Melody.h
    Created: 20 Jun 2024 12:31:31pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Melody
{
public:
    Melody() {}
    const std::vector<int>& getNotes() const { return notes; }
    const double getControlledFrequency() const { return freqForNote (controlledNote); }
    
    void addNote (int note);
    void setControlledNote (int controlledNote) { this->controlledNote = controlledNote; }
    
private:
    double freqForNote(int note) const
    {
        return 440.0 * std::pow(2.0, (note - 69) / 12.0);
    }
    
    std::vector<int> notes; // like MIDI notes, but with arbitrary range
    int controlledNote = -1.0;
};
