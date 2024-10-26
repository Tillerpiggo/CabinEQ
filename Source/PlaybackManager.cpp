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
    melodyGain = juce::Decibels::decibelsToGain (bandProfile.getMelodyVolume());
    noiseGain = juce::Decibels::decibelsToGain (bandProfile.getNoiseVolume());
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

void PlaybackManager::setMelodyVolume (float melodyVolume)
{
    this->melodyGain = juce::Decibels::decibelsToGain (melodyVolume);
}

void PlaybackManager::setNoiseVolume (float noiseVolume)
{
    this->noiseGain = juce::Decibels::decibelsToGain (noiseVolume);
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
//    noiseSweepGenerator4.setBandwidth (3.0f);
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
//    
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
    
//    // Scales II
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
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // Surround Scales I
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans (5)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 1500, 5000, 1500, 5000, 5000, 1500, 500 })
//        .withBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans (10)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withCyclingPans (3)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    MelodicNotes scaleNotes =
//    MelodicNotes (scale, freq)
//        .withCyclingPans (9);
//    
//    arbitrarySequencer.setNotes (scaleNotes.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // Surround Scales II
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans (5)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 1500, 5000, 1500, 5000, 5000, 1500, 500 })
//        .withBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans (7)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withCyclingPans (3)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    MelodicNotes scaleNotes =
//    MelodicNotes (scale, freq)
//        .withCyclingPans (15);
//    
//    arbitrarySequencer.setNotes (scaleNotes.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
    // Surround Scales III
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans (3)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 1500, 5000, 1500, 5000, 5000, 1500, 500 })
//        .withBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans (3)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withCyclingPans (3)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    MelodicNotes scaleNotes =
//    MelodicNotes (scale, freq)
//        .withCyclingPans (3);
//    
//    arbitrarySequencer.setNotes (scaleNotes.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // Surround Scales IV - weird (but fun) shape
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 500, 1500, 1500, 5000, 5000, 1500, 1500, 5000, 5000, 5000, 5000, 1500, 1500, 500, 500 })
//        .withCyclingBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans ({ -1, 1 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withCyclingPans (5)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    MelodicNotes scaleNotes =
//    MelodicNotes (scale, freq)
//        .withCyclingPans (5);
//    
//    arbitrarySequencer.setNotes (scaleNotes.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // Surround Scales V - more natural but still oddly curved. It's like there are three columns and it's oddly curved between them. Still fun and an obvious improvement
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 500, 1500, 1500, 5000, 5000, 1500, 1500, 5000, 5000, 5000, 5000, 1500, 1500, 500, 500 })
//        .withCyclingBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withCyclingPans (5)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    MelodicNotes scaleNotes =
//    MelodicNotes (scale, freq)
//        .withCyclingPans (5);
//    
//    arbitrarySequencer.setNotes (scaleNotes.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // Surround Scales VI - more natural but still oddly curved. It's like there are three columns and it's oddly curved between them. Still fun and an obvious improvement
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingPans ({ -1, 0, 1})
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 500, 1500, 1500, 5000, 5000, 1500, 1500, 5000, 5000, 5000, 5000, 1500, 1500, 500, 500 })
//        .withCyclingBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withCyclingPans (5)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    MelodicNotes scaleNotes =
//    MelodicNotes (scale, freq)
//        .withCyclingPans (5);
//    
//    arbitrarySequencer.setNotes (scaleNotes.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // Surround Scales VII - really fun and more natural. Still not quite the shape I want but it feels more open, and with that crunchy satisfaction. A bit fatiguing and strange though.
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingPans (5)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 500, 1500, 1500, 5000, 5000, 1500, 1500, 5000, 5000, 5000, 5000, 1500, 1500, 500, 500 })
//        .withCyclingBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans (5)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withCyclingPans (5)
//        .withNoteDurationInSeconds (0.1);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    MelodicNotes scaleNotes =
//    MelodicNotes (scale, freq)
//        .withCyclingPans (5);
//    
//    arbitrarySequencer.setNotes (scaleNotes.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
    // Surround Scales VIII - didn't really work, it makes the melody way too fast, and the soundstage is weird
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 0.5f })
//        .withCyclingPans ({ -1, 1, -1, 0, 1, 0, -1, 1 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 500, 1500, 1500, 5000, 5000, 1500, 1500, 5000, 5000, 5000, 5000, 1500, 1500, 500, 500 })
//        .withCyclingBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans (5)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withCyclingPans ({ -1, 1, -1, 0, 1, 0, -1, 1 })
//        .withNoteDurationInSeconds (0.1);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    MelodicNotes scaleNotes =
//    MelodicNotes (scale, freq)
//        .withCyclingPans ({ -1, 1, -1, 0, 1, 0, -1, 1 })
//        .withNoteDurationInSeconds (0.1);
//    
//    arbitrarySequencer.setNotes (scaleNotes.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
    // Surround Scales IX - still not as good as Percussive I. Makes some notes in the center harder to hear.
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 0.5f })
//        .withCyclingPans (7)
//        .withNoteDurationInSeconds (0.09);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 500, 1500, 1500, 5000, 5000, 1500, 1500, 5000, 5000, 5000, 5000, 1500, 1500, 500, 500 })
//        .withCyclingBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans (5)
//        .withNoteDurationInSeconds (0.12);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withCyclingPans (9)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    MelodicNotes scaleNotes =
//    MelodicNotes (scale, freq)
//        .withCyclingPans ({ -1, 1, -1, 0, 1, 0, -1, 1 });
//    
//    arbitrarySequencer.setNotes (scaleNotes.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
    // Surround Scales X - I really like it, huge bass boost and satisfying boost in the treble as well, but not the perfect shape, still bent in certain weird ways
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 0.5f })
//        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.09);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 500, 1500, 1500, 5000, 5000, 1500, 1500, 5000, 5000, 5000, 5000, 1500, 1500, 500, 500 })
//        .withCyclingBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.12);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    MelodicNotes scaleNotes =
//    MelodicNotes (scale, freq);
//    
//    arbitrarySequencer.setNotes (scaleNotes.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // DiffMelodies I - a really tall signature that is dark, with details overly recessed. Like a dark column.
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
//    std::vector<float> scale { 0, 6, 3, 9, 6, 12, 9, 15, 12, 18, 15, 21, 18, 24 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // DiffMelodies II - very separated, realistic results
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
//    std::vector<float> scale { 0, 12, 24, 19, 5, 7, 17, 19, 24, 12, 0, 12, 24, 12, 0, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // DiffMelodies III
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
////    std::vector<float> scale { 0, 12, 24, 19, 5, 7, 17, 19, 24, 12, 0, 12, 24, 12, 0, 12 };
////    for (int i = 0; i < scale.size(); ++i)
////        scale[i] -= 12;
//    
//    MelodicNotes scale =
//    MelodicNotes::withFreqs ({ 50, 200, 100, 400, 200, 800, 400, 1600, 800, 3200, 1600, 6400, 3200, 12800 });
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // DiffMelodies IV - getting massive results...
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
////    std::vector<float> scale { 0, 12, 24, 19, 5, 7, 17, 19, 24, 12, 0, 12, 24, 12, 0, 12 };
////    for (int i = 0; i < scale.size(); ++i)
////        scale[i] -= 12;
//    
//    MelodicNotes scale =
//    MelodicNotes::withFreqs ({ 100, 800, 6400, 800, 50, 400, 3200, 400, 200, 1600, 12800, 1600, 200, 1600, 12800, 1600 });
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
//    
//    // DiffMelodies V - even bigger but unnaturally tall, perhaps even arced. Stretched out so stuff the should be in the middle is more in my forehead area.
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
////    std::vector<float> scale { 0, 12, 24, 19, 5, 7, 17, 19, 24, 12, 0, 12, 24, 12, 0, 12 };
////    for (int i = 0; i < scale.size(); ++i)
////        scale[i] -= 12;
//    
//    MelodicNotes scale =
//    MelodicNotes::withFreqs ({ 800, 1600, 800, 1600, 400, 3200, 400, 3200, 200, 6400, 200, 6400, 100, 12800, 100, 12800 });
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // DiffMelodies VI - starting to create a wall of sound...
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
////    std::vector<float> scale { 0, 12, 24, 19, 5, 7, 17, 19, 24, 12, 0, 12, 24, 12, 0, 12 };
////    for (int i = 0; i < scale.size(); ++i)
////        scale[i] -= 12;
//    
//    MelodicNotes scale =
//    MelodicNotes::withFreqs ({ 50, 200, 50, 800, 50, 3200, 50, 12800, 100, 400, 100, 1600, 100, 6400, 100, 6400 });
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // DiffMelodies VII - not perfect but so huge and so beautiful and balanced and just... nice.
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
////    std::vector<float> scale { 0, 12, 24, 19, 5, 7, 17, 19, 24, 12, 0, 12, 24, 12, 0, 12 };
////    for (int i = 0; i < scale.size(); ++i)
////        scale[i] -= 12;
//    
//    std::vector<float> scaleSemitones { 50, 100, 200, 400, 800, 1600, 3200, 6400, 12800 };
//    
//    std::vector<float> scaleSemitonesAndFreq;
//    for (int i = 0; i < scaleSemitones.size(); ++i)
//    {
//        scaleSemitonesAndFreq.push_back (freq);
//        scaleSemitonesAndFreq.push_back (scaleSemitones[i]);
//    }
//    
//    MelodicNotes scale =
//    MelodicNotes::withFreqs (scaleSemitonesAndFreq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // Separation II 2
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes ({ -24, 24, -24, 24, -24, 24, -24, 24 }, freq)
//        .withCyclingBandwidths ({ 1.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
////    std::vector<float> scale { 0, 12, 24, 19, 5, 7, 17, 19, 24, 12, 0, 12, 24, 12, 0, 12 };
////    for (int i = 0; i < scale.size(); ++i)
////        scale[i] -= 12;
//    
////    std::vector<float> scaleSemitones { 50, 100, 200, 400, 800, 1600, 3200, 6400, 12800 };
////    
////    std::vector<float> scaleSemitonesAndFreq;
////    for (int i = 0; i < scaleSemitones.size(); ++i)
////    {
////        scaleSemitonesAndFreq.push_back (freq);
////        scaleSemitonesAndFreq.push_back (scaleSemitones[i]);
////    }
//    
////    std::vector<float> scaleVals { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
////    for (int i = 0; i < scaleVals.size(); ++i)
////        scaleVals[i] -= 15;
//    
//    std::vector<float> scaleVals { -2, 0, 2, 0 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
////    spatialPatternGenerator.setPattern (drums.noiseNotes());
////    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
//    spatialPatternGenerator3.setPeakFilter (freq, 1.5f, -12.0f);
    
