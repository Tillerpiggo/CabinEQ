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
      arbitrarySequencer (std::make_unique<PinkNoiseGenerator> (PinkNoiseGenerator())),
      arbitrarySequencer2 (std::make_unique<PinkNoiseGenerator> (PinkNoiseGenerator())),
      isTesting (false),
      isSweeping (false),
      isCalibrating (false),
      isProcessing (false),
      hasPreparedFilter (false)
{
    dryGainProcessor.setGainDecibels (0.0f);
    wetGainProcessor.setGainDecibels (0.0f);
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
        if ((isProcessing && hasPreparedFilter) || isSweeping)
        {
            filter.process (ioContext);
            wetGainProcessor.process (ioContext);
        }
        else
        {
            dryGainProcessor.process (ioContext);
        }
    }
}

void PlaybackManager::updateFilterWithCurves (Curve& amplCurve, Curve& panCurve)
{
    filter.updateWithCurves (amplCurve, panCurve, FFT_SIZE);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    arbitrarySequencer.setSampleRate (spec.sampleRate);
    arbitrarySequencer2.setSampleRate (spec.sampleRate);
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

void PlaybackManager::setSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl)
{
//    sineSweepGenerator.setCenterFrequency (centerFreq, ampl);
}

void PlaybackManager::updateSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl)
{
//    sineSweepGenerator.updateCenterFrequency (centerFreq, ampl);
}

void PlaybackManager::startPlayingFreq (float freq, Curve& amplCurve, Curve& panCurve)
{
    // Play the reference note and controlled note, alternating between left and right
    int noteDurationInSamples = 30000;
    /* old code
    float ampl = juce::Decibels::gainToDecibels (amplCurve.valueAtFrequency (freq));
    float pan = juce::Decibels::gainToDecibels (panCurve.valueAtFrequency (freq));
    auto nodeBelow = panCurve.nodeBelowFreq (freq);
    auto nodeAbove = panCurve.nodeAboveFreq (freq);
    if (nodeBelow.has_value() && nodeAbove.has_value())
    {
        StereoGainEnvelope fastAttack = StereoGainEnvelope (400);
        auto [freqBelow, panBelow] = nodeBelow.value();
        auto [freqAbove, panAbove] = nodeAbove.value();
        float amplBelow = amplCurve.valueAtFrequency (freqBelow);
        float amplAbove = amplCurve.valueAtFrequency (freqAbove);
        ampl += getCompensationDBAtFrequency (freq);
        amplBelow += getCompensationDBAtFrequency (freqBelow);
        amplAbove += getCompensationDBAtFrequency (freqAbove);
        std::cout << "freqBelow: " << freqBelow << std::endl;
        std::cout << "freq: " << freq << std::endl;
        std::cout << "freqAbove: " << freqAbove << std::endl;
        SequenceableNote controlledNote (freq, ampl, pan, noteDurationInSamples, fastAttack);
        SequenceableNote noteBelow (freqBelow, amplBelow, panBelow, noteDurationInSamples, fastAttack);
        SequenceableNote noteAbove (freqAbove, amplAbove, panAbove, noteDurationInSamples, fastAttack);
        arbitrarySequencer.setNotes ({ noteBelow, controlledNote, noteAbove });
        
        arbitrarySequencer.changeNoteAtIdx (0, noteBelow);
        arbitrarySequencer.changeNoteAtIdx (1, controlledNote);
        arbitrarySequencer.changeNoteAtIdx (2, noteAbove);
    }
     */
//    int noteDurationInSamples = 30000;
    float ampl = juce::Decibels::gainToDecibels (amplCurve.valueAtFrequency (freq));
    float pan = juce::Decibels::gainToDecibels (panCurve.valueAtFrequency (freq));
    auto nodeBelow = panCurve.nodeBelowFreq (freq);
    auto nodeAbove = panCurve.nodeAboveFreq (freq);
    if (nodeBelow.has_value() && nodeAbove.has_value())
    {
        StereoGainEnvelope fastAttack = StereoGainEnvelope (400);
        auto [freqBelow, panBelow] = nodeBelow.value();
        auto [freqAbove, panAbove] = nodeAbove.value();
        float amplBelow = amplCurve.valueAtFrequency (freqBelow);
        float amplAbove = amplCurve.valueAtFrequency (freqAbove);
        ampl += getCompensationDBAtFrequency (freq);
        amplBelow += getCompensationDBAtFrequency (freqBelow);
        amplAbove += getCompensationDBAtFrequency (freqAbove);
        SequenceableNote controlledNote (freq, ampl, pan, noteDurationInSamples, fastAttack);
        SequenceableNote noteBelow (freqBelow, amplBelow, panBelow, noteDurationInSamples, fastAttack);
        SequenceableNote noteAbove (freqAbove, amplAbove, panAbove, noteDurationInSamples, fastAttack);
        arbitrarySequencer.setNotes ({ noteBelow, controlledNote, noteAbove });
    }
//    auto hardLeft = StereoGainEnvelope::hardLeft();
//    auto hardRight = StereoGainEnvelope::hardRight();
//    SequenceableNote leftNote (freq, ampl, pan, noteDurationInSamples, hardLeft);
//    SequenceableNote rightNote (freq, ampl, pan, noteDurationInSamples, hardRight);
//    arbitrarySequencer.setNotes ({ leftNote, rightNote });
}

