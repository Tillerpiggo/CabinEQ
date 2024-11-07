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
    fillBuffer();
}

std::pair<float, float> NoiseGenerator::getNextSample()
{
    if (! pattern.has_value())
        return { 0.0f, 0.0f };
    
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
//    lowCutFilter.prepare (spec);
//    highCutFilter.prepare (spec);
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
}

void NoiseGenerator::setStartAndEndFrequency (float startFreq, float endFreq)
{
    this->startFreq = startFreq;
    this->endFreq = endFreq;
    updateFilters();
}

void NoiseGenerator::setFrequencyRange (std::pair<float, float> freqRange)
{
    float lowFreq = std::max (freqRange.first, 20.0f);
    float highFreq = std::min (freqRange.second, 18000.0f);
    setStartAndEndFrequency (lowFreq, highFreq);
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
    juce::dsp::ProcessContextReplacing<float> context (block);
    bandpass.process (context);
//    lowCutFilter.process (block);
//    highCutFilter.process (block);
}

//void NoiseGenerator::updateFilters()
//{
//    lowCutFilter.setCutoff (CutoffFilter::Type::highPass, startFreq);
//    highCutFilter.setCutoff (CutoffFilter::Type::lowPass, endFreq);
//}

void NoiseGenerator::updateFilters()
{
    // Update the low cut filter
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod (startFreq,
                                                                                                       44100, // TODO: this is super bad fix fix fix
                                                                                                       2 * (8));
    auto& lowCut = bandpass.get<0>();
    updateCutFilter (lowCut, lowCutCoefficients);
    
    // Update the high cut filter
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod (endFreq,
                                                                                                       44100, // TODO: this is super bad fix fix fix
                                                                                                       2 * (8));
    auto& highCut = bandpass.get<1>();
    updateCutFilter(highCut, highCutCoefficients);
}

template<typename ChainType, typename CoefficientType>
void NoiseGenerator::updateCutFilter(ChainType& chain, const CoefficientType& coefficients)
{
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
void NoiseGenerator::update (ChainType& chain, CoefficientType& coefficients)
{
    *chain.template get<Index>().coefficients =  *coefficients[Index];
}
