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
    : filter (FFT_SIZE), arbitrarySequencer (std::make_unique<SineWaveGenerator> (SineWaveGenerator())), 
      isTesting (false),
      isSweeping (false),
      isCalibrating (false),
      isBypassed (false),
      hasPreparedFilter (false)
{
    dryGainProcessor.setGainDecibels (0.0f);
    wetGainProcessor.setGainDecibels (0.0f);
    
    setCalibratingEQNode (EQNode (-1, REFERENCE_FREQ, 0.0f, 0.0f)); // placeholder to avoid errors
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& buffer)
{
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating || isTesting)
    {
        std::cout << "isCalibrating: " << isCalibrating << ", isTesting: " << isTesting << std::endl;
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const std::pair<float, float> value = getNextSample();
            if (sample == 0)
            {
                std::cout << "sample (left: " << value.first << ", right: " << value.second << ")" << std::endl;
            }
            leftChannel[sample] = value.first * 0.05 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
        }
        
        
    }
    else if (isSweeping)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
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
        // Process audio through the filter
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        
        if ((isBypassed && hasPreparedFilter) || isSweeping)
        {
            filter.process (context);
            wetGainProcessor.process (context);
        }
        else
        {
            dryGainProcessor.process (context);
        }
    }
}

void PlaybackManager::updateFilterWithCurve (Curve& curve)
{
    filter.updateWithCurve (curve, FFT_SIZE);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    arbitrarySequencer.setSampleRate (spec.sampleRate);
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

void PlaybackManager::setIsBypassed (bool isBypassed)
{
    this->isBypassed = isBypassed;
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
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += getCompensationDBAtFrequency (node.frequency);
    node.amplitude += getCompensationDBAtFrequency (node.frequency);
    
//    if (node.frequency > 2000.0f)
//        referenceNoteCompensated.gain += 0.5 * std::log2 (node.frequency / 2000.0f);
    
    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
    SequenceableNote note2 (node, noteDurationInSamples);
    arbitrarySequencer.setNotes ({ note1, note2 }, true);
}

void PlaybackManager::updateCalibratingEQNode (EQNode updatedNode)
{
    int noteDurationInSamples = 20000;
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += getCompensationDBAtFrequency (updatedNode.frequency);
    updatedNode.amplitude += getCompensationDBAtFrequency (updatedNode.frequency);
    
//    if (updatedNode.frequency > 2000.0f)
//        referenceNoteCompensated.gain += 0.5 * std::log2 (updatedNode.frequency / 2000.0f);
    
    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
    SequenceableNote note2 (updatedNode, noteDurationInSamples);
    arbitrarySequencer.changeNoteAtIdx (0, note1.note());
    arbitrarySequencer.changeNoteAtIdx (1, note2.note());
}

void PlaybackManager::startTestingFreq (float freq, Curve& curve)
{
    if (isTesting)
    {
        updateTestingFreq (freq, curve);
        return;
    }
    
    int noteDurationInSamples = 25000;
    isTesting = true;
    
    float ampl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq).first.real());
    ampl += getCompensationDBAtFrequency (freq);
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += getCompensationDBAtFrequency (freq);
    
//    if (freq > 2000.0f)
//        referenceNoteCompensated.gain += 0.5 * std::log2 (freq / 2000.0f);
    
    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
    SequenceableNote note2 (freq, ampl, 0.0f, 0.0f, noteDurationInSamples);
    arbitrarySequencer.setNotes ({ note1, note2 });
//    arbitrarySequencer.changeNoteAtIdx (0, note1.note());
//    arbitrarySequencer.changeNoteAtIdx (1, note2.note());
    
    testingFreq = freq;
}

void PlaybackManager::updateTestingFreq (float freq, Curve& curve)
{
    int noteDurationInSamples = 25000;
    
    float ampl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq).first.real());
    ampl += getCompensationDBAtFrequency (freq);
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += getCompensationDBAtFrequency (freq);
    
    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
    SequenceableNote note2 (freq, ampl, 0.0f, 0.0f, noteDurationInSamples);
    arbitrarySequencer.changeNoteAtIdx (0, note1.note());
    arbitrarySequencer.changeNoteAtIdx (1, note2.note());
    
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

std::pair<float, float> PlaybackManager::getNextSample()
{
    return arbitrarySequencer.getNextSample();
}

float PlaybackManager::getCompensationDBAtFrequency (float frequency)
{
    return -4.5f * std::log2 (frequency / REFERENCE_FREQ);
}
