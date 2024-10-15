/*
  ==============================================================================

    WrapperPinkNoiseGenerator.h
    Created: 15 Oct 2024 3:02:10pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SpatialPinkNoiseGenerator.h"

class WrapperPinkNoiseGenerator
{
public:
    WrapperPinkNoiseGenerator() {}
    
    std::pair<float, float> getNextSample()
    {
        return pinkNoiseGenerator.getNextSample();
    }
    void setSampleRate (float newSampleRate)
    {
        pinkNoiseGenerator.setSampleRate (newSampleRate);
    }
    
private:
    SpatialPinkNoiseGenerator pinkNoiseGenerator;
};
