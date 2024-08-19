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
      crossfeedFilter (FFT_SIZE),
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
    myCrossfeedFilter.setCrossfeedGain (0.0f); // Let's disable it for now
    myCrossfeedFilter.setDelay (0.7);
    myCrossfeedFilter.setChannelPlaying (Channel::CENTER);
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
        
        // Apply crossfeed to the calibration
//        crossfeedFilterForCalibration.processBlock (ioBuffer);
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
            myCrossfeedFilter.processBlock (ioBuffer);
            filter.process (ioContext);
//            crossfeedFilter.process (crossfeedContext);
//            mainBlock += crossfeedBlock.multiplyBy (0.5);
//            ioBlock.replaceWithSumOf(mainBlock, crossfeedBlock.multiplyBy (0.0));
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
    crossfeedFilter.updateWithCurves (flatCurve, flatCurve, FFT_SIZE);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    crossfeedFilter.prepare (spec);
//    crossfeedFilterForCalibration.prepare (spec);
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

void PlaybackManager::setCalibratingEQNode (EQNode leftNode, EQNode rightNode, Channel channel)
{
    // Play the reference note and controlled note at the same time
//    int noteDurationInSamples = 20000;
//    node.amplitude += getCompensationDBAtFrequency (node.frequency);
//    SequenceableNote refNote (referenceNote, noteDurationInSamples);
//    SequenceableNote controlledNote (node, noteDurationInSamples);
//    arbitrarySequencer.setNotes ({ refNote });
//    arbitrarySequencer2.setNotes ({ controlledNote });
//    
//    setCrossfeed (channel);
//    crossfeedFilterForCalibration.clear();
    
    // Play the reference chord alternating with the controlled chord on both sides, with no crossfeed for now
    int noteDurationInSamples = 20000;
    
    leftNode.amplitude += getCompensationDBAtFrequency (leftNode.frequency);
    rightNode.amplitude += getCompensationDBAtFrequency (rightNode.frequency);
    
    StereoGainEnvelope hardLeft (StereoGainEnvelopeType::HARD_LEFT);
    StereoGainEnvelope hardRight (StereoGainEnvelopeType::HARD_RIGHT);
    
    SequenceableNote leftControlledNote (leftNode.frequency, leftNode.amplitude, noteDurationInSamples, hardLeft);
    SequenceableNote rightControlledNote (rightNode.frequency, rightNode.amplitude, noteDurationInSamples, hardRight);
    
    SequenceableNote leftReferenceNote1 (referenceFreq1, referenceAmplLeft1, noteDurationInSamples, hardLeft);
    SequenceableNote leftReferenceNote2 (referenceFreq2, referenceAmplLeft2, noteDurationInSamples, hardLeft);
    SequenceableNote rightReferenceNote1 (referenceFreq1, referenceAmplRight1, noteDurationInSamples, hardRight);
    SequenceableNote rightReferenceNote2 (referenceFreq2, referenceAmplRight2, noteDurationInSamples, hardRight);
    
    arbitrarySequencer.setNotes ({ leftReferenceNote1, leftReferenceNote1, rightReferenceNote1, rightReferenceNote1 });
    arbitrarySequencer2.setNotes ({ leftReferenceNote2, leftControlledNote, rightReferenceNote2, rightControlledNote });
    /*
    int noteDurationInSamples = 20000;
    if (channel == Channel::LEFT)
    {
        node.amplitude += getCompensationDBAtFrequency (node.frequency);
        SequenceableNote controlledNote (node.frequency, node.amplitude, noteDurationInSamples);
        controlledNote.applyLeftCrossfeed (0.3, crossfeedDelayInMs);
        
        SequenceableNote referenceNote1 (referenceFreq1, referenceAmplLeft1, noteDurationInSamples);
        SequenceableNote referenceNote2 (referenceFreq2, referenceAmplLeft2, noteDurationInSamples);
        referenceNote1.applyLeftCrossfeed (referenceCrossfeedGainLeft1, crossfeedDelayInMs);
        referenceNote2.applyLeftCrossfeed (referenceCrossfeedGainLeft2, crossfeedDelayInMs);
        
        std::cout << "applying left crossfeed" << std::endl;
        
        arbitrarySequencer.setNotes ({ referenceNote1, referenceNote1 });
        arbitrarySequencer2.setNotes ({ referenceNote2, controlledNote });
    }
    else
    {
        node.amplitude += getCompensationDBAtFrequency (node.frequency);
        SequenceableNote controlledNote (node.frequency, node.amplitude, noteDurationInSamples);
        controlledNote.applyRightCrossfeed (0.3, crossfeedDelayInMs);
        
        SequenceableNote referenceNote1 (referenceFreq1, referenceAmplRight1, noteDurationInSamples);
        SequenceableNote referenceNote2 (referenceFreq2, referenceAmplRight2, noteDurationInSamples);
        referenceNote1.applyRightCrossfeed (referenceCrossfeedGainRight1, crossfeedDelayInMs);
        referenceNote2.applyRightCrossfeed (referenceCrossfeedGainRight2, crossfeedDelayInMs);
        
        arbitrarySequencer.setNotes ({ referenceNote1, referenceNote1 });
        arbitrarySequencer2.setNotes ({ referenceNote2, controlledNote });
//        setCrossfeed (channel);
//        crossfeedFilterForCalibration.clear();
    }
     */
}

