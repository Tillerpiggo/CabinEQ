/*
  ==============================================================================

    SliderSequencer.h
    Created: 10 Jul 2024 1:05:40pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ArbitrarySequencer.h"
#include "Note.h"

class SliderSequencer
{
public:
    SliderSequencer() {};
    
    void playInterval (float frequency, float amplitude, float pan, int noteLength);
    void playTwoToneInterval (float frequency1, float amplitude1, float pan1,
                              float frequency2, float amplitude2, float pan2); // TODO - give this dynamic tempo
    void playIntervalAndLastNote (float frequency, float amplitude, float pan,
                                  float lastFrequency, float lastAmplitude, float lastPan);
    void playComparisonFrequencies (float frequency, float amplitude, float pan,
                                    std::vector<float> frequencies,
                                    std::vector<float> amplitudes,
                                    std::vector<float> pans);
    void changeControlledAmplitude (float newAmplitude);
    void changeControlledPan (float newPan);
    
    void playTuningNotes(std::vector<float> frequencies, std::vector<float> amplitudes, std::vector<float> pans);
    void playRandomNotes (std::vector<float> frequencies, std::vector<float> amplitudes, std::vector<float> pans);
    void changeAmplitudeOfNotesWithFrequency (float frequency, float newAmplitude);
    void changePanOfNotesWithFrequency (float frequency, float newPan);
    
    void setReferenceNote (float frequency, float amplitude);
    
    std::pair<float, float> getNextSample() 
    {
        return arbitrarySequencer.getNextSample();
    }
    
    float currentlyPlayingFrequency() const
    {
        return arbitrarySequencer.currentlyPlayingFrequency();
    }
    
    void setSampleRate (float newSampleRate) 
    {
        arbitrarySequencer.setSampleRate (newSampleRate);
    }
    
private:
    ArbitrarySequencer arbitrarySequencer;
    Note referenceNote = Note (1000.0f, 6.0f, 0.0f, 0.0f);
};
