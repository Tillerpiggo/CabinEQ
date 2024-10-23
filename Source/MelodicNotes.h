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
#include "SequenceableNote.h"

// This class helps to create melodic sequences for use by SpatialPatternGenerator
class MelodicNotes
{
public:
    MelodicNotes (std::vector<float> notesInSemitones, float centerFreq)
    : notesInSemitones (notesInSemitones), pans (notesInSemitones.size(), 0), bandwidths (notesInSemitones.size(), 2), ampls (notesInSemitones.size(), 0), centerFreq (centerFreq), noteDurationInSeconds (0.2), sampleRate (44100)
    {}
    
    MelodicNotes (std::vector<float> notesInSemitones, std::vector<float> pans, std::vector<float> bandwidths, std::vector<float> ampls, float centerFreq, float noteDurationInSeconds, float sampleRate)
        : notesInSemitones (notesInSemitones), pans (pans), bandwidths (bandwidths), ampls (ampls), centerFreq (centerFreq), noteDurationInSeconds (noteDurationInSeconds), sampleRate (sampleRate)
    {}
    
    static MelodicNotes withFreqs (std::vector<float> freqs)
    {
        std::vector<float> notesInSemitones;
        float centerFreq = freqs[0];
        for (const auto& freq : freqs)
        {
            float semitonesFromCenterFreq = 12.0f * std::log2 (freq / centerFreq);
            notesInSemitones.push_back (semitonesFromCenterFreq);
        }
        
        return MelodicNotes (notesInSemitones, centerFreq);
    }
    
    static MelodicNotes withPattern (std::vector<bool> hits, float freq, float bandwidth)
    {
        std::vector<float> notesInSemitones;
        std::vector<float> pans;
        std::vector<float> bandwidths;
        std::vector<float> ampls;
        float noteDurationInSeconds = 0.1;
        for (const auto hit : hits)
        {
            notesInSemitones.push_back (0);
            pans.push_back (0);
            bandwidths.push_back (hit ? bandwidth : 0.0);
            ampls.push_back (0);
        }
        
        return MelodicNotes (notesInSemitones, pans, bandwidths, ampls, freq, noteDurationInSeconds, 44100); // todo, make sample rate legit
    }
    
    static MelodicNotes withMelodicPattern (std::vector<bool> hits, std::vector<float> noteFreqs, float freq, float bandwidth, std::vector<float> notePans)
    {
        std::vector<float> notesInSemitones;
        std::vector<float> pans;
        std::vector<float> bandwidths;
        std::vector<float> ampls;
        float noteDurationInSeconds = 0.1;
        int noteIdx = 0;
        
        for (int i = 0; i < hits.size() * noteFreqs.size() * notePans.size(); ++i)
        {
            if (hits[i % hits.size()])
            {
                notesInSemitones.push_back (noteFreqs[noteIdx % noteFreqs.size()]);
                pans.push_back (notePans[noteIdx % notePans.size()]);
                bandwidths.push_back (bandwidth);
                ampls.push_back (0);
                noteIdx++;
            }
            else
            {
                notesInSemitones.push_back (0);
                pans.push_back (0);
                bandwidths.push_back (0);
                ampls.push_back (0);
            }
        }
        
        return MelodicNotes (notesInSemitones, pans, bandwidths, ampls, freq, noteDurationInSeconds, 44100); // todo - include actual sample rate
    }
    
    static MelodicNotes withMelodicPattern (std::vector<bool> hits, std::vector<float> noteFreqs, float bandwidth, std::vector<float> notePans)
    {
        std::vector<float> notesInSemitones;
        std::vector<float> pans;
        std::vector<float> bandwidths;
        std::vector<float> ampls;
        float noteDurationInSeconds = 0.1;
        int noteIdx = 0;
        
        std::vector<float> relativeNoteFreqs;
        float centerFreq = noteFreqs[0];
        for (const auto& freq : noteFreqs)
        {
            float semitonesFromCenterFreq = 12.0f * std::log2 (freq / centerFreq);
            relativeNoteFreqs.push_back (semitonesFromCenterFreq);
        }
        
        for (int i = 0; i < hits.size() * noteFreqs.size() * notePans.size(); ++i)
        {
            if (hits[i % hits.size()])
            {
                notesInSemitones.push_back (relativeNoteFreqs[noteIdx % noteFreqs.size()]);
                pans.push_back (notePans[noteIdx % notePans.size()]);
                bandwidths.push_back (bandwidth);
                ampls.push_back (0);
                noteIdx++;
            }
            else
            {
                notesInSemitones.push_back (0);
                pans.push_back (0);
                bandwidths.push_back (0);
                ampls.push_back (0);
            }
        }
        
        return MelodicNotes (notesInSemitones, pans, bandwidths, ampls, centerFreq, noteDurationInSeconds, 44100); // todo - include actual sample rate
    }
    
