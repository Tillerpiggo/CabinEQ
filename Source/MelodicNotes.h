/*
  ==============================================================================

    MelodicNotes.h
    Created: 28 Sep 2024 10:59:33pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SpatialPatternGenerator.h"

// This class helps to create melodic sequences for use by SpatialPatternGenerator
class MelodicNotes
{
public:
    MelodicNotes (std::vector<int> notesInSemitones, float centerFreq)
    : notesInSemitones (notesInSemitones), pans (notesInSemitones.size(), 0), bandwidth (2), centerFreq (centerFreq), noteDurationInSeconds (0.2), sampleRate (44100)
    {}
    
    MelodicNotes (std::vector<int> notesInSemitones, std::vector<float> pans, float bandwidth, float centerFreq, float noteDurationInSeconds, float sampleRate)
        : notesInSemitones (notesInSemitones), pans (pans), bandwidth (bandwidth), centerFreq (centerFreq), noteDurationInSeconds (noteDurationInSeconds), sampleRate (sampleRate)
    {}
    
    MelodicNotes withPans (std::vector<float> newPans)
    {
        return MelodicNotes (notesInSemitones, newPans, bandwidth, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withBandwidth (float newBandwidth)
    {
        return MelodicNotes (notesInSemitones, pans, newBandwidth, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withCenterFreq (float newCenterFreq)
    {
        return MelodicNotes (notesInSemitones, pans, bandwidth, newCenterFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withNoteDurationInSeconds (float newNoteDurationInSeconds)
    {
        return MelodicNotes (notesInSemitones, pans, bandwidth, centerFreq, newNoteDurationInSeconds, sampleRate);
    }
    
    std::vector<NoiseNote> noiseNotes()
    {
        std::vector<NoiseNote> noiseNotes;
        
        float semitoneRatio = std::pow(2.0f, 1.0f / 12.0f);
        int noteDurationInSamples = noteDurationInSeconds * sampleRate;
        
        // Assumes notesInSemitones.size() == pans.size()
        for (int i = 0; i < notesInSemitones.size(); ++i)
        {
            float noteFreq = centerFreq * std::pow (semitoneRatio, notesInSemitones[i]);
            noiseNotes.push_back (NoiseNote(noteFreq, bandwidth, noteDurationInSamples, pans[i], { 1.0f, 1.0f }, false));
        }
        
        return noiseNotes;
    }
    
private:
    std::vector<int> notesInSemitones;
    std::vector<float> pans;
    float bandwidth;
    float centerFreq;
    float noteDurationInSeconds;
    float sampleRate;
};
