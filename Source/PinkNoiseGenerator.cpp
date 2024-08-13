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
    populateHeapBlock();
}

void PinkNoiseGenerator::setSampleRate (float newSampleRate)
{
    this->sampleRate = newSampleRate;
}

const std::pair<float, float> PinkNoiseGenerator::getNextSample()
{
    if (heapBlockIdx >= heapBlockSize)
    {
        populateHeapBlock();
        heapBlockIdx = 0;
    }
    
    float val = heapBlock[heapBlockIdx];
    heapBlockIdx++;
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

//==============================================
void PinkNoiseGenerator::populateHeapBlock()
{
    // Reset the heap block and fill it with new pink noise
    heapBlock = juce::HeapBlock<float> (heapBlockSize);
    for (int i = 0; i < heapBlockSize; ++i)
        heapBlock[i] = pinkNoise.generate();
    
    // Do other processing as needed...
}
