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
      crossfeedFilter (FFT_SIZE, 500),
      arbitrarySequencer (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      arbitrarySequencer2 (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      isTesting (false),
      isSweeping (false),
      isCalibrating (false),
      isProcessing (false),
      hasPreparedFilter (false)
{
    dryGainProcessor.setGainDecibels (0.0f);
    wetGainProcessor.setGainDecibels (0.0f);
    
    setCalibratingEQNode (EQNode (-1, REFERENCE_FREQ, 0.0f, 0.0f)); // placeholder to avoid errors
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& ioBuffer)
{
    auto* leftChannel = ioBuffer.getWritePointer(0);
    auto* rightChannel = ioBuffer.getNumChannels() > 1 ? ioBuffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating || isTesting)
    {
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            const std::pair<float, float> value = getNextSample();
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
        
        // Send main signal to mainBlock, and L/R inverted signal to crossfeedBuffer
        mainBlock.copyFrom (ioBlock);
        crossfeedBuffer.copyFrom(0, 0, ioBuffer, 1, 0, numSamples); // copy in-R to aux-L
        crossfeedBuffer.copyFrom(1, 0, ioBuffer, 0, 0, numSamples); // copy in-L to aux-R
        
        auto mainContext = juce::dsp::ProcessContextReplacing<float> (mainBlock);
        auto crossfeedContext = juce::dsp::ProcessContextReplacing<float> (crossfeedBlock);
        
        if ((isProcessing && hasPreparedFilter) || isSweeping)
        {
            filter.process (mainContext);
            crossfeedFilter.process (crossfeedContext);
            mainBlock += crossfeedBlock.multiplyBy (0.2);
            ioBlock.replaceWithSumOf(mainBlock, crossfeedBlock.multiplyBy (0.5));
            wetGainProcessor.process (ioContext);
        }
        else
        {
            dryGainProcessor.process (ioContext);
        }
    }
}

void PlaybackManager::updateFilterWithCurve (Curve& curve)
{
    filter.updateWithCurve (curve, FFT_SIZE);
    crossfeedFilter.updateWithCurve (curve, FFT_SIZE);
}

void PlaybackManager::updateFilterWithCurves (Curve& leftCurve, Curve& rightCurve)
{
    filter.updateWithCurves (leftCurve, rightCurve, FFT_SIZE);
    crossfeedFilter.updateWithCurves (rightCurve, leftCurve, FFT_SIZE);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    crossfeedFilter.prepare (spec);
    arbitrarySequencer.setSampleRate (spec.sampleRate);
    arbitrarySequencer2.setSampleRate (spec.sampleRate);
    hasPreparedFilter = true;
    
    mainBuffer = juce::AudioBuffer<float>(2, spec.maximumBlockSize);
    mainBlock = juce::dsp::AudioBlock<float>(mainBuffer);
    crossfeedBuffer = juce::AudioBuffer<float>(2, spec.maximumBlockSize);
    crossfeedBlock = juce::dsp::AudioBlock<float>(crossfeedBuffer);
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

void PlaybackManager::setSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl)
{
    sineSweepGenerator.setCenterFrequency (centerFreq, ampl);
}

void PlaybackManager::updateSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl)
{
    sineSweepGenerator.updateCenterFrequency (centerFreq, ampl);
}

void PlaybackManager::setCalibratingEQNode (EQNode node)
{
    int noteDurationInSamples = 20000;
    node.amplitude += getCompensationDBAtFrequency (node.frequency);
    StereoGainEnvelope hardLeft = StereoGainEnvelope (StereoGainEnvelopeType::SOFT_LEFT);
    StereoGainEnvelope hardRight = StereoGainEnvelope (StereoGainEnvelopeType::SOFT_RIGHT);
    
    SequenceableNote referenceNoteLeft (referenceNote, noteDurationInSamples, hardLeft);
    SequenceableNote referenceNoteRight (referenceNote, noteDurationInSamples, hardRight);
    SequenceableNote controlledNoteLeft (node, noteDurationInSamples, hardLeft);
    SequenceableNote controlledNoteRight (node, noteDurationInSamples, hardRight);
    
    arbitrarySequencer.setNotes ({ referenceNoteLeft, referenceNoteRight });
    arbitrarySequencer2.setNotes ({ controlledNoteLeft, controlledNoteRight });
//
    
    
//    Note referenceNoteCompensated = referenceNote;
//    referenceNoteCompensated.gain += getCompensationDBAtFrequency (node.frequency);
//    referenceNoteCompensated.gain += getReferenceCompensationDBAtFrequency (node.frequency);
//    node.amplitude += getCompensationDBAtFrequency (node.frequency);
//    
//    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
//    SequenceableNote note2 (node, noteDurationInSamples);
//    arbitrarySequencer.setNotes ({ note1, note2 }, true);
}