//    // Separation III
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 8, -8, 8, -8, 8, -8, 8, -8 }, freq)
//        .withBandwidths ({ 0.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 100, 2000 })
//        .withCyclingBandwidths ({ 1.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -2, 0, 2, 0 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
////    spatialPatternGenerator.setPattern (drums.noiseNotes());
////    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
////    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
////    spatialPatternGenerator3.setPeakFilter (freq, 0.5f, -12.0f);
    
//    // Separation IV
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 8, -8, 8, -8, 8, -8, 8, -8 }, freq)
//        .withBandwidths ({ 0.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes ({ -20, 20 }, freq)
//        .withCyclingBandwidths ({ 1.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -2, -1, 0, 1, 2, 1, 0, -1 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
////    spatialPatternGenerator.setPattern (drums.noiseNotes());
////    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
////    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
////    spatialPatternGenerator3.setPeakFilter (freq, 0.5f, -12.0f);
    
//    // Separation V
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 8, -8, 8, -8, 8, -8, 8, -8 }, freq)
//        .withBandwidths ({ 0.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 200, 6000 })
//        .withCyclingBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -2, -1, 0, 1, 2, 1, 0, -1 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
////    spatialPatternGenerator.setPattern (drums.noiseNotes());
////    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
////    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
////    spatialPatternGenerator3.setPeakFilter (freq, 0.5f, -12.0f);
    
