/*
  ==============================================================================

    SweepPattern.cpp
    Created: 20 Oct 2024 11:17:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SweepPattern.h"

SweepPattern::SweepPattern (float centerFreq, float bandwidth, float durationInSeconds, float sampleRate)
    : centerFreq (centerFreq), bandwidth (bandwidth), sampleRate (sampleRate), durationInSeconds (durationInSeconds),
      idx (0), cycleLen (durationInSeconds * sampleRate), currFreq (centerFreq)
{}
