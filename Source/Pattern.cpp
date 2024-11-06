/*
  ==============================================================================

    Pattern.cpp
    Created: 5 Nov 2024 5:56:07pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Pattern.h"

Pattern::Pattern (std::vector<bool> hits)
    : hits (hits)
{
}

Pattern::Feature Pattern::getNextFeature (float percentIncrement)
{
    if (sampleIdx >= hitDurationInSamples)
    {
        sampleIdx = remainder (sampleIdx, hitDurationInSamples);
        hitIdx++;
        if (hitIdx >= hits.size())
            hitIdx = 0;
    }
    
    float gain = gainEnvelope.gainAtSample (sampleIdx, hitDurationInSamples);
    
    sampleIdx += percentIncrement * 100;
    
    if (hits[hitIdx] == false)
        return Feature (0.0f, 0.0f, 1.0f);
    
    return Feature (gain, gain, 1.0f);
}

Pattern::Feature Pattern::getCurrFeature()
{
    float gain = gainEnvelope.gainAtSample (sampleIdx, hitDurationInSamples);
    return Feature (gain, gain, 1.0f);
}
