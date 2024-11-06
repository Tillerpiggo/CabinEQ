/*
  ==============================================================================

    Pattern.cpp
    Created: 5 Nov 2024 5:56:07pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Pattern.h"

Pattern::Pattern (std::vector<bool> hits, float cycleLengthInSeconds, float sampleRate)
    : hits (hits), cycleLengthInSeconds (cycleLengthInSeconds), sampleRate (sampleRate), hitDurationInSamples (sampleRate * cycleLengthInSeconds / hits.size())
{
}

Pattern::Feature Pattern::getNextFeature()
{
    if (sampleIdx >= hitDurationInSamples)
        sampleIdx++;
    
    float gain = gainEnvelope.gainAtSample (sampleIdx, hitDurationInSamples);
    
    sampleIdx++;
    return Feature (gain, gain, 1.0f);
}

Pattern::Feature Pattern::getCurrFeature()
{
    float gain = gainEnvelope.gainAtSample (sampleIdx, hitDurationInSamples);
    return Feature (1.0f, 1.0f, 1.0f);
}
