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
      isCalibrating (false),
      isProcessing (false),
      hasPreparedFilter (false)
{
    gainProcessor.setRampDurationSeconds (0.05);
    gainProcessor.setGainDecibels (0.0f);
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& ioBuffer)
{
    auto* leftChannel = ioBuffer.getWritePointer(0);
    auto* rightChannel = ioBuffer.getNumChannels() > 1 ? ioBuffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating || isProbing)
    {
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            std::pair<float, float> value = getNextSample();
            leftChannel[sample] = value.first * 0.05 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
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
    spatialPatternGenerator3.setSampleRate (spec.sampleRate);
    spatialPatternGenerator4.setSampleRate (spec.sampleRate);
    spatialPatternGenerator5.setSampleRate (spec.sampleRate);
    spatialPatternGenerator6.setSampleRate (spec.sampleRate);
    spatialNoiseGenerator.setSampleRate (spec.sampleRate);
    arbitrarySequencer.setSampleRate (spec.sampleRate);
    arbitrarySequencer2.setSampleRate (spec.sampleRate);
    arbitrarySequencer3.setSampleRate (spec.sampleRate);
    arbitrarySequencer4.setSampleRate (spec.sampleRate);
    arbitrarySequencer5.setSampleRate (spec.sampleRate);
    hasPreparedFilter = true;
}

float PlaybackManager::getCurrPlayingFreq() const
{
    return arbitrarySequencer.currentlyPlayingFrequency();
}

float PlaybackManager::getCurrProbingFreq() const
{
    return probingFreq;
}

void PlaybackManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
}

void PlaybackManager::setIsProcessing (bool isProcessing)
{
    this->isProcessing = isProcessing;
}

void PlaybackManager::setVolume (float volume)
{
    this->volume = volume;
    gainProcessor.setGainDecibels (volume);
}

void PlaybackManager::startAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve)
{
    updateGenerators (amplCurve, panCurve, phaseCurve, freq);
    
    // Instead of spiral, at the same time
    MelodicNotes centerMelody =
    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
        .withBandwidth (7.0)
        .withNoteDurationInSeconds (0.2);
    
    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
}

void PlaybackManager::updateAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve)
{
    spatialPatternGenerator.setAmplCurve (amplCurve);
    spatialPatternGenerator2.setAmplCurve (amplCurve);
    spatialPatternGenerator3.setAmplCurve (amplCurve);
    spatialPatternGenerator4.setAmplCurve (amplCurve);
    spatialPatternGenerator5.setAmplCurve (amplCurve);
    spatialPatternGenerator6.setAmplCurve (amplCurve);
}

void PlaybackManager::startPanCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    updateGenerators (amplCurve, panCurve, phaseCurve, freq);
}
    
void PlaybackManager::updatePanCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    spatialPatternGenerator.setCenterFrequency (freq);
    spatialPatternGenerator2.setCenterFrequency (freq);
    spatialPatternGenerator3.setCenterFrequency (freq);
    spatialPatternGenerator4.setCenterFrequency (freq);
    spatialPatternGenerator.setPanCurve (panCurve);
    spatialPatternGenerator2.setPanCurve (panCurve);
    spatialPatternGenerator3.setPanCurve (panCurve);
    spatialPatternGenerator4.setPanCurve (panCurve);
}

void PlaybackManager::startPhaseCalibration(float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    updateGenerators (amplCurve, panCurve, phaseCurve, freq);
    
    // Phase Calibration I
    MelodicNotes center =
    MelodicNotes ({ 0 }, freq)
        .withBandwidth (3.0)
        .withPan (0.0);
    
    spatialPatternGenerator.setPattern (center.noiseNotes());
}

void PlaybackManager::updatePhaseCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    spatialPatternGenerator.setPhaseCurve (phaseCurve);
    spatialPatternGenerator2.setPhaseCurve (phaseCurve);
    spatialPatternGenerator3.setPhaseCurve (phaseCurve);
    spatialPatternGenerator4.setPhaseCurve (phaseCurve);
    
}

void PlaybackManager::startProbingFreq (float freq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
{
    spatialNoiseGenerator.setBandpass (freq, 2.0, 1.0, 1.0);
    spatialNoiseGenerator.setAmplCurve (amplCurve);
    isProbing = true;
}

void PlaybackManager::updateProbingFreq (float freq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
{
    spatialNoiseGenerator.setBandpass (freq, 2.0, 1.0, 1.0);
}

void PlaybackManager::stopProbing()
{
    isProbing = false;
}

void PlaybackManager::setMutedGenerators (std::vector<bool> mutedGens)
{
    isSpatialPatternGeneratorMuted = mutedGens[0];
    isSpatialPatternGenerator2Muted = mutedGens[1];
    isSpatialPatternGenerator3Muted = mutedGens[2];
    isSpatialPatternGenerator4Muted = mutedGens[3];
}

void PlaybackManager::setReferenceVolume (float volume)
{
    this->referenceVolume = volume;
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    auto [leftSample1, rightSample1] = spatialPatternGenerator.getNextSample();
    auto [leftSample2, rightSample2] = spatialPatternGenerator2.getNextSample();
    auto [leftSample3, rightSample3] = spatialPatternGenerator3.getNextSample();
    auto [leftSample4, rightSample4] = spatialPatternGenerator4.getNextSample();
    auto [leftSampleNoise, rightSampleNoise] = spatialNoiseGenerator.getNextSample();
    
    float leftSample = 0.0f;
    float rightSample = 0.0f;
    
    if (! isSpatialPatternGeneratorMuted)
    {
        leftSample += leftSample1;
        rightSample += rightSample1;
    }
    
    if (! isSpatialPatternGenerator2Muted)
    {
        leftSample += leftSample2;
        rightSample += rightSample2;
    }
    
    if (! isSpatialPatternGenerator3Muted)
    {
        leftSample += leftSample3;
        rightSample += rightSample3;
    }
    
    if (! isSpatialPatternGenerator4Muted)
    {
        leftSample += leftSample4;
        rightSample += rightSample4;
    }
    
    if (isProbing)
    {
        leftSample += leftSampleNoise;
        rightSample += rightSampleNoise;
    }
    
//    auto [leftSample6, rightSample6] = spatialPatternGenerator6.getNextSample();
    return { leftSample, rightSample };
}

void PlaybackManager::updateGenerators (Curve& amplCurve, Curve& panCurve, Curve& phaseCurve, float freq)
{
    std::vector<SpatialPatternGenerator> generators { spatialPatternGenerator, spatialPatternGenerator2, spatialPatternGenerator3, spatialPatternGenerator4 };
    for (auto& generator : generators)
    {
        generator.setAmplCurve (amplCurve);
        generator.setPanCurve (panCurve);
        generator.setPhaseCurve (phaseCurve);
        generator.setCenterFrequency (freq);
    }
}
