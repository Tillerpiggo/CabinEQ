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
    else if (isTesting) // making testing sweep, for now
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

void PlaybackManager::updateFilterWithCurves (Curve& amplCurve, Curve& panCurve, Curve& phaseCurve, int fftSize)
{
    filter.updateWithCurves (amplCurve, panCurve, phaseCurve, fftSize);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
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

void PlaybackManager::startAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve)
{
    auto refNotes = getNotesForAmplCalibrationAt (1000.0f, 0.0f, true, false);
    auto refNotes2 = getNotesForAmplCalibrationAt (2000.0f, 0.0f, true, false);
    auto controlledNotes = getNotesForAmplCalibration (freq, amplCurve, panCurve, phaseCurve, true, false);
    arbitrarySequencer.setNotes (refNotes);
    arbitrarySequencer2.setNotes (controlledNotes);
    arbitrarySequencer3.setNotes (refNotes2);
}

// TODO: Add panning here
void PlaybackManager::updateAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve)
{
    auto refNotes = getNotesForAmplCalibrationAt (1000.0f, 0.0f, true, false);
    auto refNotes2 = getNotesForAmplCalibrationAt (2000.0f, 0.0f, true, false);
    auto controlledNotes = getNotesForAmplCalibration (freq, amplCurve, panCurve, phaseCurve, true, false);
    arbitrarySequencer.updateNotes (refNotes);
    arbitrarySequencer2.updateNotes (controlledNotes);
    arbitrarySequencer3.updateNotes (refNotes2);
}

void PlaybackManager::startPanCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    float noteDurationInSamples = 16000;
    StereoGainEnvelope envelope (300);
    float pan = panCurve.valueAtFrequency (freq);
    std::vector<SequenceableNote> controlledNotes;
    
    auto pannedNotes = getNotesForAmplCalibrationAt (1000.0f, -2.0f, 0.0f, 0.0f, true, false);
    
    SequenceableNote controlledNote (freq, 10.0f, pan, 0.0f, noteDurationInSamples, envelope);
    SequenceableNote leftControlledNote (freq, 0.0f, pan, 0.0f, noteDurationInSamples, envelope.withPan (-1));
    SequenceableNote rightControlledNote (freq, 0.0f, pan, 0.0f, noteDurationInSamples, envelope.withPan (1));
    SequenceableNote silentNote (0.0f, 0.0f, 0.0f, 0.0f, noteDurationInSamples, envelope);
    
    controlledNotes.push_back (controlledNote);
    controlledNotes.push_back (silentNote);
    
    auto nodeBelow = panCurve.nodeBelowFreq (freq);
    auto nodeAbove = panCurve.nodeAboveFreq (freq);
    
    if (nodeBelow.has_value() && nodeAbove.has_value())
    {
        auto [freqBelow, panBelow] = nodeBelow.value();
        auto [freqAbove, panAbove] = nodeAbove.value();
        
        SequenceableNote noteBelow (freqBelow, 10.0f, panBelow, 0.0f, noteDurationInSamples, envelope);
        SequenceableNote noteAbove (freqAbove, 10.0f, panAbove, 0.0f, noteDurationInSamples, envelope);
        
        controlledNotes.push_back (noteBelow);
        controlledNotes.push_back (silentNote);
        controlledNotes.push_back (noteAbove);
        controlledNotes.push_back (silentNote);
    }

    arbitrarySequencer.setNotes (controlledNotes);
    arbitrarySequencer2.setNotes ({ pannedNotes });
}

