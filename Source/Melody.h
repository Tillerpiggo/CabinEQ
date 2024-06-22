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
    
    static double freqForNote (int note)
    {
        return 440.0 * std::pow (2.0, (note - 69) / 12.0);
    }
    
private:
    
    
    std::vector<int> notes; // like MIDI notes, but with arbitrary range
    int controlledNote = -1.0;
};
