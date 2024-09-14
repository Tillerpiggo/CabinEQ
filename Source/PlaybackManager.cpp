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
    gainProcessor.setRampDurationSeconds (0.05);
    gainProcessor.setGainDecibels (0.0f);
    
    // Set spatial pattern generator pattern to be an "X"
    float freqFactorBelow = 0.9;
    float freqFactorAbove = 1 / freqFactorBelow;
    float bandwidth = 1.5;
    float bandwidth2 = 1.5;
    float bandwidth3 = 1.5;
    int durationInSamples = 10000;
    float pan = 1.0;
    float fallingFactor = 0.6;
    float risingFactor = 1 / fallingFactor;
    std::vector<NoiseNote> fallingPattern {
        NoiseNote (fallingFactor, bandwidth, durationInSamples, 0.25),
        NoiseNote (fallingFactor * fallingFactor, bandwidth2, durationInSamples, 0.5),
        NoiseNote (fallingFactor * fallingFactor * fallingFactor, bandwidth3, durationInSamples, 0.75),
        NoiseNote (fallingFactor, bandwidth, durationInSamples, -0.25),
        NoiseNote (fallingFactor * fallingFactor, bandwidth2, durationInSamples, -0.5),
        NoiseNote (fallingFactor * fallingFactor * fallingFactor, bandwidth3, durationInSamples, -0.75)
    };
    std::vector<NoiseNote> risingPattern {
        NoiseNote (risingFactor, bandwidth, durationInSamples, -0.25),
        NoiseNote (risingFactor * risingFactor, bandwidth2, durationInSamples, -0.5),
        NoiseNote (risingFactor * risingFactor * risingFactor, bandwidth3, durationInSamples, -0.75),
        NoiseNote (risingFactor, bandwidth, durationInSamples, 0.25),
        NoiseNote (risingFactor * risingFactor, bandwidth2, durationInSamples, 0.5),
        NoiseNote (risingFactor * risingFactor * risingFactor, bandwidth3, durationInSamples, 0.75)
    };
    
    spatialPatternGenerator.setPattern (fallingPattern);
    spatialPatternGenerator2.setPattern (risingPattern);
    
    // Set spatial pattern generator pattern to be a "V"
//    float freqFactor = 0.85;
//    float freqFactorMain = 0.93;
//    float freqFactorBelow = freqFactor * freqFactorMain;
//    float freqFactorAbove = freqFactorMain / freqFactor;
//    float bandwidth = 0.15;
//    int durationInSamples = 15000;
//    std::vector<NoiseNote> vPattern {
//        NoiseNote (freqFactorAbove, bandwidth, durationInSamples, -1.0),
//        NoiseNote (freqFactorMain, bandwidth, durationInSamples, -0.5),
//        NoiseNote (freqFactorBelow, bandwidth, durationInSamples, 0.0),
//        NoiseNote (freqFactorMain, bandwidth, durationInSamples, 0.5),
//        NoiseNote (freqFactorAbove, bandwidth, durationInSamples, 1.0),
//        NoiseNote (freqFactorMain, bandwidth, durationInSamples, 0.5),
//        NoiseNote (freqFactorBelow, bandwidth, durationInSamples, 0.0),
//        NoiseNote (freqFactorMain, bandwidth, durationInSamples, -0.5)
//    };
//    spatialPatternGenerator.setPattern (vPattern);
    
    // Set spatial pattern generator pattern to diagonal lines ////
//    float freqFactor = 0.6;
//    float freqFactorMain = 1.0;
//    float freqFactorBelow = freqFactor * freqFactorMain;
//    float freqFactorAbove = freqFactorMain / freqFactor;
//    float bandwidth = 0.45;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> diagonalPattern {
//        NoiseNote (freqFactorBelow, bandwidth, durationInSamples, -1.0),
//        NoiseNote (freqFactorMain, bandwidth, durationInSamples, -0.8),
//        NoiseNote (freqFactorAbove, bandwidth, durationInSamples, -0.6),
//    };
//    
//    for (int i = 0; i < 5; ++i)
//    {
//        diagonalPattern.push_back (diagonalPattern[0].withPanChange (i * 0.2));
//        diagonalPattern.push_back (diagonalPattern[1].withPanChange (i * 0.2));
//        diagonalPattern.push_back (diagonalPattern[2].withPanChange (i * 0.2));
//    }
//    
//    spatialPatternGenerator.setPattern (diagonalPattern);
    
    // Set spatial pattern generator pattern to diamond
