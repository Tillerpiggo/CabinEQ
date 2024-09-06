/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PlaybackManager.h"
#include <cmath>
#include <random>

PlaybackManager::PlaybackManager()
    : filter (FFT_SIZE),
      arbitrarySequencer (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      arbitrarySequencer2 (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      arbitrarySequencer3 (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      arbitrarySequencer4 (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      arbitrarySequencer5 (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      isTesting (false),
      isSweeping (false),
      isCalibrating (false),
      isProcessing (false),
      hasPreparedFilter (false)
{
    dryGainProcessor.setGainDecibels (0.0f);
    wetGainProcessor.setGainDecibels (0.0f);
    startTimer (300);
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& ioBuffer)
{
    auto* leftChannel = ioBuffer.getWritePointer(0);
    auto* rightChannel = ioBuffer.getNumChannels() > 1 ? ioBuffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating || isTesting)
    {
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            std::pair<float, float> value = getNextSample();
//            value.first += pinkNoise.generate() * 4.0;
//            value.second += pinkNoise.generate() * 4.0;
            leftChannel[sample] = value.first * 0.05 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
        }
    }
    else if (isSweeping)
    {
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            const std::pair<float, float> value = sineSweepGenerator.getNextSample();
            float referenceGain = juce::Decibels::decibelsToGain (referenceVolume);
            referenceGain *= juce::Decibels::decibelsToGain (getCompensationDBAtFrequency (sineSweepGenerator.getCurrFreq()));
            
            leftChannel[sample] = value.first * 0.05 * 0.5 * referenceGain;
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5 * referenceGain;
        }
    }
    else
    {
        auto numSamples = ioBuffer.getNumSamples();
        
        juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
        auto ioContext = juce::dsp::ProcessContextReplacing<float> (ioBlock);
        if ((isProcessing && hasPreparedFilter) || isSweeping)
        {
            filter.process (ioContext);
            wetGainProcessor.process (ioContext);
        }
        else
        {
            dryGainProcessor.process (ioContext);
        }
        
//        if (isCalibrating && playBandpass)
//        {
//            stereoBandpass.process (ioContext);
//        }
    }
}

void PlaybackManager::updateFilterWithCurves (Curve& leftAmplCurve, Curve& rightAmplCurve, int fftSize)
{
    filter.updateWithCurves (leftAmplCurve, rightAmplCurve, fftSize); // make right curve control everything for experimentation
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    stereoBandpass.prepare (spec);
    sampleRate = spec.sampleRate;
    *stereoBandpass.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass (spec.sampleRate, 200.0f, 5.0f);
    arbitrarySequencer.setSampleRate (spec.sampleRate);
    arbitrarySequencer2.setSampleRate (spec.sampleRate);
    arbitrarySequencer3.setSampleRate (spec.sampleRate);
    arbitrarySequencer4.setSampleRate (spec.sampleRate);
    arbitrarySequencer5.setSampleRate (spec.sampleRate);
    sineWaveGenerator1.setSampleRate (spec.sampleRate);
    sineWaveGenerator2.setSampleRate (spec.sampleRate);
    hasPreparedFilter = true;
}

float PlaybackManager::getCurrPlayingFreq() const
{
    return arbitrarySequencer.currentlyPlayingFrequency();
}

float PlaybackManager::getCurrTestingFreq() const
{
    return testingFreq;
}

float PlaybackManager::getCurrSineSweepFreq() const
{
    return sineSweepGenerator.getCurrFreq();
}

void PlaybackManager::setIsTesting (bool isTesting)
{
    this->isTesting = isTesting;
}

void PlaybackManager::setIsSweeping (bool isSweeping)
{
    this->isSweeping = isSweeping;
}

void PlaybackManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
}

void PlaybackManager::setIsProcessing (bool isProcessing)
{
    this->isProcessing = isProcessing;
}

void PlaybackManager::setDryWetVolumeBalance (float balance)
{
    dryGainProcessor.setGainDecibels (-balance);
    wetGainProcessor.setGainDecibels (+balance);
}

void PlaybackManager::setWetVolume (float wetVolume)
{
    wetGainProcessor.setGainDecibels (wetVolume);
}

void PlaybackManager::setDryVolume (float dryVolume)
{
    dryGainProcessor.setGainDecibels (dryVolume);
}

void PlaybackManager::setSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl)
{
//    sineSweepGenerator.setCenterFrequency (centerFreq, ampl);
}

