/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PlaybackManager.h"
#include "ArbitraryResponseFilter.h"
#include <cmath>
#include <random>

PlaybackManager::PlaybackManager()
    : isCalibrating (false),
      isProcessing (false),
      hasPreparedFilter (false),
      tiltFilter (8),
      arbitrarySequencer (std::make_unique<SineWaveGenerator>()),
      arbitrarySequencer2 (std::make_unique<SineWaveGenerator>()),
      arbitrarySequencer3 (std::make_unique<SineWaveGenerator>())
{
    profileVolumeProcessor.setRampDurationSeconds (0.05);
    profileVolumeProcessor.setGainDecibels (0.0f);
    overallVolumeProcessor.setRampDurationSeconds (0.05);
    overallVolumeProcessor.setGainDecibels (0.0f);
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& ioBuffer)
{
    auto* leftChannel = ioBuffer.getWritePointer(0);
    auto* rightChannel = ioBuffer.getNumChannels() > 1 ? ioBuffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating)
    {
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            std::pair<float, float> value = getNextSample();
            leftChannel[sample] = value.first * 0.15 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.15 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
        }
    }
    
    juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
    juce::dsp::ProcessContextReplacing<float> ioContext(ioBlock);
    
    if (isCalibrating)
    {
        tiltFilter.process (ioContext);
    }
    
    if (isProcessing || isCalibrating)
    {
        filter.process (ioBlock);
        profileVolumeProcessor.process (ioContext);
    }
    
    overallVolumeProcessor.process (ioContext);
}

void PlaybackManager::updateFilterWithBandProfile (BandProfile bandProfile)
{
    filter.setBands (bandProfile.getBands(), spec.sampleRate);
    profileVolumeProcessor.setGainDecibels (bandProfile.getVolume());
//    melodyGain = juce::Decibels::decibelsToGain (bandProfile.getMelodyVolume());
//    noiseGain = juce::Decibels::decibelsToGain (bandProfile.getNoiseVolume());
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    filter.prepare (spec);
    spatialPatternGenerator.prepare (spec);
    spatialPatternGenerator2.prepare (spec);
    spatialPatternGenerator3.prepare (spec);
    spatialPatternGenerator4.prepare (spec);
    spatialPatternGenerator5.prepare (spec);
    spatialPatternGenerator6.prepare (spec);
    arbitrarySequencer.setSampleRate (spec.sampleRate);
    arbitrarySequencer2.setSampleRate (spec.sampleRate);
    arbitrarySequencer3.setSampleRate (spec.sampleRate);
    sineSweepGenerator.setSampleRate (spec.sampleRate);
    spatialPinkNoiseGenerator.setSampleRate (spec.sampleRate);
    wrapperPinkNoiseGenerator.setSampleRate (spec.sampleRate);
    noiseSweepGenerator.prepare (spec);
    noiseSweepGenerator2.prepare (spec);
    noiseSweepGenerator3.prepare (spec);
    noiseSweepGenerator4.prepare (spec);
    melodicNoiseSequencer.prepare (spec);
    tiltFilter.prepare (spec);
    tiltFilter.updateWithCurve (tiltCurve);
    hasPreparedFilter = true;
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
    overallVolumeProcessor.setGainDecibels (volume);
}

void PlaybackManager::setCalibrationVolume (float calibrationVolume)
{
    this->calibrationVolume = calibrationVolume;
}

void PlaybackManager::setSpacing (float spacing)
{
    this->spacing = spacing;
    updateSpatialPatternGenerators();
}

void PlaybackManager::setBandwidth (float bandwidth)
{
    this->bandwidth = bandwidth;
    updateSpatialPatternGenerators();
}

void PlaybackManager::setPitch (float pitch)
{
    this->centerFreq = pitch;
    updateSpatialPatternGenerators();
}

