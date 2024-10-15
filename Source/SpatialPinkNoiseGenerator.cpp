/*
  ==============================================================================

    SpatialPinkNoiseGenerator.cpp
    Created: 15 Oct 2024 11:33:41am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SpatialPinkNoiseGenerator.h"

SpatialPinkNoiseGenerator::SpatialPinkNoiseGenerator()
    : leftBuffer (1, bufferSize), rightBuffer (1, bufferSize)
{}

std::pair<float, float> SpatialPinkNoiseGenerator::getNextSample()
{
//    if (bufferIdx >= bufferSize)
//    {
//        fillBuffers();
//        bufferIdx = 0;
//    }
//    
//    float leftVal = leftBuffer.getReadPointer(0)[bufferIdx];
//    float rightVal = rightBuffer.getReadPointer(0)[bufferIdx];
//    bufferIdx++;
    
    float val = pinkNoise.generate();
    return { val, val };
    
//    return { leftVal, rightVal };
}

void SpatialPinkNoiseGenerator::setSampleRate (float newSampleRate)
{
    this->sampleRate = newSampleRate;
    
}

void SpatialPinkNoiseGenerator::setBandpass (float centreFreq, float bandwidth)
{
    
    *bandpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, centreFreq, Band::bandwidthToQFactor (bandwidth));
}

void SpatialPinkNoiseGenerator::fillBuffers()
{
//    leftBuffer.clear();
//    rightBuffer.clear();
//    
//    // Reset the heap blocka nd fill it with new pink noise
//    auto leftBufferPtr = leftBuffer.getWritePointer (0);
//    for (int i = 0; i < bufferSize; ++i)
//        leftBufferPtr[i] = pinkNoise.generate();
//    
//    auto rightBufferPtr = rightBuffer.getWritePointer(0);
//    for (int i = 0; i < bufferSize; ++i)
//        rightBufferPtr[i] = pinkNoise.generate();
    
    // Process the buffer with the bandpass filter
//    juce::dsp::AudioBlock<float> leftBlock (leftBuffer);
//    juce::dsp::AudioBlock<float> rightBlock (rightBuffer);
//    juce::dsp::ProcessContextReplacing<float> leftContext (leftBlock);
//    juce::dsp::ProcessContextReplacing<float> rightContext (rightBlock);
//    bandpass.process (leftContext);
//    bandpass.process (rightContext);
}
