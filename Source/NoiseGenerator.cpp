/*
  ==============================================================================

    NoiseGenerator.cpp
    Created: 5 Nov 2024 4:30:21pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "NoiseGenerator.h"

NoiseGenerator::NoiseGenerator()
    : bufferSize (2048), buffer (1, bufferSize)
{
}

std::pair<float, float> NoiseGenerator::getNextSample()
{
    if (! pattern.has_value())
        return { 0.0f, 0.0f };
    
    if (shouldUpdateFilters)
        updateFilters();
    
    if (bufferIdx >= bufferSize)
    {
        fillBuffer();
        bufferIdx = 0;
    }
    
    auto feature = pattern->getNextFeature (percentIncrementPerSample);
    float val = buffer.getReadPointer(0)[bufferIdx];
    std::pair<float, float> nextSample { val * leftGain * feature.leftGain, val * rightGain * feature.rightGain };
    
    bufferIdx++;
    
    return nextSample;
}

void NoiseGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    lowCutFilter.prepare (spec);
    highCutFilter.prepare (spec);
}

void NoiseGenerator::setCenterFrequencyAndBandwidth (float centerFreq, float bandwidthInOctaves)
{
    float ratio = std::pow (2.0f, bandwidthInOctaves / 2.0f);
    startFreq = centerFreq / ratio;
    endFreq = centerFreq * ratio;
    if (startFreq <= 20.0f)
        startFreq = 20.0f;
    if (endFreq >= 20000.0f)
        endFreq = 20000.0f;
    shouldUpdateFilters = true;
}

void NoiseGenerator::setStartAndEndFrequency (float startFreq, float endFreq)
{
    this->startFreq = startFreq;
    this->endFreq = endFreq;
    shouldUpdateFilters = true;
}

void NoiseGenerator::setFrequencyRange (std::pair<float, float> freqRange)
{
    setStartAndEndFrequency (freqRange.first, freqRange.second);
}

void NoiseGenerator::setPattern (Pattern pattern)
{
    this->pattern = pattern;
}

void NoiseGenerator::fillBuffer()
{
    buffer.clear();
    
    // Fill the buffer with pink noise
    auto bufferPtr = buffer.getWritePointer (0);
    for (int i = 0; i < bufferSize; ++i)
        bufferPtr[i] = pinkNoise.generate();
    
    // Bandpass the noise with the cut filters
    juce::dsp::AudioBlock<float> block (buffer);
    lowCutFilter.process (block);
    highCutFilter.process (block);
}

void NoiseGenerator::updateFilters()
{
    lowCutFilter.setCutoff (CutoffFilter::Type::highPass, startFreq);
    highCutFilter.setCutoff (CutoffFilter::Type::lowPass, endFreq);
}