void PlaybackManager::updateSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl)
{
//    sineSweepGenerator.updateCenterFrequency (centerFreq, ampl);
}

void PlaybackManager::startLeftAmplCalibration (float freq, Curve& leftAmplCurve)
{
    // Taking a break
}

void PlaybackManager::updateLeftAmplCalibration (float freq, Curve& leftAmplCurve)
{
    // Taking a break
}

void PlaybackManager::startRightAmplCalibration (float freq, Curve& rightAmplCurve)
{
    // Taking a break
}

void PlaybackManager::updateRightAmplCalibration (float freq, Curve& rightAmplCurve)
{
    // Taking a break
}


void PlaybackManager::startTestingFreq (float freq, Curve& curve)
{
    // for now do nothing
}

void PlaybackManager::updateTestingFreq (float freq, Curve& curve)
{
    // for now do nothing
}

void PlaybackManager::startSineSweep (float centerFreq, Curve amplCurve, Curve panCurve)
{
//    sineSweepGenerator.setSweep (centerFreq, amplCurve, panCurve);
}

void PlaybackManager::updateSineSweep (float centerFreq, Curve amplCurve, Curve panCurve)
{
//    sineSweepGenerator.updateSweep (centerFreq, amplCurve, panCurve);
}

void PlaybackManager::stopTestingFreq()
{
    isTesting = false;
}

void PlaybackManager::setReferenceVolume (float volume)
{
    this->referenceVolume = volume;
}

void PlaybackManager::startPlayingReferenceFreqs()
{
//    int noteDurationInSamples = 5000;
//    SequenceableNote refNote (referenceNote.frequency, referenceNote.amplitude, 0.0f, noteDurationInSamples);
//    SequenceableNote refNote2 (referenceNote2.frequency, referenceNote2.amplitude, 0.0f, noteDurationInSamples);
//    arbitrarySequencer.setNotesForSpatialCalibration ({ refNote, refNote2 });
}

void PlaybackManager::updatePlayingReferenceFreqs()
{
//    int noteDurationInSamples = 5000;
//    SequenceableNote refNote (referenceNote.frequency, referenceNote.amplitude, 0.0f, noteDurationInSamples);
//    SequenceableNote refNote2 (referenceNote2.frequency, referenceNote2.amplitude, 0.0f, noteDurationInSamples);
//    arbitrarySequencer.updateNotesForSpatialCalibration({ refNote, refNote2 });
}

void PlaybackManager::setReferenceVolume1 (float volume)
{
    this->referenceNote.amplitude = volume + 6.0f;
}

void PlaybackManager::setReferenceVolume2 (float volume)
{
    this->referenceNote2.amplitude = volume + 6.0f;
}

void PlaybackManager::setReferencePan (float pan)
{
    this->referencePan = pan;
    this->leftRefNote.amplitude = referenceNote.amplitude - (pan < 0 ? pan : 0);
    this->rightRefNote.amplitude = referenceNote.amplitude + (pan > 0 ? pan : 0);
}

void PlaybackManager::timerCallback()
{
    playBandpass = ! playBandpass;
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    auto sample0 = arbitrarySequencer.getNextSample();
    auto sample1 = arbitrarySequencer2.getNextSample();
    auto sample2 = arbitrarySequencer3.getNextSample();
    auto sample3 = arbitrarySequencer4.getNextSample();
    auto sample4 = arbitrarySequencer5.getNextSample();
    
    return getSumOfSamples ({ sample0, sample1, sample2, sample3, sample4 });
}

std::pair<float, float> PlaybackManager::getSumOfSamples (std::vector<std::pair<float, float>> samples)
{
    float leftSample = 0;
    float rightSample = 0;
    for (const auto& sample : samples)
    {
        leftSample += sample.first;
        rightSample += sample.second;
    }
    return { leftSample, rightSample };
}