void PlaybackManager::updateSpatialPatternGenerators()
{
//    spatialPatternGenerator.setNoteCenterFreq (centerFreq);
//    spatialPatternGenerator2.setNoteCenterFreq (centerFreq);
//    spatialPatternGenerator.setNoteBandwidth (bandwidth);
//    spatialPatternGenerator2.setNoteBandwidth (bandwidth);
//    spatialPatternGenerator2.setPeakFilter (centerFreq, bandwidth, -30.0f);
//    spatialPatternGenerator3.setNoteCenterFreq (centerFreq * std::pow (2.0f, spacing));
//    spatialPatternGenerator3.setPeakFilter (centerFreq * std::pow (2.0f, spacing), 0.1f, -12.0f);
}

//void PlaybackManager::setMelodyVolume (float melodyVolume)
//{
//    this->melodyGain = juce::Decibels::decibelsToGain (melodyVolume);
//}
//
//void PlaybackManager::setNoiseVolume (float noiseVolume)
//{
//    this->noiseGain = juce::Decibels::decibelsToGain (noiseVolume);
//}

void PlaybackManager::startCalibrationCenteredAt (float freq, float bandwidth)
{
    std::cout << "starting calibration centered at " << std::endl;
//    // Experiments in Noise II
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withMelodicPattern({ 1, 0, 1, 0, 1, 0, 1, 0 }, { 200 }, 0.5f, { 0.0f })
//        .withNoteDurationInSeconds (0.08);
//    
//    MelodicNotes backgroundNoise2 =
//    MelodicNotes::withMelodicPattern({ 1, 0, 0, 1, 0, 0, 1, 0 }, { 1000 }, 0.5f, { 0.0f })
//        .withNoteDurationInSeconds (0.08);
//    
//    MelodicNotes backgroundNoise3 =
//    MelodicNotes::withMelodicPattern({ 1, 1, 0, 0, 1, 1, 0, 0 }, { 5000 }, 0.5f, { 0.0f })
//        .withNoteDurationInSeconds (0.08);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, centerFreq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), centerFreq);
//    spatialPatternGenerator.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise2.noiseNotes());
//    spatialPatternGenerator.setNoteBandwidth (100.0f);
//    spatialPatternGenerator2.setNoteBandwidth (100.0f);
////    spatialPatternGenerator3.setPattern (backgroundNoise3.noiseNotes());
////    spatialPatternGenerator3.setPeakFilter (1000, 0.1f, -12.0f);
    
//    // Works ok but weird, not ideal
//    MelodicNotes melody =
//    MelodicNotes::withMelodicPattern({ 1, 0, 0, 1, 0, 0, 1, 0 }, { 0, 5, 7, 5, 12, 7 }, freq, 1.0f, { 0.0f })
//        .withNoteDurationInSeconds (0.2f);
//    melodicNoiseSequencer.setPattern (melody.noiseNotes());
//    std::cout << "set melodic pattern" << std::endl;
    
//    // Endgame I
//    MelodicNotes melody =
//    MelodicNotes ({ -12, 0, 12, 0, 5, 7, 0, -5 }, freq)
//        .withNoteDurationInSeconds (0.2f);
//    melodicNoiseSequencer.setPattern (melody.noiseNotes());
//    std::cout << "set melodic pattern" << std::endl;
    
//    // Endgame II
//    MelodicNotes melody =
//    MelodicNotes ({ -12, 0, 12, 5, -5, 0, 7, -5 }, freq)
//        .withNoteDurationInSeconds (0.2f);
//    melodicNoiseSequencer.setPattern (melody.noiseNotes());
//    std::cout << "set melodic pattern" << std::endl;
    
//    // Endgame III - not tall enough...
//    MelodicNotes melody =
//    MelodicNotes ({ -12, -7, -5, -2, -1, 0, 2, -1, 0, 5, 7, 9, 11, 12, 14, 12 }, freq)
//        .withNoteDurationInSeconds (0.15f);
//    melodicNoiseSequencer.setPattern (melody.noiseNotes());
//    std::cout << "set melodic pattern" << std::endl;
    
//    // Endgame IV - good, very square, too muddy/kick drum too high up
//    MelodicNotes melody =
//    MelodicNotes ({ -12, -6, 0, 6, 12, 6, 0, -6 }, freq)
//        .withNoteDurationInSeconds (0.15f);
//    melodicNoiseSequencer.setPattern (melody.noiseNotes());
//    std::cout << "set melodic pattern" << std::endl;
    
//    // Endgame V - more natural but still not quite right...
//    MelodicNotes melody =
//    MelodicNotes ({ -12, 0, -11, 1, -10, 2, -9, 3, -8, 4, -7, 5, -6, 6, -5, 7, -4, 8, -3, 9, -2, 10, -1, 11, 0, 12  }, freq)
//        .withNoteDurationInSeconds (0.15f);
//    melodicNoiseSequencer.setPattern (melody.noiseNotes());
//    std::cout << "set melodic pattern" << std::endl;
    
//    // Endgame VI - quite stable, discouraging large changes on small bandwidths... might be on to something, however it's also not encouraging much change at all. The changes are good though.
//    MelodicNotes melody =
//    MelodicNotes ({ -12, -9, -6, -3, 0, 3, 6, 9, 12, 9, 6, 3, 0, -3, -6, -9 }, freq)
//        .withNoteDurationInSeconds (0.15f);
//    melodicNoiseSequencer.setPattern (melody.noiseNotes());
//    std::cout << "set melodic pattern" << std::endl;
    
    // Endgame VI - quite stable, discouraging large changes on small bandwidths... might be on to something, however it's also not encouraging much change at all. The changes are good though.
    MelodicNotes melody =
    MelodicNotes::withFreqs ({ 50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 6400, 3200, 1600, 800, 400, 200, 100 })
        .withNoteDurationInSeconds (0.15f);
    melodicNoiseSequencer.setPattern (melody.noiseNotes());
    std::cout << "set melodic pattern" << std::endl;
}