void PlaybackManager::updateCalibratingEQNode (EQNode leftNode, EQNode rightNode, Channel channel)
{
    // Play the reference note and controlled note at the same time
//    int noteDurationInSamples = 20000;
//    updatedNode.amplitude += getCompensationDBAtFrequency (updatedNode.frequency);
//    SequenceableNote controlledNote (updatedNode, noteDurationInSamples);
//    arbitrarySequencer.changeNoteAtIdx (0, referenceNote);
//    arbitrarySequencer2.changeNoteAtIdx (0, controlledNote.note());
//    
//    setCrossfeed (channel);
    
    // Play the reference chord alternating with the controlled chord on both sides, with no crossfeed for now
    int noteDurationInSamples = 20000;
    
    leftNode.amplitude += getCompensationDBAtFrequency (leftNode.frequency);
    rightNode.amplitude += getCompensationDBAtFrequency (rightNode.frequency);
    
    StereoGainEnvelope hardLeft (StereoGainEnvelopeType::HARD_LEFT);
    StereoGainEnvelope hardRight (StereoGainEnvelopeType::HARD_RIGHT);
    
    SequenceableNote leftControlledNote (leftNode.frequency, leftNode.amplitude, noteDurationInSamples, hardLeft);
    SequenceableNote rightControlledNote (rightNode.frequency, rightNode.amplitude, noteDurationInSamples, hardRight);
    
    SequenceableNote leftReferenceNote1 (referenceFreq1, referenceAmplLeft1, noteDurationInSamples, hardLeft);
    SequenceableNote leftReferenceNote2 (referenceFreq2, referenceAmplLeft2, noteDurationInSamples, hardLeft);
    SequenceableNote rightReferenceNote1 (referenceFreq1, referenceAmplRight1, noteDurationInSamples, hardRight);
    SequenceableNote rightReferenceNote2 (referenceFreq2, referenceAmplRight2, noteDurationInSamples, hardRight);
    
    arbitrarySequencer.changeNoteAtIdx (0, leftReferenceNote1);
    arbitrarySequencer.changeNoteAtIdx (1, leftReferenceNote1);
    arbitrarySequencer.changeNoteAtIdx (2, rightReferenceNote1);
    arbitrarySequencer.changeNoteAtIdx (3, rightReferenceNote1);
    
    arbitrarySequencer2.changeNoteAtIdx (0, leftReferenceNote2);
    arbitrarySequencer2.changeNoteAtIdx (1, leftControlledNote);
    arbitrarySequencer2.changeNoteAtIdx (2, rightReferenceNote2);
    arbitrarySequencer2.changeNoteAtIdx (3, rightControlledNote);
    
    /*
    int noteDurationInSamples = 20000;
    if (channel == Channel::LEFT)
    {
        node.amplitude += getCompensationDBAtFrequency (node.frequency);
        SequenceableNote controlledNote (node.frequency, node.amplitude, noteDurationInSamples);
        controlledNote.applyLeftCrossfeed (0.3, crossfeedDelayInMs);
        
        SequenceableNote referenceNote1 (referenceFreq1, referenceAmplLeft1, noteDurationInSamples);
        SequenceableNote referenceNote2 (referenceFreq2, referenceAmplLeft2, noteDurationInSamples);
        referenceNote1.applyLeftCrossfeed (referenceCrossfeedGainLeft1, crossfeedDelayInMs);
        referenceNote2.applyLeftCrossfeed (referenceCrossfeedGainLeft2, crossfeedDelayInMs);
        
        arbitrarySequencer.changeNoteAtIdx (0, referenceNote1);
        arbitrarySequencer.changeNoteAtIdx (1, referenceNote1);
        arbitrarySequencer2.changeNoteAtIdx (0, referenceNote2);
        arbitrarySequencer2.changeNoteAtIdx (1, controlledNote);
//        setCrossfeed (channel);
    }
    else
    {
        node.amplitude += getCompensationDBAtFrequency (node.frequency);
        SequenceableNote controlledNote (node.frequency, node.amplitude, noteDurationInSamples);
        controlledNote.applyRightCrossfeed (0.3, crossfeedDelayInMs);
        
        SequenceableNote referenceNote1 (referenceFreq1, referenceAmplRight1, noteDurationInSamples);
        SequenceableNote referenceNote2 (referenceFreq2, referenceAmplRight2, noteDurationInSamples);
        referenceNote1.applyRightCrossfeed (referenceCrossfeedGainRight1, crossfeedDelayInMs);
        referenceNote2.applyRightCrossfeed (referenceCrossfeedGainRight2, crossfeedDelayInMs);
        
        arbitrarySequencer.changeNoteAtIdx (0, referenceNote1);
        arbitrarySequencer.changeNoteAtIdx (1, referenceNote1);
        arbitrarySequencer2.changeNoteAtIdx (0, referenceNote2);
        arbitrarySequencer2.changeNoteAtIdx (1, controlledNote);
//        setCrossfeed (channel);
    }
     */
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
    referenceNoteCompensated.amplitude += getCompensationDBAtFrequency (freq);
    referenceNoteCompensated.amplitude += getReferenceCompensationDBAtFrequency (freq);
    
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
    
    SequenceableNote noteBelow (freqBelow, amplBelow, 0.0f, noteDurationInSamples);
    SequenceableNote noteMid (freq, ampl, 0.0f, noteDurationInSamples);
    SequenceableNote noteAbove (freqAbove, amplAbove, 0.0f, noteDurationInSamples);
    SequenceableNote silentNote (0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope (StereoGainEnvelopeType::SILENT));
    arbitrarySequencer.setNotes ({ noteBelow, noteMid, noteAbove, silentNote });
    
    testingFreq = freq;
}