float PlaybackManager::getCompensationDBAtFrequency (float frequency)
{
    return 0.0f;//-4.5f * std::log2 (frequency / 1000.0f);
}

float PlaybackManager::getReferenceCompensationDBAtFrequency (float frequency)
{
    return 0.0f;
}
 
juce::dsp::IIR::Coefficients<float>::Ptr PlaybackManager::createDelayCoefficients(float sampleRate, float delaytime) const
{
    // Basic first order all pass filter
    float a = (1.0f - delaytime * 0.5f * sampleRate) / (1.0f + delaytime * 0.5f * sampleRate);
    juce::dsp::IIR::Coefficients<float>::Ptr coefs(new juce::dsp::IIR::Coefficients<float>(a * a, 2.0f * a, 1.0f, 1.0f, 2.0f * a, a * a));
    return coefs;
}

std::vector<SequenceableNote> PlaybackManager::getNotesForAmplCalibrationAt (float freq, float ampl)
{
    // Play reference note and then do below - controlled - above
    int noteDurationInSamples = 2000;
    StereoGainEnvelope envelope (500);
    
    // Notes that will be used regardless
    SequenceableNote mainNote (freq, ampl, 0.0f, 0.0f, noteDurationInSamples, envelope);
    SequenceableNote refNote (1000.0f, 0.0f, 0.0f, 0.0f, noteDurationInSamples, envelope);
    SequenceableNote refNote2 (4000.0f, 0.0f, 0.0f, 0.0f, noteDurationInSamples, envelope);
    
    // Play below - controlled - above while reference note is playing
    std::vector<SequenceableNote> notes;
//    std::vector<float> ampls { 0.0, 20.0, -10.0, -20.0, 10.0 };
    std::vector<float> ampls { 0.0 };
//    std::vector<float> ampls { -6.0, -4.0, -2.0, 0.0, 2.0, 4.0, 6.0 };
//    std::vector<float> pans { -1.0, -0.75, -0.5, -0.25, 0.0, 0.25, 0.5, 0.75, 1.0 };
    std::vector<float> pans { 0.0 };
    
//    float inverseFMVal = -1.0 * inverseFM.valueAtFrequency (freq, 0.0f);
    
    int amplIdx = 0;
    for (const auto& pan : pans)
    {
        notes.emplace_back (mainNote.withAmplitudeChange (ampls[amplIdx] - 40.0f));
        notes.emplace_back (mainNote.withAmplitudeChange (ampls[amplIdx] - 46.0f));
        amplIdx++;
        if (amplIdx >= ampls.size())
            amplIdx = 0;
    }
    
    return notes;
}

// bandpass stuff
void PlaybackManager::updateBandpassFilter (const float lowCutFreq, const float highCutFreq)
{
    // Update the low cut filter
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod (lowCutFreq,
                                                                                                       44100,
                                                                                                       2 * (8));
    
    auto& lowCut = bandpass.get<ChainPositions::LowCut>();
    updateCutFilter(lowCut, lowCutCoefficients);
    
    // Update the high cut filter
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod (highCutFreq,
                                                                                                           44100,
                                                                                                       2 * (8));
    auto& highCut = bandpass.get<ChainPositions::HighCut>();
    updateCutFilter(highCut, highCutCoefficients);
}

template<typename ChainType, typename CoefficientType>
void PlaybackManager::updateCutFilter(ChainType& chain, const CoefficientType& coefficients)
{
//    chain.template setBypassed<0>(true);
//    chain.template setBypassed<1>(true);
//    chain.template setBypassed<2>(true);
//    chain.template setBypassed<3>(true);
//    chain.template setBypassed<4>(true);
//    chain.template setBypassed<5>(true);
//    chain.template setBypassed<6>(true);
//    chain.template setBypassed<7>(true);
    
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
void PlaybackManager::update (ChainType& chain, CoefficientType& coefficients)
{
    updateCoefficients (chain.template get<Index>().coefficients, coefficients[Index]);
    chain.template setBypassed<Index>(false);
}

void PlaybackManager::updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}
