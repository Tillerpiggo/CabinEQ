/*
  =============================================================================

    PinkNoiseGenerator.cpp
    Created: 12 Aug 2024 8:56:18p
    Author:  Tyler Ge

  =============================================================================
*/

#include "PinkNoiseGenerator.h"


PinkNoiseGenerator::PinkNoiseGenerator()
{
}

void PinkNoiseGenerator::setSampleRate (float newSampleRate)
{
    this->sampleRate = newSampleRate;
}

const std::pair<float, float> PinkNoiseGenerator::getNextSample()
{
    float val = pinkNoise.generate();
    return { val, val };
}

void PinkNoiseGenerator::setNote (Note note)
{
    // TODO
}

void PinkNoiseGenerator::setFrequency (float frequencyInHz)
{
    // TODO
}

void PinkNoiseGenerator::setVolume (float volumeInDecibels)
{
    // TODO
}

void PinkNoiseGenerator::setPan (float panInDecibels)
{
    // TODO
}

void PinkNoiseGenerator::setPhase (float phaseInDecibels)
{
    // TODO
}