    MelodicNotes withPans (std::vector<float> newPans)
    {
        return MelodicNotes (notesInSemitones, newPans, bandwidths, ampls, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withBandwidth (float newBandwidth)
    {
        return MelodicNotes (notesInSemitones, pans, std::vector<float> (notesInSemitones.size(), newBandwidth), ampls, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withBandwidths (std::vector<float> newBandwidths)
    {
        return MelodicNotes (notesInSemitones, pans, newBandwidths, ampls, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withCenterFreq (float newCenterFreq)
    {
        return MelodicNotes (notesInSemitones, pans, bandwidths, ampls, newCenterFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withNoteDurationInSeconds (float newNoteDurationInSeconds)
    {
        return MelodicNotes (notesInSemitones, pans, bandwidths, ampls, centerFreq, newNoteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withPanCopies (std::vector<float> panCopies)
    {
        std::vector<float> newNotesInSemitones;
        std::vector<float> newBandwidths;
        std::vector<float> newPans;
        std::vector<float> newAmpls;
        for (const auto& panCopy : panCopies)
        {
            for (int i = 0; i < notesInSemitones.size(); ++i)
            {
                newNotesInSemitones.push_back (notesInSemitones[i]);
                newBandwidths.push_back (bandwidths[i]);
                newPans.push_back (panCopy);
                newAmpls.push_back (ampls[i]);
            }
        }
        
        return MelodicNotes (newNotesInSemitones, newPans, newBandwidths, newAmpls, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withRepeatedTranspositions (std::vector<float> transpositions)
    {
        std::vector<float> newNotesInSemitones;
        std::vector<float> newBandwidths;
        std::vector<float> newPans;
        std::vector<float> newAmpls;
        for (const auto& transposition : transpositions)
        {
            for (int i = 0; i < notesInSemitones.size(); ++i)
            {
                newNotesInSemitones.push_back (notesInSemitones[i] + transposition);
                newBandwidths.push_back (bandwidths[i]);
                newPans.push_back (pans[i]);
                newAmpls.push_back (ampls[i]);
            }
        }
        
        return MelodicNotes (newNotesInSemitones, newPans, newBandwidths, newAmpls, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withPan (float newPan)
    {
        return MelodicNotes (notesInSemitones, std::vector<float> (notesInSemitones.size(), newPan), bandwidths, ampls, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withCyclingPans (std::vector<float> cyclingPans)
    {
        std::vector<float> newNotesInSemitones;
        std::vector<float> newPans;
        std::vector<float> newBandwidths;
        std::vector<float> newAmpls;
        
        for (int i = 0; i < notesInSemitones.size() * cyclingPans.size(); ++i)
        {
            newNotesInSemitones.push_back (notesInSemitones[i % notesInSemitones.size()]);
            newPans.push_back (cyclingPans[i % cyclingPans.size()]);
            newBandwidths.push_back (bandwidths[i % bandwidths.size()]);
            newAmpls.push_back (ampls[i % ampls.size()]);
        }
        
        return MelodicNotes (newNotesInSemitones, newPans, newBandwidths, newAmpls, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withCyclingPans (int numPositions)
    {
        std::vector<float> cyclingPans;
        float factor = 2.0f / static_cast<float> (numPositions - 1);
        
        for (float i = -1; i < 1 - factor; i += factor)
            cyclingPans.push_back (i);
        for (float i = 1; i > -1 + factor; i -= factor)
            cyclingPans.push_back (i);
        
        return this->withCyclingPans (cyclingPans);
    }
    
    MelodicNotes withCyclingBandwidths (std::vector<float> cyclingBandwidths)
    {
        std::vector<float> newNotesInSemitones;
        std::vector<float> newPans;
        std::vector<float> newBandwidths;
        std::vector<float> newAmpls;
        
        for (int i = 0; i < notesInSemitones.size() * cyclingBandwidths.size(); ++i)
        {
            newNotesInSemitones.push_back (notesInSemitones[i % notesInSemitones.size()]);
            newPans.push_back (pans[i % pans.size()]);
            newBandwidths.push_back (cyclingBandwidths[i % cyclingBandwidths.size()]);
            newAmpls.push_back (ampls[i % ampls.size()]);
        }
        
        return MelodicNotes (newNotesInSemitones, newPans, newBandwidths, newAmpls, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withCyclingAmpls (std::vector<float> cyclingAmpls)
    {
        std::vector<float> newNotesInSemitones;
        std::vector<float> newPans;
        std::vector<float> newBandwidths;
        std::vector<float> newAmpls;
        
        for (int i = 0; i < notesInSemitones.size() * cyclingAmpls.size(); ++i)
        {
            newNotesInSemitones.push_back (notesInSemitones[i % notesInSemitones.size()]);
            newPans.push_back (pans[i % pans.size()]);
            newBandwidths.push_back (bandwidths[i % bandwidths.size()]);
            newAmpls.push_back (cyclingAmpls[i % cyclingAmpls.size()]);
        }
        
        return MelodicNotes (newNotesInSemitones, newPans, newBandwidths, newAmpls, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withSubdivisions (int numSections, int position) // subdivides into numSections, puts your position in that section. e.g. 3 sections, position = 1 would give you a (0-1-0, 0-1-0) pattern
    {
        std::vector<float> newNotesInSemitones;
        std::vector<float> newPans;
        std::vector<float> newBandwidths;
        std::vector<float> newAmpls;
        
        for (int i = 0; i < notesInSemitones.size() * numSections; ++i)
        {
            if (i % numSections == position)
            {
                newNotesInSemitones.push_back (notesInSemitones[i / numSections]);
                newPans.push_back (pans[i / numSections]);
                newBandwidths.push_back (bandwidths[i / numSections]);
                newAmpls.push_back (ampls[i / numSections]);
            }
            else
            {
                newNotesInSemitones.push_back (0);
                newPans.push_back (0);
                newBandwidths.push_back (0);
                newAmpls.push_back (0);
            }
        }
        
        return MelodicNotes (newNotesInSemitones, newPans, newBandwidths, newAmpls, centerFreq, noteDurationInSeconds / static_cast<float> (numSections), sampleRate);
    }
    
    MelodicNotes withSegmentedPans (int numSegments, bool leftToRight)
    {
        std::vector<float> cyclingPans;
        float factor = leftToRight ? 1.0 : -1.0;
        float segmentLength = 2.0f / static_cast<float> (numSegments);
        for (int i = 0; i < numSegments; ++i)
        {
            cyclingPans.push_back ((-1.0f + i * segmentLength) * factor);
        }
        
        return this->withCyclingPans (cyclingPans);
    }
    
    MelodicNotes withNoteOffset (int offset)
    {
        std::vector<float> newNotesInSemitones;
        std::vector<float> newPans;
        std::vector<float> newBandwidths;
        std::vector<float> newAmpls;
        
        for (int i = 0; i < notesInSemitones.size(); ++i)
        {
            int idx = (i + offset) % notesInSemitones.size();
            newNotesInSemitones.push_back (notesInSemitones[idx]);
            newPans.push_back (pans[idx]);
            newBandwidths.push_back (bandwidths[idx]);
            newAmpls.push_back (ampls[idx]);
        }
        
        return MelodicNotes (newNotesInSemitones, newPans, newBandwidths, newAmpls, centerFreq, noteDurationInSeconds, sampleRate);
    }
    
    MelodicNotes withTransposition (float semitonesToTranspose)
    {
        std::vector<float> newNotesInSemitones;
        for (const auto& noteInSemitones : notesInSemitones)
        {
            newNotesInSemitones.push_back (noteInSemitones + semitonesToTranspose);
        }
        
        return MelodicNotes (newNotesInSemitones, pans, bandwidths, ampls, centerFreq, noteDurationInSeconds, sampleRate);
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
            noiseNotes.push_back (NoiseNote(noteFreq, bandwidths[i], noteDurationInSamples, pans[i], { 1.0f, 1.0f }, false, ampls[i]));
        }
        
        return noiseNotes;
    }
    
    std::vector<SequenceableNote> sequenceableNotes()
    {
        std::vector<SequenceableNote> sequenceableNotes;
        float semitoneRatio = std::pow (2.0f, 1.0f / 12.0f);
        int noteDurationInSamples = noteDurationInSeconds * sampleRate;
        
        // Assumes notesInSemitones.size() == pans.size()
        for (int i = 0; i < notesInSemitones.size(); ++i)
        {
            float noteFreq = centerFreq * std::pow (semitoneRatio, notesInSemitones[i]);
            sequenceableNotes.push_back (SequenceableNote (noteFreq, ampls[i], pans[i], 0.0f, noteDurationInSamples));
        }
        return sequenceableNotes;
    }
    
    std::vector<NoiseNote> noiseNotesWithInterspersedReference (float referenceFreq, float refBandwidth = 3.0)
    {
        // Creates noise notes, but alternates the reference frequency in the center in between each note
        std::vector<NoiseNote> noiseNotes;
        
        float semitoneRatio = std::pow(2.0f, 1.0f / 12.0f);
        int noteDurationInSamples = noteDurationInSeconds * sampleRate;
        
        // Assumes notesInSemitones.size() == pans.size()
        for (int i = 0; i < notesInSemitones.size(); ++i)
        {
            float noteFreq = centerFreq * std::pow (semitoneRatio, notesInSemitones[i]);
            noiseNotes.push_back (NoiseNote(noteFreq, bandwidths[i], noteDurationInSamples, pans[i], { 1.0f, 1.0f }, false, ampls[i]));
            noiseNotes.push_back (NoiseNote(referenceFreq, refBandwidth, noteDurationInSamples, pans[i], { 1.0f, 1.0f }, false)); // reference frequency
        }
        
        return noiseNotes;
    }
    
private:
    std::vector<float> notesInSemitones;
    std::vector<float> pans;
    std::vector<float> bandwidths;
    std::vector<float> ampls;
    float centerFreq;
    float noteDurationInSeconds;
    float sampleRate = 44100;
};