void PlaybackManager::updatePanCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    float noteDurationInSamples = 16000;
    StereoGainEnvelope envelope (300);
    float pan = panCurve.valueAtFrequency (freq);
    std::vector<SequenceableNote> controlledNotes;
    
    auto pannedNotes = getNotesForAmplCalibrationAt (1000.0f, -2.0f, 0.0f, 0.0f, true, false);
    
    SequenceableNote controlledNote (freq, 10.0f, pan, 0.0f, noteDurationInSamples, envelope);
    SequenceableNote leftControlledNote (freq, 0.0f, pan, 0.0f, noteDurationInSamples, envelope.withPan (-1));
    SequenceableNote rightControlledNote (freq, 0.0f, pan, 0.0f, noteDurationInSamples, envelope.withPan (1));
    SequenceableNote silentNote (0.0f, 0.0f, 0.0f, 0.0f, noteDurationInSamples, envelope);
    
    controlledNotes.push_back (controlledNote);
    controlledNotes.push_back (silentNote);
    
    auto nodeBelow = panCurve.nodeBelowFreq (freq);
    auto nodeAbove = panCurve.nodeAboveFreq (freq);
    
    if (nodeBelow.has_value() && nodeAbove.has_value())
    {
        auto [freqBelow, panBelow] = nodeBelow.value();
        auto [freqAbove, panAbove] = nodeAbove.value();
        
        SequenceableNote noteBelow (freqBelow, 10.0f, panBelow, 0.0f, noteDurationInSamples, envelope);
        SequenceableNote noteAbove (freqAbove, 10.0f, panAbove, 0.0f, noteDurationInSamples, envelope);
        
        controlledNotes.push_back (noteBelow);
        controlledNotes.push_back (silentNote);
        controlledNotes.push_back (noteAbove);
        controlledNotes.push_back (silentNote);
    }

    arbitrarySequencer.updateNotes (controlledNotes);
    arbitrarySequencer2.updateNotes ({ pannedNotes });
}

void PlaybackManager::startPhaseCalibration(float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // Use the same calibration as pan for now
    float noteDurationInSamples = 2000;
    StereoGainEnvelope envelope (500);
    float pan = panCurve.valueAtFrequency (freq);
    float phase = phaseCurve.valueAtFrequency (freq);
    std::vector<float> ampls { -8.0f, -4.0f, 0.0f, 4.0f, 8.0f, 4.0f, 0.0f, -4.0f };
    std::vector<float> pans { -1.0f, -0.5f, 0.0f, 0.5f, 1.0f, 0.5f, 0.0f, -0.5f, -1.0f };
    std::vector<SequenceableNote> controlledNotes;
    
    int refAmplIdx = 0;
    int controlledAmplIdx = 0;
    SequenceableNote controlledNote (freq, 0.0f, pan, phase, noteDurationInSamples, envelope);
    for (const auto& pan : pans)
    {
        controlledNotes.push_back (controlledNote.withPan (pan).withPhase (phase));
    }
    
    arbitrarySequencer.setNotes (controlledNotes);
}

void PlaybackManager::updatePhaseCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // Use the same calibration as pan for now
    float noteDurationInSamples = 2000;
    StereoGainEnvelope envelope (500);
    float pan = panCurve.valueAtFrequency (freq);
    float phase = phaseCurve.valueAtFrequency (freq);
    std::vector<float> ampls { -8.0f, -4.0f, 0.0f, 4.0f, 8.0f, 4.0f, 0.0f, -4.0f };
    std::vector<float> pans { -1.0f, -0.5f, 0.0f, 0.5f, 1.0f, 0.5f, 0.0f, -0.5f, -1.0f };
    std::vector<SequenceableNote> controlledNotes;
    
    int refAmplIdx = 0;
    int controlledAmplIdx = 0;
    SequenceableNote controlledNote (freq, 0.0f, pan, phase, noteDurationInSamples, envelope);
    for (const auto& pan : pans)
    {
        controlledNotes.push_back (controlledNote.withPan (pan).withPhase (phase));
    }
    
    arbitrarySequencer.updateNotes (controlledNotes);
}