//    // Separation VI - pleasant on Yamahas
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 8, -8, 8, -8, 8, -8, 8, -8 }, freq)
//        .withBandwidths ({ 0.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 100, 2000 })
//        .withCyclingBandwidths ({ 1.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -4, 0, 4 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
////    spatialPatternGenerator.setPattern (drums.noiseNotes());
////    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
////    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
////    spatialPatternGenerator3.setPeakFilter (freq, 0.5f, -12.0f);
    
//    // Separation VII - makes it obvious, resists cutting and boosting
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 8, -8, 8, -8, 8, -8, 8, -8 }, freq)
//        .withBandwidths ({ 0.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 100, 2000 })
//        .withCyclingBandwidths ({ 1.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
////    spatialPatternGenerator.setPattern (drums.noiseNotes());
////    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
////    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
////    spatialPatternGenerator3.setPeakFilter (freq, 0.5f, -12.0f);
    
//    // Separation VII - pleasant on Yamahas
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 8, -8, 8, -8, 8, -8, 8, -8 }, freq)
//        .withBandwidths ({ 0.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 100, 2000, 7000 })
//        .withCyclingBandwidths ({ 1.5f, 1.5f, 0.3f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
////    spatialPatternGenerator.setPattern (drums.noiseNotes());
////    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
////    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
////    spatialPatternGenerator3.setPeakFilter (freq, 0.5f, -12.0f);
    
    // Separation VIII
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 8, -8, 8, -8, 8, -8, 8, -8 }, freq)
//        .withBandwidths ({ 0.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 1000, 10000 })
//        .withCyclingBandwidths ({ 0.5f, 0.5f, 0.5f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.4);
    
    // Sweep II - Giving nice results on AKG 702 but idk what i'm doing
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    noiseSweepGenerator.setSweepPattern(SweepPattern(1000, 4.0f, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.2);
////    spatialPatternGenerator.setPattern (drums.noiseNotes());
////    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
////    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
////    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
////    spatialPatternGenerator3.setPeakFilter (freq, 0.5f, -12.0f);
    
//    // Sweep III - getting great results just trying to even it out a bit
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    noiseSweepGenerator.setSweepPattern(SweepPattern(1000, 6.0f, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.2);
////    spatialPatternGenerator.setPattern (drums.noiseNotes());
////    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
////    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
////    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
////    spatialPatternGenerator3.setPeakFilter (freq, 0.5f, -12.0f);
    
//    // Sweep IV
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    noiseSweepGenerator.setSweepPattern(SweepPattern(1000, 6.0f, 1.0f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (2.0);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
//    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, 0.5f, -12.0f);
    
//    // Sweep VII - pretty amazing results with this ngl
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    noiseSweepGenerator.setSweepPattern(SweepPattern(freq, bandwidth * 2.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.1f);
//    noiseSweepGenerator.setPan (1.0f);
    
//    // Sweep VIII - super crisp
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    noiseSweepGenerator.setSweepPattern(SweepPattern(freq, bandwidth * 2.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.1f);
//    noiseSweepGenerator.setPan (0.8f);
    
    // Sweep IX
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    noiseSweepGenerator.setSweepPattern(SweepPattern(freq, bandwidth * 2.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.1f);
//    noiseSweepGenerator.setPan (0.6f);
    
//    // Sweep X
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    noiseSweepGenerator.setSweepPattern(SweepPattern(freq, bandwidth * 2.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.1f);
//    noiseSweepGenerator.setPan (0.0f);
    
//    // DiffMelodies V
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 8100, 900, 2700, 300, 200, 5000, 1200 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ 0 })
//        .withPan (0.5)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 1500, 5000, 1500, 5000, 5000, 1500, 500 })
//        .withBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withPan (0.5)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
//        .withPan (0.5)
//        .withNoteDurationInSeconds (0.05);
//    
////    std::vector<float> scale { 0, 12, 24, 19, 5, 7, 17, 19, 24, 12, 0, 12, 24, 12, 0, 12 };
////    for (int i = 0; i < scale.size(); ++i)
////        scale[i] -= 12;
//    
//    MelodicNotes scale =
//    MelodicNotes::withFreqs ({ 100, 800, 6400, 800, 50, 400, 3200, 400, 200, 1600, 12800, 1600, 200, 1600, 12800, 1600 })
//        .withPan (0.5);
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
//    // Experiments in Noise
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withPattern({ 1, 0, 0, 0, 1, 0, 0, 0 }, 200, 0.8f)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes backgroundNoise2 =
//    MelodicNotes::withPattern({ 0, 0, 1, 0, 0, 0, 1, 0 }, 1000, 0.8f)
//        .withBandwidth (0.8f)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes backgroundNoise3 =
//    MelodicNotes::withPattern({ 1, 0, 0, 1, 0, 0, 1, 0 }, 5000, 0.8f)
//        .withBandwidth (0.8f)
//        .withNoteDurationInSeconds (0.15);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise2.noiseNotes());
//    spatialPatternGenerator3.setPattern (backgroundNoise3.noiseNotes());
////    noiseSweepGenerator.setSweepPattern(SweepPattern(freq, bandwidth * 2.0f, 0.5f, spec.sampleRate));
////    noiseSweepGenerator.setBandwidth (0.1f);
    
//    // Experiments in Noise - got a super nice curve by going for intelligibility of the hits
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withMelodicPattern({ 1, 0, 0, 1, 0, 0, 1, 0 }, { 200, 1000, 5000 }, 1.0f, { 0.0f })
//        .withNoteDurationInSeconds (0.08);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise2.noiseNotes());
//    spatialPatternGenerator3.setPattern (backgroundNoise3.noiseNotes());
//    noiseSweepGenerator.setSweepPattern(SweepPattern(freq, bandwidth * 2.0f, 0.5f, spec.sampleRate));
//    noiseSweepGenerator.setBandwidth (0.1f);
    
//    // Experiments in Noise - very pleasing separation, has the new almost woofy effect that's nice.
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withMelodicPattern({ 1, 0, 0, 1, 0, 0, 1, 0 }, { 1000 }, 1.0f, { 0.0f })
//        .withNoteDurationInSeconds (0.08);
//    
//    MelodicNotes backgroundNoise2 =
//    MelodicNotes::withMelodicPattern({ 1, 0, 1, 0 }, { 200 }, 1.0f, { 0.0f })
//        .withNoteDurationInSeconds (0.08);
//    
//    MelodicNotes backgroundNoise3 =
//    MelodicNotes::withMelodicPattern({ 0, 1, 0, 1 }, { 5000 }, 1.0f, { 0.0f })
//        .withNoteDurationInSeconds (0.08);
//    
//    std::vector<float> scaleVals { -12, 0, 12 };
//    
//    MelodicNotes scale =
//    MelodicNotes (scaleVals, freq);
//    
//    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
//    spatialPatternGenerator.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise2.noiseNotes());
//    spatialPatternGenerator3.setPattern (backgroundNoise3.noiseNotes());
    
    // Experiments in Noise II
    MelodicNotes backgroundNoise =
    MelodicNotes::withMelodicPattern({ 1, 0, 0, 1, 0, 0, 1, 0 }, { 1000 }, 0.5f, { 0.0f })
        .withNoteDurationInSeconds (0.08);
    
    MelodicNotes backgroundNoise2 =
    MelodicNotes::withMelodicPattern({ 1, 0, 1, 0, 1, 0, 1, 0 }, { 200 }, 0.5f, { 0.0f })
        .withNoteDurationInSeconds (0.08);
    
    MelodicNotes backgroundNoise3 =
    MelodicNotes::withMelodicPattern({ 1, 1, 0, 0, 1, 1, 0, 0 }, { 5000 }, 0.5f, { 0.0f })
        .withNoteDurationInSeconds (0.08);
    
    std::vector<float> scaleVals { -12, 0, 12 };
    
    MelodicNotes scale =
    MelodicNotes (scaleVals, freq);
    
    arbitrarySequencer.setNotes (scale.sequenceableNotes(), freq);
    spatialPatternGenerator.setPattern (backgroundNoise.noiseNotes());
    spatialPatternGenerator2.setPattern (backgroundNoise2.noiseNotes());
    spatialPatternGenerator3.setPattern (backgroundNoise3.noiseNotes());
    
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
    auto [leftSample5, rightSample5] = arbitrarySequencer.getNextSample();
    auto [leftSample6, rightSample6] = noiseSweepGenerator.getNextSample();
    auto [leftSample7, rightSample7] = noiseSweepGenerator2.getNextSample();
    auto [leftSample8, rightSample8] = noiseSweepGenerator3.getNextSample();
    auto [leftSample9, rightSample9] = noiseSweepGenerator4.getNextSample();
    
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
    
    leftSample *= noiseGain;
    rightSample *= noiseGain;
    
//    leftSample += leftSample5 * 0.01 * melodyGain;
//    rightSample += rightSample5 * 0.01 * melodyGain;
//    
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