void PlaybackManager::updateCalibratingEQNode (EQNode updatedNode)
{
    int noteDurationInSamples = 20000;
    updatedNode.amplitude += getCompensationDBAtFrequency (updatedNode.frequency);
    StereoGainEnvelope hardLeft = StereoGainEnvelope (StereoGainEnvelopeType::SOFT_LEFT);
    StereoGainEnvelope hardRight = StereoGainEnvelope (StereoGainEnvelopeType::SOFT_RIGHT);
    SequenceableNote controlledNoteLeft (updatedNode, noteDurationInSamples, hardLeft);
    SequenceableNote controlledNoteRight (updatedNode, noteDurationInSamples, hardRight);
    
    arbitrarySequencer2.changeNoteAtIdx (0, controlledNoteLeft.note());
    arbitrarySequencer2.changeNoteAtIdx (1, controlledNoteRight.note());
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += getCompensationDBAtFrequency (updatedNode.frequency);
    referenceNoteCompensated.gain += getReferenceCompensationDBAtFrequency (updatedNode.frequency);
    updatedNode.amplitude += getCompensationDBAtFrequency (updatedNode.frequency);
    
//    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
//    SequenceableNote note2 (updatedNode, noteDurationInSamples);
//    arbitrarySequencer.changeNoteAtIdx (0, note1.note());
//    arbitrarySequencer.changeNoteAtIdx (1, note2.note());
    
//    updatedNode.amplitude -= 3.0f;
//    SequenceableNote quieterNote (updatedNode, noteDurationInSamples);
//    SequenceableNote refNote (referenceNoteCompensated, noteDurationInSamples);
//    updatedNode.amplitude += 6.0f;
//    SequenceableNote louderNote (updatedNode, noteDurationInSamples);
//    SequenceableNote silentNote (referenceNoteCompensated, noteDurationInSamples, StereoGainEnvelope (StereoGainEnvelopeType::SILENT));
//    float volumeStep = 3.0f;
//    volumeStep += 0.2 * std::log2 (updatedNode.frequency / 1000.0f);
//    referenceNoteCompensated.gain -= volumeStep;
//    SequenceableNote quietRef (referenceNoteCompensated, noteDurationInSamples);
//    SequenceableNote updatedNote (updatedNode, noteDurationInSamples);
//    referenceNoteCompensated.gain += volumeStep * 2;
//    SequenceableNote loudRef (referenceNoteCompensated, noteDurationInSamples);
//    arbitrarySequencer.changeNoteAtIdx (0, quietRef.note());
//    arbitrarySequencer.changeNoteAtIdx (1, updatedNote.note());
//    arbitrarySequencer.changeNoteAtIdx (3, updatedNote.note());
//    arbitrarySequencer.changeNoteAtIdx (4, loudRef.note());
//
//    arbitrarySequencer.changeNoteAtIdx (0, quieterNote.note());
//    arbitrarySequencer.changeNoteAtIdx (1, refNote.note());
//    arbitrarySequencer.changeNoteAtIdx (2, louderNote.note());
}

void PlaybackManager::startTestingFreq (float freq, Curve& curve)
{
    if (isTesting)
    {
        updateTestingFreq (freq, curve);
        return;
    }
    
    int noteDurationInSamples = 20000;
    isTesting = true;
    
    float ampl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq).first.real());
    ampl += getCompensationDBAtFrequency (freq);
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += getCompensationDBAtFrequency (freq);
    referenceNoteCompensated.gain += getReferenceCompensationDBAtFrequency (freq);
    
//    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
//    SequenceableNote note2 (freq, ampl, 0.0f, 0.0f, noteDurationInSamples);
//    arbitrarySequencer.setNotes ({ note1, note2 });
    
//    float bandwidth = 1.1;
//    float freqBelow = freq / bandwidth;
//    float freqAbove = freq * bandwidth;
//    float amplBelow = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freqBelow).first.real()) + getCompensationDBAtFrequency (freqBelow);
//    float amplAbove = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freqAbove).first.real()) + getCompensationDBAtFrequency (freqAbove);
//    
//    SequenceableNote noteBelow (freqBelow, amplBelow, 0.0f, 0.0f, noteDurationInSamples);
//    SequenceableNote noteMid (freq, ampl, 0.0f, 0.0f, noteDurationInSamples);
//    SequenceableNote noteAbove (freqAbove, amplAbove, 0.0f, 0.0f, noteDurationInSamples);
//    
//    arbitrarySequencer.setNotes ({ noteBelow, noteMid, noteAbove });
    
    auto nodeBelow = curve.nodeBelowFreq (freq);
    auto nodeAbove = curve.nodeAboveFreq (freq);
    
    if (! nodeBelow.has_value() || ! nodeAbove.has_value())
    {
        // for now, do nothing
        return;
    }
    
    std::cout << "NodeBelow: " << nodeBelow->first << ", " << nodeBelow->second << std::endl;
    std::cout << "NodeAbove: " << nodeAbove->first << ", " << nodeAbove->second << std::endl;
    
    auto [freqBelow, amplBelow] = nodeBelow.value();
    auto [freqAbove, amplAbove] = nodeAbove.value();
    amplBelow += getCompensationDBAtFrequency (freqBelow);
    amplAbove += getCompensationDBAtFrequency (freqAbove);
    
    SequenceableNote noteBelow (freqBelow, amplBelow, 0.0f, 0.0f, noteDurationInSamples);
    SequenceableNote noteMid (freq, ampl, 0.0f, 0.0f, noteDurationInSamples);
    SequenceableNote noteAbove (freqAbove, amplAbove, 0.0f, 0.0f, noteDurationInSamples);
    SequenceableNote silentNote (0.0f, 0.0f, 0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope (StereoGainEnvelopeType::SILENT));
    arbitrarySequencer.setNotes ({ noteBelow, noteMid, noteAbove, silentNote });
    
    testingFreq = freq;
}

