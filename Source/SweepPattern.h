/*
  ==============================================================================

    SweepPattern.h
    Created: 20 Oct 2024 11:17:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SequencerListener.h"

class SweepPattern
{
public:
    SweepPattern (const std::vector<std::pair<float, float>>& points, float durationInSeconds, float sampleRate);
    
    std::pair<float, float> getNextFrequencyAndPan();
    std::pair<float, float> getCurrFrequencyAndPan() const;
    
    SweepPattern withPan (float pan); // returns a new sweep pattern with all of the points at this pan
    SweepPattern withTranspositionInOctaves (float numOctaves);
    
    void setListener (SequencerListener* listener);

private:
    std::vector<std::pair<float, float>> points; // Vector of (frequency, pan) pairs
    std::vector<int> segmentSampleCounts;        // Samples per segment for even distribution

    float sampleRate;
    float durationInSeconds;

    int idx;               // Current overall sample index
    int cycleLen;          // Total samples in the cycle
    int currSegment;       // Index of current segment
    int totalSegments;
    int samplesPerSegment; // Samples in the current segment
    int segmentSampleIdx;  // Sample index within the current segment

    float currFreq;
    float currPan;

    float segmentStartFreq;
    float segmentEndFreq;
    float segmentStartPan;
    float segmentEndPan;

    void advanceSegment(); // Advance to the next segment
    
    SequencerListener* listener;
};
