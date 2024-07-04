/*
  ==============================================================================

    CalibrationIntervalSequencer.h
    Created: 1 Jul 2024 3:35:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SineWaveGenerator.h"

class CalibrationIntervalSequencer
{
public:
    CalibrationIntervalSequencer() {}
    
    const std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void setNotes (Note note1, Note note2);
    bool isPlayingFirstNote() const;
    
private:
    static const int NOTE_DURATION_IN_SAMPLES = 60000;
    
    std::optional<Note> note1;
    std::optional<Note> note2;
    SineWaveGenerator sineWaveGenerator;
    int numSamplesNoteHasBeenPlaying = 0;
    
    bool isFirstNotePlaying = true; // switched name order so C++ doesn't complain
};