void PlaybackManager::updateTestingFreq (float freq, Curve& curve)
{
    int noteDurationInSamples = 20000;
    
    float ampl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq).first.real());
    ampl += getCompensationDBAtFrequency (freq);
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += getCompensationDBAtFrequency (freq);
    referenceNoteCompensated.gain += getReferenceCompensationDBAtFrequency (freq);
    
//    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
//    SequenceableNote note2 (freq, ampl, 0.0f, 0.0f, noteDurationInSamples);
//    arbitrarySequencer.changeNoteAtIdx (0, note1.note());
//    arbitrarySequencer.changeNoteAtIdx (1, note2.note());
    
//    float bandwidth = 1.05;
//    float freqBelow = freq / bandwidth;
//    float freqAbove = freq * bandwidth;
//    float amplBelow = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freqBelow).first.real()) + getCompensationDBAtFrequency (freqBelow);
//    float amplAbove = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freqAbove).first.real()) + getCompensationDBAtFrequency (freqAbove);
//    
//    SequenceableNote noteBelow (freqBelow, amplBelow, 0.0f, 0.0f, noteDurationInSamples);
//    SequenceableNote noteMid (freq, ampl, 0.0f, 0.0f, noteDurationInSamples);
//    SequenceableNote noteAbove (freqAbove, amplAbove, 0.0f, 0.0f, noteDurationInSamples);
//    
//    arbitrarySequencer.setNotes ({ noteBelow, noteMid, noteAbove });
    
    auto nodeBelow = curve.nodeBelowFreq (freq);
    auto nodeAbove = curve.nodeAboveFreq (freq);
    
    if (! nodeBelow.has_value() || ! nodeAbove.has_value())
    {
        // for now, do nothing
        return;
    }
    
    auto [freqBelow, amplBelow] = nodeBelow.value();
    auto [freqAbove, amplAbove] = nodeAbove.value();
    amplBelow += getCompensationDBAtFrequency (freqBelow);
    amplAbove += getCompensationDBAtFrequency (freqAbove);
    
    SequenceableNote noteBelow (freqBelow, amplBelow, 0.0f, 0.0f, noteDurationInSamples);
    SequenceableNote noteMid (freq, ampl, 0.0f, 0.0f, noteDurationInSamples);
    SequenceableNote noteAbove (freqAbove, amplAbove, 0.0f, 0.0f, noteDurationInSamples);
    SequenceableNote silentNote (0.0f, 0.0f, 0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope (StereoGainEnvelopeType::SILENT));
    arbitrarySequencer.setNotes ({ noteBelow, noteMid, noteAbove, silentNote });
    
    testingFreq = freq;
}

void PlaybackManager::stopTestingFreq()
{
    isTesting = false;
    arbitrarySequencer.setNotes ({ SequenceableNote (referenceNote, 25000) });
}

void PlaybackManager::setReferenceVolume (float volume)
{
    this->referenceVolume = volume;
}

// Crossfeed
void PlaybackManager::setCalibratingEQNodeWithLeftCrossfeed (EQNode node, float crossfeedGain, float crossfeedDelay)
{
    setCalibratingEQNode (node);
}

void PlaybackManager::updateCalibratingEQNodeWithLeftCrossfeed (EQNode updatingNode, float crossfeedGain, float crossfeedDelay)
{
    updateCalibratingEQNode (updatingNode);
}

void PlaybackManager::setCalibratingEQNodeWithRightCrossfeed (EQNode node, float crossfeedGain, float crossfeedDelay)
{
    setCalibratingEQNode (node);
}

void PlaybackManager::updateCalibratingEQNodeWithRightCrossfeed (EQNode updatingNode, float crossfeedGain, float crossfeedDelay)
{
    updateCalibratingEQNode (updatingNode);
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    auto [leftSample1, rightSample1] = arbitrarySequencer.getNextSample();
    auto [leftSample2, rightSample2] = arbitrarySequencer2.getNextSample();
    return { leftSample1 + leftSample2, rightSample1 + rightSample2 };
}

float PlaybackManager::getCompensationDBAtFrequency (float frequency)
{
    return -4.5f * std::log2 (frequency / REFERENCE_FREQ);
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