void PlaybackManager::updateAmplCalibration (float freq, float bandwidth)
{
     
}

void PlaybackManager::setPatternSolo (bool solo)
{
    if (solo)
    {
        isSpatialPatternGeneratorMuted = true;
        isSpatialPatternGenerator2Muted = false;
        isSpatialPatternGenerator3Muted = true;
        isSpatialPatternGenerator4Muted = true;
    }
    else
    {
        isSpatialPatternGeneratorMuted = false;
        isSpatialPatternGenerator2Muted = false;
        isSpatialPatternGenerator3Muted = false;
        isSpatialPatternGenerator4Muted = false;
    }
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

float PlaybackManager::getCurrPlayingFreq()
{
    return arbitrarySequencer.getCurrFreq();
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    pan += 0.0001f;
    if (pan > 1.0f)
        pan = -1.0f;
    float angle = (pan + 1.0f) * M_PI / 4.0f; // Map pan from [-1, 1] to angle [0, π/2]
    float leftAmplitudeCompensation = std::cos(angle);
    float rightAmplitudeCompensation = std::sin(angle);
    
    auto sample = melodicNoiseSequencer.getNextSample();
    auto noiseSample = pinkNoise.generate() * 15.0f;
    noiseSample *= std::abs (leftAmplitudeCompensation - rightAmplitudeCompensation);
    
    return { sample.first * leftAmplitudeCompensation + noiseSample * rightAmplitudeCompensation, sample.second * rightAmplitudeCompensation + noiseSample * leftAmplitudeCompensation };
//    auto [leftSample1, rightSample1] = spatialPatternGenerator.getNextSample();
//    auto [leftSample2, rightSample2] = spatialPatternGenerator2.getNextSample();
//    auto [leftSample3, rightSample3] = spatialPatternGenerator3.getNextSample();
//    
//    return { leftSample1 + leftSample2 + leftSample3, rightSample1 + rightSample2 + rightSample3 };
}