void PlaybackManager::startTestingFreq (float freq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
{
    auto ampl = amplCurve.valueAtFrequency (freq);
    auto pan = panCurve.valueAtFrequency (freq);
    auto phase = phaseCurve.valueAtFrequency (freq);
    SequenceableNote testingNote (freq, ampl, pan, phase, 2000);
    arbitrarySequencer5.setNotes ({ testingNote });
    
    //    std::cout << "start testing freq: " << freq << std::endl;
    //    sineSweepGenerator.updateSweep (freq, freq, amplCurve, panCurve, phaseCurve);
        
}

void PlaybackManager::updateTestingFreq (float freq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
{
    auto ampl = amplCurve.valueAtFrequency (freq);
    auto pan = panCurve.valueAtFrequency (freq);
    auto phase = phaseCurve.valueAtFrequency (freq);
    SequenceableNote testingNote (freq, ampl, pan, phase, 2000);
    arbitrarySequencer5.updateNotes ({ testingNote });
}

//void PlaybackManager::startSineSweep (float centerFreq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
//{
//    sineSweepGenerator.setSweep (centerFreq, centerFreq, amplCurve, panCurve, phaseCurve);
//}
//
//void PlaybackManager::updateSineSweep (float centerFreq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
//{
//    sineSweepGenerator.updateSweep (centerFreq, centerFreq, amplCurve, panCurve, phaseCurve);
//}

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

std::pair<float, float> PlaybackManager::getNextSample()
{
    auto [leftSample0, rightSample0] = arbitrarySequencer.getNextSample();
    auto [leftSample1, rightSample1] = arbitrarySequencer2.getNextSample();
    auto [leftSample2, rightSample2] = arbitrarySequencer3.getNextSample();
    return { leftSample0 + leftSample1 + leftSample2, rightSample0 + rightSample1 + rightSample2 };
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

std::vector<SequenceableNote> PlaybackManager::getNotesForAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve, bool alternateSilence, bool alternateSilenceBefore)
{
    float ampl = amplCurve.valueAtFrequency (freq);
    ampl += getCompensationDBAtFrequency (freq);
    
    float pan = panCurve.valueAtFrequency (freq);
    
    return getNotesForAmplCalibrationAt (freq, ampl, pan, alternateSilence, alternateSilenceBefore, true);
}

std::vector<SequenceableNote> PlaybackManager::getNotesForAmplCalibrationAt (float freq, float ampl, float pan, float phase, bool alternateSilence, bool alternateSilenceBefore, bool changeAmpl)
{
    // Play reference note and then do below - controlled - above
    int noteDurationInSamples = 1000;
    
    StereoGainEnvelope envelope (500);
    
    // Notes that will be used regardless
    SequenceableNote mainNote (freq, ampl, pan, phase, noteDurationInSamples, envelope);
    SequenceableNote silentNote (0.0f, 0.0f, 0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope::silent());
    
    std::vector<SequenceableNote> controlledNotes;
    if (alternateSilence && alternateSilenceBefore)
        controlledNotes.push_back (silentNote);
    controlledNotes.push_back (mainNote);
    if (alternateSilence && ! alternateSilenceBefore)
        controlledNotes.push_back (silentNote);
    
    // Play below - controlled - above while reference note is playing
    std::vector<SequenceableNote> notes;
    std::vector<float> pans { -1, -0.5, 0, 0.5, 1 };
    std::vector<float> ampls { 0.0, 20.0, -10.0, -20.0, 10.0 };
    
    int amplIdx = 0;
    for (const auto& pan : pans)
    {
        for (const auto& note : controlledNotes)
        {
            notes.emplace_back (note.withPan (pan).withAmplitudeChange (ampls[amplIdx]));
            if (changeAmpl)
                amplIdx++;
            if (amplIdx >= ampls.size())
                amplIdx = 0;
        }
    }
    
    
    return notes;
}


std::vector<SequenceableNote> PlaybackManager::getNotesForPanCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve)
{
    float ampl = amplCurve.valueAtFrequency (freq);
    float pan = panCurve.valueAtFrequency (freq);
    float phase = phaseCurve.valueAtFrequency (freq);
    float noteDurationInSamples = 2000;
    StereoGainEnvelope envelope (500);
    
    std::vector<SequenceableNote> controlledNotes;
    std::vector<float> pans { -1, -0.5, 0.0, 0.5, 1 }; // 5 because it's a prime number
    std::vector<float> ampls { -6.0, 0.0, 6.0 }; // 3 because it's a prime number
    int numValues = 7;
    std::vector<float> phases(numValues);
    
    for (int i = 0; i < numValues; ++i)
    {
        phases[i] = -M_PI + i * (2 * M_PI / (numValues - 1)); // 7 because it's a prime number
    }
    
    SequenceableNote controlledNote (freq, ampl, pan, phase, noteDurationInSamples, envelope);
    
    int amplIdx = 0;
    int phaseIdx = 0;
    for (const auto& pan : pans)
    {
        controlledNotes.push_back (controlledNote.withPan (pan));//.withAmplitudeChange (ampls[amplIdx]).withPhase (phases[phaseIdx]));
        amplIdx++;
        phaseIdx++;
        if (amplIdx >= ampls.size())
            amplIdx = 0;
        if (phaseIdx >= phases.size())
            phaseIdx = 0;
    }
    
    return controlledNotes;
}
