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
    : filter (FFT_SIZE), isSweeping (false), isCalibrating (false), isBypassed (false), hasPreparedFilter (false)
{
    dryGainProcessor.setGainDecibels (0.0f);
    wetGainProcessor.setGainDecibels (0.0f);
    
    setCalibratingEQNode (EQNode (-1, 1000.0f, 0.0f, 0.0f)); // placeholder to avoid errors
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& buffer)
{
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const std::pair<float, float> value = getNextSample();
            leftChannel[sample] = value.first * 0.05 * 0.5;
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5;
        }
    }
    else
    {
        if (isSweeping)
        {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                const std::pair<float, float> value = sineSweepGenerator.getNextSample();
                leftChannel[sample] = value.first * 0.05 * 0.5;
                
                if (rightChannel)
                    rightChannel[sample] = value.second * 0.05 * 0.5;
            }
        }
        
        // process audio through the filter
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

void PlaybackManager::updateFilterWithCurve (const Curve& curve)
{
    filter.updateWithCurve (curve, FFT_SIZE);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    arbitrarySequencer.setSampleRate (spec.sampleRate);
    hasPreparedFilter = true;
}

float PlaybackManager::getCurrSineSweepFreq() const
{
    return sineSweepGenerator.getCurrFreq();
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

void PlaybackManager::setSineSweepCenterFrequency (float centerFreq)
{
    sineSweepGenerator.setCenterFrequency (centerFreq);
}

void PlaybackManager::setCalibratingEQNode (EQNode node)
{
    int noteDurationInSamples = 25000;
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += getCompensationDBAtFrequency (node.frequency);
    
    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
    SequenceableNote note2 (node, noteDurationInSamples);
    arbitrarySequencer.setNotes ({ note1, note2 }, true);
}

void PlaybackManager::updateCalibratingEQNode (EQNode updatedNode)
{
    int noteDurationInSamples = 25000;
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.gain += getCompensationDBAtFrequency (updatedNode.frequency);
    
    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
    SequenceableNote note2 (updatedNode, noteDurationInSamples);
    arbitrarySequencer.changeNoteAtIdx (0, note1.note());
    arbitrarySequencer.changeNoteAtIdx (1, note2.note());
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    return arbitrarySequencer.getNextSample();
}

float PlaybackManager::getCompensationDBAtFrequency (float frequency)
{
    // To compensate for music curve
    float amplitudeCompensationGain = std::pow (0.59, std::log2(frequency / 1000.0f));
    float amplitudeCompensationDB = juce::Decibels::gainToDecibels (amplitudeCompensationGain);
    
    // Introduce custom slope for clarity
//    const float referenceFrequency = 1000.0;
//    float slope = 1.7f;
//    float octaves = std::log2((frequency) / (referenceNote.frequency));
//    float dbDifference = octaves * slope;
//    
//    amplitudeCompensationDB += dbDifference;
    
    return amplitudeCompensationDB;
}