// TODO: Add panning here
void PlaybackManager::updatePlayingFreq (float freq, Curve& amplCurve, Curve& panCurve)
{
//    int noteDurationInSamples = 20000;
//    ampl += getCompensationDBAtFrequency (freq);
//    StereoGainEnvelope fastAttack = StereoGainEnvelope (400);
//    SequenceableNote refNote (referenceNote, noteDurationInSamples, fastAttack);
//    SequenceableNote controlledNote (freq, ampl, pan, noteDurationInSamples);
//    arbitrarySequencer.changeNoteAtIdx (0, refNote);
//    arbitrarySequencer.changeNoteAtIdx (1, controlledNote);
    
    int noteDurationInSamples = 30000;
    float ampl = juce::Decibels::gainToDecibels (amplCurve.valueAtFrequency (freq));
    float pan = juce::Decibels::gainToDecibels (panCurve.valueAtFrequency (freq));
    auto nodeBelow = panCurve.nodeBelowFreq (freq);
    auto nodeAbove = panCurve.nodeAboveFreq (freq);
    if (nodeBelow.has_value() && nodeAbove.has_value())
    {
        StereoGainEnvelope fastAttack = StereoGainEnvelope (400);
        auto [freqBelow, panBelow] = nodeBelow.value();
        auto [freqAbove, panAbove] = nodeAbove.value();
        float amplBelow = amplCurve.valueAtFrequency (freqBelow);
        float amplAbove = amplCurve.valueAtFrequency (freqAbove);
        ampl += getCompensationDBAtFrequency (freq);
        amplBelow += getCompensationDBAtFrequency (freqBelow);
        amplAbove += getCompensationDBAtFrequency (freqAbove);
        SequenceableNote controlledNote (freq, ampl, pan, noteDurationInSamples, fastAttack);
        SequenceableNote noteBelow (freqBelow, amplBelow, panBelow, noteDurationInSamples, fastAttack);
        SequenceableNote noteAbove (freqAbove, amplAbove, panAbove, noteDurationInSamples, fastAttack);
        arbitrarySequencer.changeNoteAtIdx (0, noteBelow);
        arbitrarySequencer.changeNoteAtIdx (1, controlledNote);
        arbitrarySequencer.changeNoteAtIdx (2, noteAbove);
    }
//    auto hardLeft = StereoGainEnvelope::hardLeft();
//    auto hardRight = StereoGainEnvelope::hardRight();
//    SequenceableNote leftNote (freq, ampl, pan, noteDurationInSamples, hardLeft);
//    SequenceableNote controlledNote (freq, ampl, pan, noteDurationInSamples);
//    SequenceableNote rightNote (freq, ampl, pan, noteDurationInSamples, hardRight);
//    arbitrarySequencer.changeNoteAtIdx (0, leftNote);
//    arbitrarySequencer.changeNoteAtIdx (1, rightNote);
}