//    float freqFactor = 0.9;
//    float freqFactorMain = 1.0;
//    float freqFactorBelow = freqFactor * freqFactorMain;
//    float freqFactorAbove = freqFactorMain / freqFactor;
//    float bandwidth = 0.1;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> diamondPattern {
//        NoiseNote (freqFactorAbove, bandwidth, durationInSamples, 0.0),
//        NoiseNote (freqFactorMain, bandwidth, durationInSamples, -0.05),
//        NoiseNote (freqFactorBelow, bandwidth, durationInSamples, 0.0),
//        NoiseNote (freqFactorMain, bandwidth, durationInSamples, 0.05),
//    };
//    spatialPatternGenerator.setPattern (diamondPattern);
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
        juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
        auto ioContext = juce::dsp::ProcessContextReplacing<float> (ioBlock);
        ioContext.isBypassed = ! isProcessing; // convolution will handle the bypass appropriately in it's process method
        filter.process (ioContext);
        gainProcessor.process (ioContext);
    }
}

void PlaybackManager::updateFilterWithCurves (Curve& amplCurve, Curve& panCurve, Curve& phaseCurve, int fftSize)
{
    filter.updateWithCurves (amplCurve, panCurve, phaseCurve, fftSize);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    spatialPatternGenerator.setSampleRate (spec.sampleRate);
    spatialPatternGenerator2.setSampleRate (spec.sampleRate);
//    spatialPatternGenerator.prepare (spec);
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
    gainProcessor.setGainDecibels (isProcessing ? wetVolume : dryVolume);
}

void PlaybackManager::setIsSweeping (bool isSweeping)
{
    this->isSweeping = isSweeping;
}

void PlaybackManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
//    gainProcessor.setGainDecibels (isCalibrating ? wetVolume : dryVolume);
}

void PlaybackManager::setIsProcessing (bool isProcessing)
{
    this->isProcessing = isProcessing;
    gainProcessor.setGainDecibels (isProcessing ? wetVolume : dryVolume);
}

void PlaybackManager::setDryWetVolumeBalance (float balance)
{
//    dryGainProcessor.setGainDecibels (-balance);
//    wetGainProcessor.setGainDecibels (+balance);
}

void PlaybackManager::setWetVolume (float wetVolume)
{
    this->wetVolume = wetVolume;
    gainProcessor.setGainDecibels (isProcessing ? wetVolume : dryVolume);
}

void PlaybackManager::setDryVolume (float dryVolume)
{
    this->dryVolume = dryVolume;
    gainProcessor.setGainDecibels (isProcessing ? wetVolume : dryVolume);
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
//    spatialNoiseGenerator.setAmplCurve (amplCurve);
//    spatialNoiseGenerator.setBandpass (freq, 1.3);
    spatialPatternGenerator.setAmplCurve (amplCurve);
    spatialPatternGenerator.setCenterFrequency (freq);
    spatialPatternGenerator2.setAmplCurve (amplCurve);
    spatialPatternGenerator2.setCenterFrequency (freq);
}

void PlaybackManager::updateAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve)
{
//    spatialNoiseGenerator.setAmplCurve (amplCurve);
//    spatialNoiseGenerator.setBandpass (freq, 1.3);
    spatialPatternGenerator.setAmplCurve (amplCurve);
    spatialPatternGenerator.setCenterFrequency (freq);
    spatialPatternGenerator2.setAmplCurve (amplCurve);
    spatialPatternGenerator2.setCenterFrequency (freq);
}

void PlaybackManager::startPanCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // Taking a break
}

void PlaybackManager::updatePanCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // Taking a break
}

void PlaybackManager::startPhaseCalibration(float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // Taking a break
}

void PlaybackManager::updatePhaseCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // Taking a break
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
    auto [leftSample1, rightSample1] = spatialPatternGenerator.getNextSample();
    auto [leftSample2, rightSample2] = spatialPatternGenerator2.getNextSample();
    return { leftSample1 + leftSample2, rightSample1 + rightSample2 };
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
