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
    updateBandpassFilter (900, 1100);
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

void PinkNoiseGenerator::updateBandpassFilter (const float lowCutFreq, const float highCutFreq)
{
    // Update the low cut filter
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod (lowCutFreq,
                                                                                                       sampleRate,
                                                                                                       2 * (4));
    
    auto& lowCut = bandpass.get<ChainPositions::LowCut>();
    updateCutFilter(lowCut, lowCutCoefficients);
    
    // Update the high cut filter
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod (highCutFreq,
                                                                                                       sampleRate,
                                                                                                       2 * (4));
    auto& highCut = bandpass.get<ChainPositions::HighCut>();
    updateCutFilter(highCut, highCutCoefficients);
}

template<typename ChainType, typename CoefficientType>
void PinkNoiseGenerator::updateCutFilter(ChainType& chain, const CoefficientType& coefficients)
{
    chain.template setBypassed<0>(true);
    chain.template setBypassed<1>(true);
    chain.template setBypassed<2>(true);
    chain.template setBypassed<3>(true);
    
    update<3>(chain, coefficients);
    update<2>(chain, coefficients);
    update<1>(chain, coefficients);
    update<0>(chain, coefficients);
}

void PinkNoiseGenerator::updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}