void PlaybackManager::startTestingFreq (float freq, Curve& curve)
{
//    if (isTesting)
//    {
//        updateTestingFreq (freq, curve);
//        return;
//    }
//    
//    int noteDurationInSamples = 20000;
//    isTesting = true;
//    
//    float ampl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq));
//    ampl += getCompensationDBAtFrequency (freq);
//    
//    Note referenceNoteCompensated = referenceNote;
//    referenceNoteCompensated.amplitude += getCompensationDBAtFrequency (freq);
//    referenceNoteCompensated.amplitude += getReferenceCompensationDBAtFrequency (freq);
//    
//    auto nodeBelow = curve.nodeBelowFreq (freq);
//    auto nodeAbove = curve.nodeAboveFreq (freq);
//    
//    if (! nodeBelow.has_value() || ! nodeAbove.has_value())
//    {
//        // for now, do nothing
//        return;
//    }
//    
//    auto [freqBelow, amplBelow] = nodeBelow.value();
//    auto [freqAbove, amplAbove] = nodeAbove.value();
//    amplBelow += getCompensationDBAtFrequency (freqBelow);
//    amplAbove += getCompensationDBAtFrequency (freqAbove);
//    
//    SequenceableNote noteBelow (freqBelow, amplBelow, 0.0f, noteDurationInSamples);
//    SequenceableNote noteMid (freq, ampl, 0.0f, noteDurationInSamples);
//    SequenceableNote noteAbove (freqAbove, amplAbove, 0.0f, noteDurationInSamples);
//    SequenceableNote silentNote (0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope (StereoGainEnvelopeType::SILENT));
//    arbitrarySequencer.setNotes ({ noteBelow, noteMid, noteAbove, silentNote });
//    
//    testingFreq = freq;
}

void PlaybackManager::updateTestingFreq (float freq, Curve& curve)
{
    /*
    int noteDurationInSamples = 20000;
    
    float ampl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq).first.real());
    ampl += getCompensationDBAtFrequency (freq);
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.amplitude += getCompensationDBAtFrequency (freq);
    referenceNoteCompensated.amplitude += getReferenceCompensationDBAtFrequency (freq);
    
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
    
    SequenceableNote noteBelow (freqBelow, amplBelow, noteDurationInSamples);
    SequenceableNote noteMid (freq, ampl, noteDurationInSamples);
    SequenceableNote noteAbove (freqAbove, amplAbove, noteDurationInSamples);
    SequenceableNote silentNote (0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope (StereoGainEnvelopeType::SILENT));
    arbitrarySequencer.setNotes ({ noteBelow, noteMid, noteAbove, silentNote });
    
    testingFreq = freq;
    */
}

void PlaybackManager::startSineSweep (float centerFreq, Curve amplCurve, Curve panCurve)
{
    sineSweepGenerator.setSweep (centerFreq, amplCurve, panCurve);
}

void PlaybackManager::updateSineSweep (float centerFreq, Curve amplCurve, Curve panCurve)
{
    sineSweepGenerator.updateSweep (centerFreq, amplCurve, panCurve);
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

void PlaybackManager::setReferencePan (float pan)
{
    this->referencePan = pan;
    this->leftRefNote.amplitude = referenceNote.amplitude - 0.5 * pan;
    this->rightRefNote.amplitude = referenceNote.amplitude + 0.5 * pan;
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    auto [leftSample1, rightSample1] = arbitrarySequencer.getNextSample();
    auto [leftSample2, rightSample2] = arbitrarySequencer2.getNextSample();
    return { leftSample1 + leftSample2, rightSample1 + rightSample2 };
}

float PlaybackManager::getCompensationDBAtFrequency (float frequency)
{
    return -4.5f * std::log2 (frequency / 1000.0f);
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
