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
    setFrequency (1000.0f);
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
    
    float val = buffer.getReadPointer (0)[bufferIdx] * amplitudeCompensation;
    bufferIdx++;
    return { val, val };
}

void PinkNoiseGenerator::setNote (Note note)
{
    setFrequency (note.frequency);
    setVolume (note.gain);
}

void PinkNoiseGenerator::setFrequency (float frequencyInHz)
{
    float bandwidthFactor = 1;
    updateBandpassFilter (frequencyInHz / bandwidthFactor, frequencyInHz * bandwidthFactor);
    centerFrequency = frequencyInHz;
    updateAmplitudeCompensation();
}

void PinkNoiseGenerator::setVolume (float volumeInDecibels)
{
    volumeInDB = volumeInDecibels;
    updateAmplitudeCompensation();
}

void PinkNoiseGenerator::setPan (float panInDecibels)
{
    // TODO
}

void PinkNoiseGenerator::setPhase (float phaseInDecibels)
{
    // TODO
}

//==============================================================
void PinkNoiseGenerator::populateBuffer()
{
    buffer.clear();
    
    // Reset the heap block and fill it with new pink noise
    auto bufferPtr = buffer.getWritePointer (0);
    for (int i = 0; i < bufferSize; ++i)
    {
        bufferPtr[i] = noiseSrc.nextFloat();
    }
    
    // Do other processing as needed...
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    bandpass.process (context);
}

void PinkNoiseGenerator::updateAmplitudeCompensation()
{
    float tiltInDB = -4.5 * std::log2 (centerFrequency / 1000.0f);
    amplitudeCompensation = tiltInDB + volumeInDB + 30.0f;
    amplitudeCompensation = juce::Decibels::decibelsToGain (amplitudeCompensation);
}

void PinkNoiseGenerator::updateBandpassFilter (const float lowCutFreq, const float highCutFreq)
{
    // Update the low cut filter
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod (lowCutFreq,
                                                                                                       sampleRate,
                                                                                                       2 * (8));
    
    auto& lowCut = bandpass.get<ChainPositions::LowCut>();
    updateCutFilter(lowCut, lowCutCoefficients);
    
    // Update the high cut filter
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod (highCutFreq,
                                                                                                       sampleRate,
                                                                                                       2 * (8));
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
    chain.template setBypassed<4>(true);
    chain.template setBypassed<5>(true);
    chain.template setBypassed<6>(true);
    chain.template setBypassed<7>(true);
    
    update<7>(chain, coefficients);
    update<6>(chain, coefficients);
    update<5>(chain, coefficients);
    update<4>(chain, coefficients);
    update<3>(chain, coefficients);
    update<2>(chain, coefficients);
    update<1>(chain, coefficients);
    update<0>(chain, coefficients);
}

template<int Index, typename ChainType, typename CoefficientType>
void PinkNoiseGenerator::update (ChainType& chain, CoefficientType& coefficients)
{
    updateCoefficients (chain.template get<Index>().coefficients, coefficients[Index]);
    chain.template setBypassed<Index>(false);
}

void PinkNoiseGenerator::updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}
