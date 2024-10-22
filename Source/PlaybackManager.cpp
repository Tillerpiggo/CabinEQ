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

void PlaybackManager::startAmplCalibration (float freq, float bandwidth)
{
//    // Tricky IV
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 300, 900, 2700, 8100 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 1500, 5000 })
//        .withBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
////    std::vector<float> scale { -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5 };
////    
////    std::vector<float> scale2;
////    std::vector<float> scale3;
////    
////    for (const auto& note : scale)
////    {
////        scale2.push_back (note + 12);
////        scale3.push_back (note - 12);
////    }
////    
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
    // With linear up/down sweep, gives oddly neutral results
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 3.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (1000, 5.0f, 0.5f, spec.sampleRate));
    
//    // Sweeps Ia - didn't work as well, because I didn't set up noise sweep generator properly
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth * 2.0f, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (1000, 5.0f, 0.2f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (1000, 1.0f, 0.1f, spec.sampleRate));
    
//    // Sweeps Ib - creates way too big of a dig in the midrange for some reason
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth * 2.0f, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (1000, 3.0f, 0.2f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (1000, 5.0f, 0.1f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (5.0f);
//    noiseSweepGenerator2.setBandwidth (1.0f);
    
//    // Sweeps Ic - really hard to hear what you're doing to the sine sweep exactly
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth * 2.0f, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (1000, 5.0f, 0.2f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (1000, 5.0f, 0.1f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (5.0f);
//    noiseSweepGenerator2.setBandwidth (0.5f);
    
//    // Sweeps Id - changes too subtle (edit: with linear, more natural)
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth * 0.5f, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (1000, 5.0f, 0.2f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (1000, 5.0f, 0.1f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (5.0f);
//    noiseSweepGenerator2.setBandwidth (0.5f);
    
//     Sweeps Ie - changes too subtle (edit: with linear, more natural)
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (1000, 10.0f, 0.2f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (1000, 10.0f, 0.15f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (4.0f);
//    noiseSweepGenerator2.setBandwidth (1.0f);
    
//   // Sweeps If - too tucked back, likely due to high bandwidth
//   sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth * 0.8f, 2.0f, spec.sampleRate));
//   noiseSweepGenerator.setSweepPattern (SweepPattern (1000, 6.0f, 0.2f, spec.sampleRate));
////   noiseSweepGenerator2.setSweepPattern (SweepPattern (1000, 10.0f, 0.15f, spec.sampleRate));
//   noiseSweepGenerator.setBandwidth (4.0f);
////   noiseSweepGenerator2.setBandwidth (1.0f);
    
//    // Sweeps Ig - too tucked back, likely due to high bandwidth
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth * 0.8f, 2.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (1000, 10.0f, 1.0f, spec.sampleRate));
// //   noiseSweepGenerator2.setSweepPattern (SweepPattern (1000, 10.0f, 0.15f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.5f);
// //   noiseSweepGenerator2.setBandwidth (1.0f);
    
//    // Sweeps Ih (paralell) - really really subtle changes
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 2.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 2.0f, spec.sampleRate));
// //   noiseSweepGenerator2.setSweepPattern (SweepPattern (1000, 10.0f, 0.15f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.5f);
// //   noiseSweepGenerator2.setBandwidth (1.0f);
    
//    // Sweeps 2 - pleasing, but a bit muddy/uninspired
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (400, 4.0f, 2.0f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (1600, 4.0f, 1.7f, spec.sampleRate));
//    noiseSweepGenerator3.setSweepPattern (SweepPattern (6400, 4.0f, 2.3f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.5f);
//    noiseSweepGenerator2.setBandwidth (0.5f);
//    noiseSweepGenerator3.setBandwidth (0.5f);
    
//    // Sweeps 2b - more spacious, still pleasing, but quite tucked back
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (400, 4.0f, 2.0f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (1600, 4.0f, 1.7f, spec.sampleRate));
//    noiseSweepGenerator3.setSweepPattern (SweepPattern (6400, 4.0f, 2.3f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (3.0f);
//    noiseSweepGenerator2.setBandwidth (3.0f);
//    noiseSweepGenerator3.setBandwidth (3.0f);
    
//    // Sweeps 2c - has this really calm, pleasant quality and more separation
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (400, 1.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (1600, 1.0f, 0.7f, spec.sampleRate));
//    noiseSweepGenerator3.setSweepPattern (SweepPattern (6400, 1.0f, 0.3f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.5f);
//    noiseSweepGenerator2.setBandwidth (0.5f);
//    noiseSweepGenerator3.setBandwidth (0.5f);
    
//    // Sweeps 2d - also pleasant, and the pleasantness is almost concentrated around the frequency regions
//    // where the noise is at
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (200, 1.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (1000, 1.0f, 0.7f, spec.sampleRate));
//    noiseSweepGenerator3.setSweepPattern (SweepPattern (6000, 1.0f, 0.3f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.2f);
//    noiseSweepGenerator2.setBandwidth (0.2f);
//    noiseSweepGenerator3.setBandwidth (0.2f);
    
//    // Sweeps 2e - also pleasant, especially in the upper mids
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (100, 4.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (800, 4.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator3.setSweepPattern (SweepPattern (3200, 4.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.2f);
//    noiseSweepGenerator2.setBandwidth (0.2f);
//    noiseSweepGenerator3.setBandwidth (0.2f);
    
    // Sweeps 2f - finally an increase in soundstage size, very nice. (with sine: the results are more fun and maybe more muddy)
    // edit: with peak filters, the changes are more subtle, and the result is more expansive - less contortion
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (100, 2.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (800, 2.0f, 0.7f, spec.sampleRate));
//    noiseSweepGenerator3.setSweepPattern (SweepPattern (6400, 2.0f, 0.3f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.2f);
//    noiseSweepGenerator2.setBandwidth (0.2f);
//    noiseSweepGenerator3.setBandwidth (0.2f);
//    noiseSweepGenerator.setPeakFilter (50, 0.3f, 6.0f);
//    noiseSweepGenerator2.setPeakFilter (400, 0.3f, 6.0f);
//    noiseSweepGenerator3.setPeakFilter (3200, 0.3f, 6.0f);
    
//    // Sweeps 2g - really big for some reason? And has this nice crunchiness that's super satisfying
//    // Edit: with the (hard-coded) moving dips, it sounds super spacious and separated
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (100, 2.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (800, 2.0f, 0.7f, spec.sampleRate));
//    noiseSweepGenerator3.setSweepPattern (SweepPattern (6400, 2.0f, 0.3f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.2f);
//    noiseSweepGenerator2.setBandwidth (0.2f);
//    noiseSweepGenerator3.setBandwidth (0.2f);
////    noiseSweepGenerator.setPeakFilter (100, 0.3f, -6.0f);
////    noiseSweepGenerator2.setPeakFilter (800, 0.3f, -6.0f);
////    noiseSweepGenerator3.setPeakFilter (6400, 0.3f, -6.0f);
    
//    // Dips I
//    // Like sweeps 2g but more muddy and larger
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (100, 2.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (800, 2.0f, 0.7f, spec.sampleRate));
//    noiseSweepGenerator3.setSweepPattern (SweepPattern (6400, 2.0f, 0.3f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (1.0f);
//    noiseSweepGenerator2.setBandwidth (1.0f);
//    noiseSweepGenerator3.setBandwidth (1.0f);
////    noiseSweepGenerator.setPeakFilter (100, 0.3f, -6.0f);
////    noiseSweepGenerator2.setPeakFilter (800, 0.3f, -6.0f);
////    noiseSweepGenerator3.setPeakFilter (6400, 0.3f, -6.0f);
    
    // Dips II
    // Like sweeps 2g but more muddy and larger
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    arbitrarySequencer.setNotes (scale, freq);
////    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (100, 2.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (800, 2.0f, 0.7f, spec.sampleRate));
//    noiseSweepGenerator3.setSweepPattern (SweepPattern (6400, 2.0f, 0.3f, spec.sampleRate));
//    noiseSweepGenerator4.setSweepPattern (SweepPattern (1000, 6.0f, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.5f);
//    noiseSweepGenerator2.setBandwidth (0.5f);
//    noiseSweepGenerator3.setBandwidth (0.5f);
    noiseSweepGenerator4.setBandwidth (3.0f);
//    noiseSweepGenerator.setPeakFilter (100, 0.3f, -6.0f);
//    noiseSweepGenerator2.setPeakFilter (800, 0.3f, -6.0f);
//    noiseSweepGenerator3.setPeakFilter (6400, 0.3f, -6.0f);
    
//    // Dips IIb
//    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setSweepPattern (SweepPattern (100, 2.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator2.setSweepPattern (SweepPattern (800, 2.0f, 0.7f, spec.sampleRate));
//    noiseSweepGenerator3.setSweepPattern (SweepPattern (6400, 2.0f, 0.3f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (1.5f);
//    noiseSweepGenerator2.setBandwidth (1.5f);
//    noiseSweepGenerator3.setBandwidth (1.5f);
    
//    // Percussive I
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 1500, 5000, 1500, 5000, 5000, 1500, 500 })
//        .withBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
    
//    std::vector<float> scale { -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5 };
//
//    std::vector<float> scale2;
//    std::vector<float> scale3;
//
//    for (const auto& note : scale)
//    {
//        scale2.push_back (note + 12);
//        scale3.push_back (note - 12);
//    }
//
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
    // Scales II
    bandwidth *= 2.0f;
     
    MelodicNotes drums =
    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
        .withBandwidths ({ 1.0f })
        .withCyclingAmpls ({ 0 })
        .withNoteDurationInSeconds (0.1);
    
    MelodicNotes drums2 =
    MelodicNotes::withFreqs ({ 500, 1500, 5000, 1500, 5000, 5000, 1500, 500 })
        .withBandwidths ({ 4.0f })
        .withCyclingAmpls ({ 0 })
        .withNoteDurationInSeconds (0.1);
    
    MelodicNotes backgroundNoise =
    MelodicNotes::withFreqs ({ 1000 })
        .withBandwidth (10.0f)
        .withNoteDurationInSeconds (0.05);
    
    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
    for (int i = 0; i < scale.size(); ++i)
        scale[i] -= 12;
    
    arbitrarySequencer.setNotes (scale, freq);
    spatialPatternGenerator.setPattern (drums.noiseNotes());
    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
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
//    return sineSweepGenerator.getNextSample();
//    return spatialPatternGenerator.getNextSample();
//    return spatialPinkNoiseGenerator.getNextSample();
//    return wrapperPinkNoiseGenerator.getNextSample();
    
    auto [leftSample1, rightSample1] = spatialPatternGenerator.getNextSample();
    auto [leftSample2, rightSample2] = spatialPatternGenerator2.getNextSample();
    auto [leftSample3, rightSample3] = spatialPatternGenerator3.getNextSample();
    auto [leftSample4, rightSample4] = spatialPatternGenerator4.getNextSample();
//    auto [leftSample5, rightSample5] = sineSweepGenerator.getNextSample();
    auto [leftSample5, rightSample5] = arbitrarySequencer.getNextSample();
    auto [leftSample6, rightSample6] = noiseSweepGenerator.getNextSample();
    auto [leftSample7, rightSample7] = noiseSweepGenerator2.getNextSample();
    auto [leftSample8, rightSample8] = noiseSweepGenerator3.getNextSample();
    auto [leftSample9, rightSample9] = noiseSweepGenerator4.getNextSample();
//    auto [leftSample5, rightSample5] = arbitrarySequencer.getNextSample();
//    auto [leftSample6, rightSample6] = arbitrarySequencer2.getNextSample();
//    auto [leftSample7, rightSample7] = arbitrarySequencer3.getNextSample();
    
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
    
    leftSample += leftSample5 * 0.01;
    rightSample += rightSample5 * 0.01;
    
//    leftSample += leftSample6;
//    rightSample += rightSample6;
//    
//    leftSample += leftSample7;
//    rightSample += rightSample7;
//    
//    leftSample += leftSample8;
//    rightSample += rightSample8;
//    
//    leftSample += leftSample9 * 0.5;
//    rightSample += rightSample9 * 0.5;
    
//    auto [leftSample6, rightSample6] = spatialPatternGenerator6.getNextSample();
    return { leftSample * 30.0, rightSample * 30.0 };
}