void PlaybackManager::updateTestingFreq (float freq, Curve& curve)
{
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

void PlaybackManager::setReferenceFreq1 (float freq)
{
    this->referenceFreq1 = freq;
}

void PlaybackManager::setReferenceFreq2 (float freq)
{
    this->referenceFreq2 = freq;
}

void PlaybackManager::setReferenceAmplLeft1 (float ampl)
{
    this->referenceAmplLeft1 = ampl;
}

void PlaybackManager::setReferenceAmplRight1 (float ampl)
{
    this->referenceAmplRight1 = ampl;
}

void PlaybackManager::setReferenceAmplLeft2 (float ampl)
{
    this->referenceAmplLeft2 = ampl;
}

void PlaybackManager::setReferenceAmplRight2 (float ampl)
{
    this->referenceAmplRight2 = ampl;
}

void PlaybackManager::setReferenceCrossfeedGainLeft1 (float gain)
{
    this->referenceCrossfeedGainLeft1 = gain;
}

void PlaybackManager::setReferenceCrossfeedGainRight1 (float gain)
{
    this->referenceCrossfeedGainRight1 = gain;
}

void PlaybackManager::setReferenceCrossfeedGainLeft2 (float gain)
{
    this->referenceCrossfeedGainLeft2 = gain;
}

void PlaybackManager::setReferenceCrossfeedGainRight2 (float gain)
{
    this->referenceCrossfeedGainRight2 = gain;
}

void PlaybackManager::setCrossfeedDelayInMs (float delayInMs)
{
    this->crossfeedDelayInMs = delayInMs;
}

//void PlaybackManager::setCrossfeed (Channel channel)
//{
//    crossfeedFilterForCalibration.setChannelPlaying (channel);
//}

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
