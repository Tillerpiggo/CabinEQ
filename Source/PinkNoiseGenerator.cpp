/*
  =============================================================================

    PinkNoiseGenerator.cpp
    Created: 12 Aug 2024 8:56:18p
    Author:  Tyler Ge

  =============================================================================
*/

#include "PinkNoiseGenerator.h"


PinkNoiseGenerator::PinkNoiseGenerator()
    : bufferSize (2048), buffer (1, bufferSize)
{
    populateBuffer();
    updateFilters();
}

void PinkNoiseGenerator::setSampleRate (float newSampleRate)
{
    this->sampleRate = newSampleRate;
}

const std::pair<float, float> PinkNoiseGenerator::getNextSample()
{
    if (bufferIdx >= bufferSize)
    {
        populateBuffer();
        bufferIdx = 0;
    }
    
    float val = buffer.getReadPointer (0)[bufferIdx];
    bufferIdx++;
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
void PinkNoiseGenerator::populateBuffer()
{
    buffer.clear();
    
    // Reset the heap block and fill it with new pink noise
    auto bufferPtr = buffer.getWritePointer (0);
    for (int i = 0; i < bufferSize; ++i)
    {
        bufferPtr[i] = pinkNoise.generate();
    }
    
    // Do other processing as needed...
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    bandpass.process (context);
}

void PinkNoiseGenerator::updateLowCutFilters(const ChainSettings& chainSettings)
{
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                                       sampleRate,
                                                                                                       2 * (chainSettings.lowCutSlope + 1));
    
    auto& lowCut = bandpass.get<ChainPositions::LowCut>();
    updateCutFilter(lowCut, lowCutCoefficients, chainSettings.lowCutSlope);
}

void PinkNoiseGenerator::updateHighCutFilters(const ChainSettings& chainSettings)
{
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                                                                                                       sampleRate,
                                                                                                       2 * (chainSettings.highCutSlope + 1));
    
    auto& highCut = bandpass.get<ChainPositions::HighCut>();
    updateCutFilter(highCut, highCutCoefficients, chainSettings.highCutSlope);
}

void PinkNoiseGenerator::updateFilters()
{
    auto chainSettings = ChainSettings();
    
    updateLowCutFilters(chainSettings);
    updateHighCutFilters(chainSettings);
}

void PinkNoiseGenerator::updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}
