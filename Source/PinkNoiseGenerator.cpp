/*
  =============================================================================

    PinkNoiseGenerator.cpp
    Created: 12 Aug 2024 8:56:18p
    Author:  Tyler Ge

  =============================================================================
*/

#include "PinkNoiseGenerator.h"


PinkNoiseGenerator::PinkNoiseGenerator()
    : bufferSize (2048), leftBuffer (1, bufferSize), rightBuffer (1, bufferSize)
{
    populateBuffers();
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
        populateBuffers();
        bufferIdx = 0;
    }
    bufferIdx++;
    
    float leftVal = leftBuffer.getReadPointer (0)[bufferIdx];
    float rightVal = rightBuffer.getReadPointer (0)[bufferIdx];
//    float leftVal = val * leftAmplitudeCompensation;
//    float rightVal = val * rightAmplitudeCompensation;
    
//    return delayFilter.processSample ({ leftVal, rightVal });
    return { leftVal, rightVal };
}

void PinkNoiseGenerator::setNote (Note note)
{
    setFrequency (note.frequency);
    setVolume (note.amplitude);
    setPan (note.pan);
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
    panInDB = panInDecibels;
//    delayFilter.setDelay (panInDecibels * 2);
    updateAmplitudeCompensation();
}

//==============================================================
void PinkNoiseGenerator::populateBuffers()
{
    leftBuffer.clear();
    rightBuffer.clear();
    
    // Reset the heap block and fill it with new pink noise
    auto leftBufferPtr = leftBuffer.getWritePointer (0);
    auto rightBufferPtr = rightBuffer.getWritePointer (0);
    for (int i = 0; i < bufferSize; ++i)
    {
        leftBufferPtr[i] = pinkNoise.generate();
        rightBufferPtr[i] = pinkNoise.generate();
    }
    
    // Do other processing as needed...
    juce::dsp::AudioBlock<float> leftBlock (leftBuffer);
    juce::dsp::AudioBlock<float> rightBlock (rightBuffer);
    juce::dsp::ProcessContextReplacing<float> leftContext (leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext (rightBlock);
    bandpass.process (leftContext);
    bandpass.process (rightContext);
}

void PinkNoiseGenerator::updateAmplitudeCompensation()
{
    float tiltInDB = -1.5 * std::log2 (centerFrequency / 1000.0f);
    float amplitudeCompensation = tiltInDB + volumeInDB + 30.0f;
    leftAmplitudeCompensation = juce::Decibels::decibelsToGain (amplitudeCompensation - 0.5 * panInDB);
    rightAmplitudeCompensation = juce::Decibels::decibelsToGain (amplitudeCompensation + 0.5 * panInDB);
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
