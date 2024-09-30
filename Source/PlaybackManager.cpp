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
    spatialPatternGenerator.setAmplCurve (amplCurve);
    spatialPatternGenerator.setCenterFrequency (freq);
    spatialPatternGenerator2.setAmplCurve (amplCurve);
    spatialPatternGenerator2.setCenterFrequency (freq);
    spatialPatternGenerator3.setAmplCurve (amplCurve);
    spatialPatternGenerator3.setCenterFrequency (freq);
    spatialPatternGenerator4.setAmplCurve (amplCurve);
    spatialPatternGenerator4.setCenterFrequency (freq);
    
//    // Melodic Calibration I
//    float bandwidth = 3.0f;
//    spatialPatternGenerator.setMelodicPattern({ -6, 6, 1, 1, -1, 6, -6 -6}, freq, bandwidth, 300);
    
//    // Melodic Calibration II
//    float bandwidth = 5.0f;
//    spatialPatternGenerator.setMelodicPattern({ -6, 6, 1, 1, -1, 6, -6 -6}, freq, bandwidth, 300);
    
    // Melodic Calibration III
//    MelodicNotes undertaleMelody = MelodicNotes({ -6, 6, 1, -1, 6, -6, -6 }, freq).withBandwidth (1.0f);
//    spatialPatternGenerator.setPattern(undertaleMelody.noiseNotes());
    
    // Melodic Calibration IV
//    MelodicNotes patternedMelody = MelodicNotes({ -4, -2, 0, 2, 4 }, freq).withBandwidth (4.0f);
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
//    // Melodic Calibration V
//    MelodicNotes patternedMelody = MelodicNotes({ 0, 4, 8, 12 }, freq).withBandwidth (4.0f);
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
    // Melodic Calibration VI
//    MelodicNotes patternedMelody = MelodicNotes({ 0, 4, 8, 12 }, freq).withBandwidth (2.5f);
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
    // Melodic Calibration VII
//    MelodicNotes patternedMelody = MelodicNotes({ -8, -4, 0, 4, 8 }, freq).withBandwidth (2.0f);
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
//    // Melodic Calibration VIII
//    MelodicNotes patternedMelody = MelodicNotes({ -6, -3, 0, 3, 6 }, freq).withBandwidth (2.0f);
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
    // Melodic Calibration IX
//    MelodicNotes patternedMelody = MelodicNotes({ 0, 3, -1, 0, 0, -4, 3, 0 }, freq).withBandwidth (3.0f);
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        
//    }
    
    // Melodic Calibration X
//    MelodicNotes patternedMelody = MelodicNotes({ -12, -6, 0, 6, 12 }, freq).withBandwidth (4.0f);
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
    // Melodic Calibration XI
//    MelodicNotes patternedMelody = MelodicNotes({ -12, 0, 12, 0, -12, 0, 12, 0 }, freq).withBandwidths ({ 4.5, 4.5, 4.5, 1.0, 4.5, 0.5, 4.5, 3.0 });
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
    // Melodic Calibration XII
//    MelodicNotes patternedMelody = MelodicNotes({ -12, 0, 12, -12, 0, 12 }, freq).withBandwidths ({ 3.0, 3.0, 3.0, 0.5, 0.5, 0.5 });
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
    // Melodic Calibration XIII
//    MelodicNotes patternedMelody = MelodicNotes({ -24, 0, 24, -12, 0, 12 }, freq).withBandwidth (3.0f);
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
    // Melodic Calibration XIV
//    MelodicNotes patternedMelody = MelodicNotes({ -24, 0, 24, -12, 0, 12 }, freq).withBandwidths ({ 4.5, 0.5, 4.5, 3.0, 3.0, 3.0 });
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
    // Melodic Calibration XV
//    MelodicNotes patternedMelody = MelodicNotes({ -24, 0, 24, -12, 0, 12, -18, 0, 18 }, freq).withBandwidths ({ 4.5, 0.5, 4.5, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0 });
//    spatialPatternGenerator.setPattern(patternedMelody.noiseNotes());
    
    // Melodic Calibration XV Spatial
//    MelodicNotes pannedMelody = MelodicNotes({ -24, 0, 24, -12, 0, 12, -18, 0, 18 }, freq).withBandwidths ({ 4.5, 0.5, 4.5, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0 }).withPanCopies ({ -1, 0, 1 });
//    spatialPatternGenerator.setPattern(pannedMelody.noiseNotes());
    
//    // Melodic Calibration XVI Spatial
//    MelodicNotes pannedMelody = MelodicNotes({ -24, 0, 24, -12, 0, 12, -18, 0, 18 }, freq).withBandwidths ({ 4.5, 0.5, 4.5, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0 });
//    spatialPatternGenerator.setPattern(pannedMelody.withPan(-1).noiseNotes());
//    spatialPatternGenerator2.setPattern(pannedMelody.withPan(0).noiseNotes());
//    spatialPatternGenerator3.setPattern(pannedMelody.withPan(1).noiseNotes());
    
    // Melodic Calibration XVII Spatial
//    MelodicNotes pannedMelody = MelodicNotes({ -24, 0, 24, -12, 0, 12, -12, 12 }, freq).withBandwidths ({ 4.5, 0.5, 4.5, 3.0, 3.0, 3.0, 4.5, 4.5 });
//    spatialPatternGenerator.setPattern(pannedMelody.withPan(-1).noiseNotes());
//    spatialPatternGenerator2.setPattern(pannedMelody.withPan(0).noiseNotes());
//    spatialPatternGenerator3.setPattern(pannedMelody.withPan(1).noiseNotes());
    
    // Melodic Calibration XVIII Spatial
//    MelodicNotes pannedMelody = MelodicNotes({ -24, 0, 24, -12, 0, 12, -18, 0, 18, -36, 0 }, freq).withBandwidths ({ 4.5, 0.5, 4.5, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0, 6.0, 6.0 });
//    spatialPatternGenerator.setPattern(pannedMelody.withPan(-1).noiseNotes());
//    spatialPatternGenerator2.setPattern(pannedMelody.withPan(0).noiseNotes());
//    spatialPatternGenerator3.setPattern(pannedMelody.withPan(1).noiseNotes());
    
//    // Melodic Calibration XIX Spatial
//    MelodicNotes pannedMelody = MelodicNotes({ -24, 0, 24, -12, 0, 12, -18, 0, 18, -36, 0 }, freq).withBandwidths ({ 4.5, 0.5, 4.5, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0, 6.0, 6.0 });
//    spatialPatternGenerator.setPattern(pannedMelody.withPan(-1).noiseNotes());
//    spatialPatternGenerator2.setPattern(pannedMelody.withPan(0).withNoteOffset(5).noiseNotes());
//    spatialPatternGenerator3.setPattern(pannedMelody.withPan(1).withNoteOffset(8).noiseNotes());
    
    // Melodic Calibration 2X Spatial
//    MelodicNotes centerMelody =
//    MelodicNotes({ 0, -7, -3, -7, 0, -7, -3, -7, -2, -7, -3, -7, -2, -7, -3, -7 }, freq)
//        .withBandwidth (4.5)
//        .withPan (0);
//    MelodicNotes leftMelody = MelodicNotes ({ -12, -7, -5, -7, -12, -7, -5, -7, -14, -10, -9, -10, -14, -10, -9, -10 }, freq)
//        .withBandwidth (4.5)
//        .withPan (-1);
//    MelodicNotes rightMelody = MelodicNotes ({ -12, -7, -5, -7, -12, -7, -5, -7, -14, -10, -9, -10, -14, -10, -9, -10 }, freq)
//        .withBandwidth (4.5)
//        .withPan (1);
//    spatialPatternGenerator.setPattern(centerMelody.noiseNotes());
//    spatialPatternGenerator2.setPattern(leftMelody.noiseNotes());
//    spatialPatternGenerator3.setPattern(rightMelody.noiseNotes());
    
    // Melodic Calibration 2XI Spatial
//    MelodicNotes centerMelody =
//    MelodicNotes({ 0, -7, -3, -7, 0, -7, -3, -7, -2, -7, -3, -7, -2, -7, -3, -7 }, freq)
//        .withBandwidth (1.5)
//        .withTransposition (5)
//        .withPan (0);
//    
//    MelodicNotes aboveMelody =
//    MelodicNotes({ -12 }, freq)
//        .withBandwidth (1.5)
//        .withPan (0);
//    
//    MelodicNotes belowMelody =
//    MelodicNotes({ 12 }, freq)
//        .withBandwidth (1.5)
//        .withPan (0);
//    spatialPatternGenerator.setPattern(centerMelody.noiseNotes());
//    spatialPatternGenerator2.setPattern(aboveMelody.noiseNotes());
//    spatialPatternGenerator3.setPattern(belowMelody.noiseNotes());
    
//    // Melodic Calibration 2XII Spatial
//    MelodicNotes centerMelody =
//    MelodicNotes({ 0, -7, -3, -7, 0, -7, -3, -7, -2, -7, -3, -7, -2, -7, -3, -7 }, freq)
//        .withBandwidth (1.5)
//        .withTransposition (3)
//        .withPan (0);
//    
//    MelodicNotes aboveMelody =
//    MelodicNotes({ -24 }, freq)
//        .withBandwidth (3.5)
//        .withPan (0);
//    
//    MelodicNotes belowMelody =
//    MelodicNotes({ 24 }, freq)
//        .withBandwidth (3.5)
//        .withPan (0);
//    spatialPatternGenerator.setPattern(centerMelody.noiseNotes());
//    spatialPatternGenerator2.setPattern(aboveMelody.noiseNotes());
//    spatialPatternGenerator3.setPattern(belowMelody.noiseNotes());
    
    // Melodic Calibration 2XIII Spatial
//    MelodicNotes centerMelody =
//    MelodicNotes({ -2, 0, 3, 2 }, freq)
//        .withBandwidth (1.5)
//        .withPan (0);
//    
//    MelodicNotes aboveMelody =
//    MelodicNotes({ -36 }, freq)
//        .withBandwidth (3.5)
//        .withPan (0);
//    
//    MelodicNotes belowMelody =
//    MelodicNotes({ 36 }, freq)
//        .withBandwidth (3.5)
//        .withPan (0);
//    spatialPatternGenerator.setPattern(centerMelody.noiseNotes());
//    spatialPatternGenerator2.setPattern(aboveMelody.noiseNotes());
//    spatialPatternGenerator3.setPattern(belowMelody.noiseNotes());
    
    // Melodic Calibration 2XIV Spatial
//    MelodicNotes centerMelody =
//    MelodicNotes({ 0, -7, -3, -7, 0, -7, -3, -7, -2, -7, -3, -7, -2, -7, -3, -7 }, freq)
//        .withBandwidth (1.5)
//        .withTransposition (5)
//        .withPan (0);
//    
//    MelodicNotes aboveMelody =
//    MelodicNotes({ -12 }, freq)
//        .withBandwidth (1.5)
//        .withPan (0);
//    
//    MelodicNotes belowMelody =
//    MelodicNotes({ 12 }, freq)
//        .withBandwidth (1.5)
//        .withPan (0);
//    spatialPatternGenerator.setPattern(centerMelody.noiseNotes());
//    spatialPatternGenerator2.setPattern(aboveMelody.noiseNotes());
//    spatialPatternGenerator3.setPattern(belowMelody.noiseNotes());
    
    // Spatial 2XV
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.5)
//        .withPan (0);
//    MelodicNotes leftMelody =
//    MelodicNotes({ 36, 24, 12, 0, -12 }, freq)
//        .withBandwidth (4.5)
//        .withPan (-1);
//    MelodicNotes rightMelody =
//    MelodicNotes({ 36, 24, 12, 0, -12 }, freq)
//        .withBandwidth (4.5)
//        .withPan (1);
//    
//    spatialPatternGenerator.setPattern(centerMelody.noiseNotes());
//    spatialPatternGenerator2.setPattern(leftMelody.noiseNotes());
//    spatialPatternGenerator3.setPattern(rightMelody.noiseNotes());
    
//    // Spatial 2XVI
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.5)
//        .withCyclingPans ({ -1, 0, 1 });
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
    // Spatial 2V
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (6.0)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
//    // Spatial 2Y
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.5)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
//    // Spatial 2Y4
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.5)
//        .withTransposition (-24)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
    // Spatial 2Y5
//    MelodicNotes centerMelody =
//    MelodicNotes({ -24, 0, -12, 0, 12, 0, 24, 0 }, freq)
//        .withBandwidth (4.5)
//        .withTransposition (-24);
////        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//    spatialPatternGenerator2.setPattern(centerMelody.withPan(-1).noiseNotes());
//    spatialPatternGenerator3.setPattern(centerMelody.withPan(1).noiseNotes());
    
//    // Spatial 2Y6
//    MelodicNotes centerMelody =
//    MelodicNotes({ -36, -24, -12, 0, 12, 24, 36 }, 1000)
//        .withBandwidth (4.5);
////        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//    spatialPatternGenerator2.setPattern(centerMelody.withPan(-1).noiseNotes());
//    spatialPatternGenerator3.setPattern(centerMelody.withPan(1).noiseNotes());
    
//    // Spatial 2Y7
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, -7, -5, 0, 5, 7, 12 }, freq)
//        .withBandwidth (3.0)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
//    // Spatial 2Y8
//    MelodicNotes centerMelody =
//    MelodicNotes({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.5)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds(0.1);
//    
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
//    // Spatial 2Y9 (dynamic)
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float ratioInOctaves = std::log2 (freqAbove / freqBelow);
//        float freqRatioInSemitones = ratioInOctaves * 12;
//        freqRatioInSemitones *= 0.5;
//        std::vector<float> melody { -2, -1, 0, 1, 2 };
//        for (int i = 0; i < melody.size(); ++i)
//            melody[i] *= freqRatioInSemitones;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes (melody, freq)
//            .withBandwidth (ratioInOctaves * 1.5)
//            .withCyclingPans ({ -1, 0, 1 })
//            .withNoteDurationInSeconds (0.2);
//        
//        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//    }
//    else
//    {
////        MelodicNotes centerMelody =
////        MelodicNotes ({ -48, -24, 0, 24, 48 }, freq)
////            .withBandwidth (6.0)
////            .withCyclingPans ({ -1, 0, 1 })
////            .withNoteDurationInSeconds (0.2);
////        
////        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//    }
//    
    
    // Spatial 2Y9D2
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float ratioInOctaves = std::log2 (freqAbove / freqBelow);
//        float freqRatioInSemitones = ratioInOctaves * 12;
//        freqRatioInSemitones *= 0.15;
//        std::vector<float> melody { -2, -1, 0, 1, 2, 5, 6, 7, 8, 9 };
//        for (int i = 0; i < melody.size(); ++i)
//            melody[i] *= freqRatioInSemitones;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes (melody, freq)
//            .withBandwidth (ratioInOctaves * 0.8)
//            .withTransposition (ratioInOctaves * -7)
//            .withNoteDurationInSeconds (0.2);
//        
//        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
//    else
//    {
////        MelodicNotes centerMelody =
////        MelodicNotes ({ -48, -24, 0, 24, 48 }, freq)
////            .withBandwidth (6.0)
////            .withCyclingPans ({ -1, 0, 1 })
////            .withNoteDurationInSeconds (0.2);
////
////        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//    }
    
//    // Spatial Edges 3
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float ratioInOctaves = std::log2 (freqAbove / freqBelow);
//        float freqRatioInSemitones = ratioInOctaves * 12;
//        float scale = 0.5;
//        freqRatioInSemitones *= 0.8 * scale;
//        std::vector<float> melody { -2, -1, 0, 1, 2 };
//        for (int i = 0; i < melody.size(); ++i)
//            melody[i] *= freqRatioInSemitones;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes (melody, freq)
//            .withBandwidth (ratioInOctaves * 0.5 * scale)
////            .withTransposition (ratioInOctaves * -7)
//            .withNoteDurationInSeconds (0.2);
//        
//        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
//    else
//    {
////        MelodicNotes centerMelody =
////        MelodicNotes ({ -48, -24, 0, 24, 48 }, freq)
////            .withBandwidth (6.0)
////            .withCyclingPans ({ -1, 0, 1 })
////            .withNoteDurationInSeconds (0.2);
////
////        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//    }
    
    // Spatial Edges 4
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float ratioInOctaves = std::log2 (freqAbove / freqBelow);
//        float freqRatioInSemitones = ratioInOctaves * 12;
//        float scale = 0.3;
//        freqRatioInSemitones *= 0.8 * scale;
//        std::vector<float> melody { -2, -1, 0, 1, 2 };
//        for (int i = 0; i < melody.size(); ++i)
//            melody[i] *= freqRatioInSemitones;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes (melody, freq)
//            .withBandwidth (ratioInOctaves * 4.0 * scale)
////            .withTransposition (ratioInOctaves * -7)
//            .withNoteDurationInSeconds (0.2);
//        
//        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
//    else
//    {
////        MelodicNotes centerMelody =
////        MelodicNotes ({ -48, -24, 0, 24, 48 }, freq)
////            .withBandwidth (6.0)
////            .withCyclingPans ({ -1, 0, 1 })
////            .withNoteDurationInSeconds (0.2);
////
////        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//    }
    
//    // Spatial Edges 5
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float ratioInOctaves = std::log2 (freqAbove / freqBelow);
//        float freqRatioInSemitones = ratioInOctaves * 12;
//        float scale = 0.3;
//        freqRatioInSemitones *= 0.8 * scale;
//        std::vector<float> melody { -2, -1, 0, 1, 2 };
//        for (int i = 0; i < melody.size(); ++i)
//            melody[i] *= freqRatioInSemitones;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes (melody, freq)
//            .withBandwidth (ratioInOctaves * 5.0 * scale)
//            .withCyclingPans({ -1, 0, 1 })
//            .withNoteDurationInSeconds (0.2);
//        
//        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
////        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
////        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
//    else
//    {
////        MelodicNotes centerMelody =
////        MelodicNotes ({ -48, -24, 0, 24, 48 }, freq)
////            .withBandwidth (6.0)
////            .withCyclingPans ({ -1, 0, 1 })
////            .withNoteDurationInSeconds (0.2);
////
////        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//    }
    
    
    
//
//    // Pink Noise 2XIII
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 3), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<float> pans { -1.0, -0.5, 0.0, 0.5, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 0.1, durationInSamples / 4.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 3.0, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 3.0, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 3.0, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
    
    // Spatial N1
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.5)
//        .withTransposition (8)
//        .withCyclingPans ({ -1, 0, 1 });
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
    // Spatial N2
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (5.5)
//        .withTransposition (8)
//        .withCyclingPans ({ -1, 0, 1 });
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
//    // Spatial N3
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (5.5)
//        .withTransposition (8)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
    // Spatial N3
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (5.5)
//        .withTransposition (8)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
    // Spatial N4 (quite good)
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, -5, 12, 24, 19, 36, 31 }, freq)
//        .withBandwidth (5.5)
//        .withTransposition (8)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
//    // Spatial N5 (experiment) - worked really well!!
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, -5, 12, 24, 19, 36, 31 }, freq)
//        .withBandwidth (1.0)
//        .withTransposition (8)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes noiseMelody =
//    MelodicNotes({ 0, 0, 0}, 1000)
//        .withBandwidth (100.0)
//        .withCyclingPans ({ -1, 0, 1 });
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//    spatialPatternGenerator2.setPattern (noiseMelody.noiseNotes());
    
    // Spirals I (experiment) (works well)
//    MelodicNotes spiral1 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (3.0)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes spiral2 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
    
//    // Spirals II (experiment)
//    MelodicNotes spiral1 =
//    MelodicNotes({ -12, 0, 12, 24, 36, 30, 18, 6, -6 }, freq)
//        .withBandwidth (3.0)
//        .withCyclingPans ({ -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1,
//                            0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 });
    
//    MelodicNotes spiral2 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
    
    // Spirals III (experiment)
//    MelodicNotes spiral1 =
//    MelodicNotes({ -12, 0, 12, 24, 36, 24, 12, 0 }, freq)
//        .withBandwidth (3.0)
//        .withSegmentedPans (9, true);
//    
//    MelodicNotes spiral2 =
//    MelodicNotes({ -12, 0, 12, 24, 36, 24, 12, 0 }, freq)
//        .withBandwidth (1.0)
//        .withSegmentedPans (9, false);
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
    
    // Soundstage I (works ok)
//    MelodicNotes sides =
//    MelodicNotes ({ -12, -12, 0, 0, 12, 12, 24, 24, 36, 36 }, freq)
//        .withBandwidth (2.0)
//        .withCyclingPans ({ -1, 1 });
//    
//    MelodicNotes partials =
//    MelodicNotes ({ -12, -12, 0, 0, 5, 5, 12, 12 }, freq)
//        .withBandwidth (1.0)
//        .withCyclingPans ({ -0.5, 0.5 });
//    
//    MelodicNotes center =
//    MelodicNotes ({ 36, 24, 12, 0, -12 }, freq)
//        .withBandwidth (3.0);
//    
//    spatialPatternGenerator.setPattern (sides.noiseNotes());
//    spatialPatternGenerator2.setPattern (partials.noiseNotes());
//    spatialPatternGenerator3.setPattern (center.noiseNotes());
        
    // Spirals IV
//    MelodicNotes spiral1 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (3.0)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes spiral2 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    MelodicNotes spiral3 =
//    MelodicNotes({ -12, 0, 12, 24 }, freq)
//        .withBandwidth (4.5)
//        .withTransposition (12)
//        .withNoteOffset (3)
//        .withCyclingPans ({ 1, 0.5, 0, -0.5, -1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
//    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
    // 3 Melodies
//    MelodicNotes leftMelody =
//    MelodicNotes({ 12, -12, 0, 24 }, freq)
//        .withBandwidth (2.0)
//        .withPan (1);
//    
//    MelodicNotes rightMelody =
//    MelodicNotes({ 0, 24, 12, -12 }, freq)
//        .withBandwidth (2.0)
//        .withPan (-1);
//    
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24 }, freq)
//        .withBandwidth (2.0);
//    
//    spatialPatternGenerator.setPattern (leftMelody.noiseNotes());
//    spatialPatternGenerator2.setPattern (rightMelody.noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.noiseNotes());
    
    // Spirals V
//    MelodicNotes spiral1 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (3.0)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 })
//        .withTransposition (-36);
//    
//    MelodicNotes spiral2 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 })
//        .withTransposition (-36);
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
    
//    // Spirals VI
//    MelodicNotes spiral1 =
//    MelodicNotes({ -12, -7, 0, 7, 12, 19, 24, 31, 36, 31 }, freq)
//        .withBandwidth (1.0)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 })
//        .withTransposition (-12)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes spiral2 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (3.0)
//        .withTransposition (-12)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 })
//        .withTransposition (-12)
//        .withNoteDurationInSeconds (0.3);
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
    
    // Spirals VII (works great!)
//    MelodicNotes spiral1 =
//    MelodicNotes({ -12, -7, 0, 7, 12, 19, 24, 31, 36, 31 }, freq)
//        .withBandwidth (0.5)
//        .withPans ({ -1, -0.5, 0, 0.5, 1 })
//        .withTransposition (-12)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes spiral2 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (4.5)
//        .withTransposition (-12)
//        .withPans ({ -1, 0, 1 })
//        .withTransposition (-12)
//        .withNoteDurationInSeconds (0.3);
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
    
    // 3 Melody Problem II
//    MelodicNotes spiral1 =
//    MelodicNotes({ -12, -7, 0, 7, 12, 19, 24, 31, 36, 31 }, freq)
//        .withBandwidth (0.5)
//        .withTransposition (-12)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes spiral2 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (4.5)
//        .withTransposition (-12)
//        .withTransposition (-12)
//        .withNoteDurationInSeconds (0.3);
//    
//    spatialPatternGenerator.setPattern (spiral1.withPan (-1).noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral1.withPan (1).noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
    
    // Spirals VII
//    MelodicNotes spiral1 =
//    MelodicNotes({ -12, -5, 0, 7, 12, 19, 24, 31, 36, 31 }, freq)
//        .withBandwidth (1.0)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 })
//        .withTransposition (-12)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes spiral2 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (3.0)
//        .withTransposition (-12)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 })
//        .withTransposition (-12)
//        .withNoteDurationInSeconds (0.3);
//    
//    MelodicNotes spiral3 =
//    MelodicNotes({ -12, -8, -5, 0, 4, 7, 12, 16, 19, 24, 28, 31, 36, 31, 28 }, freq)
//        .withBandwidth (3.0)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 })
//        .withTransposition (-12)
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
//    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
    // 3 Spiral Problem (maybe the best??)
//    MelodicNotes spiral1 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (3.0)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes spiral2 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    MelodicNotes spiral3 =
//    MelodicNotes({ -12, 0, 12, 24, 36}, freq)
//        .withBandwidth (2.0)
//        .withTransposition (-9)
//        .withNoteOffset (4)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
//    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
//    // 3 Spiral Problem 2 (experimental)
//    std::vector<float> melody { -12, 0, 7, 5, 12, 24, 7, 5, 36, 24, 12, 5, 7 };
//    MelodicNotes spiral1 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes spiral2 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    MelodicNotes spiral3 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (2.0)
//        .withTransposition (-9)
//        .withNoteOffset (4)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
//    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
//    // 3 Spiral Problem 4 (experimental)
//    std::vector<float> melody { -12, 12, 7, 5, 0, 24, 19, 17, 12, 36, 31, 29, 24, 12, -12, 0 };
//    MelodicNotes spiral1 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes spiral2 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    MelodicNotes spiral3 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (2.0)
//        .withTransposition (-9)
//        .withNoteOffset (4)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
//    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
//    // 3 Spiral Problem 5 (experimental)
//    std::vector<float> melody { -12, 12, 7, 5, 0, 24, 19, 17 };
//    int transposition = 6;
//    MelodicNotes spiral1 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes spiral2 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    MelodicNotes spiral3 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (2.0)
//        .withTransposition (-9)
//        .withNoteOffset (4)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
//    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
//    // 3 Spiral Problem VI (experimental)
//    std::vector<float> melody { -12, 12, 24, 19, 0, 24, 36, 31 };
//    int transposition = 6;
//    MelodicNotes spiral1 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes spiral2 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    MelodicNotes spiral3 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (2.0)
//        .withTransposition (-9)
//        .withNoteOffset (4)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
//    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
//    // 3 Spiral Problem V (experimental)
//    std::vector<float> melody { -12, 12, 24, 19, 0, 24, 36, 31 };
//    int transposition = 6;
//    MelodicNotes spiral1 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes spiral2 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    MelodicNotes spiral3 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (2.0)
//        .withTransposition (-9)
//        .withNoteOffset (4)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotesWithInterspersedReference (1000));
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotesWithInterspersedReference (1000));
//    spatialPatternGenerator3.setPattern (spiral3.noiseNotesWithInterspersedReference (1000));
    
    // 3 Spiral Problem VI (experimental)
//    std::vector<float> melody { -24, -12, 24, 0, -24, 12, 24, 0 };
//    int transposition = 0;
//    MelodicNotes spiral1 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes spiral2 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    MelodicNotes spiral3 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (2.0)
//        .withTransposition (-9)
//        .withNoteOffset (4)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotesWithInterspersedReference (1000));
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotesWithInterspersedReference (1000));
//    spatialPatternGenerator3.setPattern (spiral3.noiseNotesWithInterspersedReference (1000));
    
//    // Spirals VII
//    std::vector<float> melody { -24, -12, 24, 0, -24, 12, 24, 0 };
//    int transposition = 0;
//    MelodicNotes spiral1 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, -0.66, -0.33, 0, 0.33, 0.66, 1, 0.66, 0.33, 0, -0.33, -0.66 });
//    
//    MelodicNotes spiral2 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (1.0)
//        .withTransposition (-6)
//        .withNoteOffset (3)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
//    
//    MelodicNotes spiral3 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (2.0)
//        .withTransposition (-9)
//        .withNoteOffset (4)
//        .withTransposition (transposition)
//        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
//    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
//    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
////    // Spirals VIII
//    std::vector<float> melody { -24, -12, 24, 0, -24, 12, 24, 0 };
//    int transposition = 0;
//    MelodicNotes spiral1 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withTransposition (transposition)
////        .withPan (0);
//        .withCyclingPans ({ -1, 0, 1 });
//
////    MelodicNotes spiral2 =
////    MelodicNotes(melody, freq)
////        .withBandwidth (1.0)
////        .withTransposition (-6)
////        .withNoteOffset (3)
////        .withTransposition (transposition)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
////    
////    MelodicNotes spiral3 =
////    MelodicNotes(melody, freq)
////        .withBandwidth (2.0)
////        .withTransposition (-9)
////        .withNoteOffset (4)
////        .withTransposition (transposition)
////        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
////    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
////    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
    // Reference I
//    std::vector<float> melody { -24, -12, 24, 0, -24, 12, 24, 0 };
//    int transposition = 0;
//    MelodicNotes spiral1 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withTransposition (transposition)
////        .withPan (0);
//        .withCyclingPans ({ 0, -1, 0, 0, 0, 1 });
//    
////    MelodicNotes spiral2 =
////    MelodicNotes(melody, freq)
////        .withBandwidth (1.0)
////        .withTransposition (-6)
////        .withNoteOffset (3)
////        .withTransposition (transposition)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
////
////    MelodicNotes spiral3 =
////    MelodicNotes(melody, freq)
////        .withBandwidth (2.0)
////        .withTransposition (-9)
////        .withNoteOffset (4)
////        .withTransposition (transposition)
////        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
////    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
////    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
    // Spirals X
//    std::vector<float> melody { -24, -12, 24, 0, -24, 12, 24, 0 };
//    int transposition = 0;
//    MelodicNotes spiral1 =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withTransposition (transposition)
////        .withPan (0);
//        .withCyclingPans ({ -1, 0, 1 });
//
////    MelodicNotes spiral2 =
////    MelodicNotes(melody, freq)
////        .withBandwidth (1.0)
////        .withTransposition (-6)
////        .withNoteOffset (3)
////        .withTransposition (transposition)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 });
////
////    MelodicNotes spiral3 =
////    MelodicNotes(melody, freq)
////        .withBandwidth (2.0)
////        .withTransposition (-9)
////        .withNoteOffset (4)
////        .withTransposition (transposition)
////        .withCyclingPans ({ -1, 0, 1 });
//    
//    spatialPatternGenerator.setPattern (spiral1.noiseNotes());
////    spatialPatternGenerator2.setPattern (spiral2.noiseNotes());
////    spatialPatternGenerator3.setPattern (spiral3.noiseNotes());
    
//    // The Wall I
//    std::vector<float> melody { -24, 0, -24, 0, -24, 0, 0, 0, 24, 0, 24, 0, 24, 0, 0, 0 };
//    int transposition = 0;
//    MelodicNotes theWall =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.0)
//        .withTransposition (transposition)
//        .withPans ({ -1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, -1, 0, -1, 0 });
//    
//    spatialPatternGenerator.setPattern (theWall.noiseNotes());
    
//    // The Wall II
//    std::vector<float> melody { -24, 0, -24, 0, 24, 0, 24, 0 };
//    int transposition = 0;
//    MelodicNotes theWall =
//    MelodicNotes(melody, freq)
//        .withBandwidth (4.5)
//        .withTransposition (transposition)
//        .withPans ({ -1, 0, 1, 0, 1, 0, -1, 0 });
//    
//    spatialPatternGenerator.setPattern (theWall.noiseNotes());
    
//    // The Wall III
//    std::vector<float> melody { -18, 0, 18, -18, 0, 18, -18, 0, 18 };
//    int transposition = 0;
//    MelodicNotes theWall =
//    MelodicNotes(melody, freq)
//        .withBandwidth (4.5)
//        .withTransposition (transposition)
//        .withPans ({ -1, 0, 1, 1, 0, -1, 0, 0, 0 });
//    
//    spatialPatternGenerator.setPattern (theWall.noiseNotes());
    
    // The Wall III
//    std::vector<float> melody { -18, -9, 0, 9, 18, -18, -9, 0, 9, 18, -18, -9, 0, 9, 18 };
//    int transposition = 0;
//    MelodicNotes theWall =
//    MelodicNotes(melody, freq)
//        .withBandwidth (4.5)
//        .withTransposition (transposition)
//        .withPans ({ -1, -0.5, 0, 0.5, 1, 1, 0.5, 0, -0.5, -1, 0, 0, 0, 0, 0 });
//    
//    spatialPatternGenerator.setPattern (theWall.noiseNotes());
    
//    // The Wall IV
//    std::vector<float> melody { -18, -9, 0, 9, 18, -18, -9, 0, 9, 18, -18, -9, 0, 9, 18 };
//    int transposition = 0;
//    MelodicNotes theWall =
//    MelodicNotes(melody, freq)
//        .withBandwidth (4.5)
//        .withTransposition (transposition)
//        .withPans ({ -1, -0.5, 0, 0.5, 1, 1, 0.5, 0, -0.5, -1, 0, 0, 0, 0, 0 });
//    
//    spatialPatternGenerator.setPattern (theWall.noiseNotes());
    
//    // The Wall IV (mini)
//    float scale = 0.5;
//    std::vector<float> melody { -18, -9, 0, 9, 18, -18, -9, 0, 9, 18, -18, -9, 0, 9, 18 };
//    for (int i = 0; i < melody.size(); ++i)
//        melody[i] *= scale;
//    int transposition = 0;
//    MelodicNotes theWall =
//    MelodicNotes(melody, freq)
//        .withBandwidth (3.5 * scale)
//        .withTransposition (transposition)
//        .withPans ({ -1, -0.5, 0, 0.5, 1, 1, 0.5, 0, -0.5, -1, 0, 0, 0, 0, 0 });
//    
//    spatialPatternGenerator.setPattern (theWall.noiseNotes());
    
//    // The Wall V (dynamic)
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float ratioInOctaves = std::log2 (freqAbove / freqBelow);
//        float scale = ratioInOctaves / 5.0;
//        std::vector<float> melody { -18, -9, 0, 9, 18, -18, -9, 0, 9, 18, -18, -9, 0, 9, 18 };
//        for (int i = 0; i < melody.size(); ++i)
//            melody[i] *= scale;
//        int transposition = 0;
//        MelodicNotes theWall =
//        MelodicNotes(melody, freq)
//            .withBandwidth (3.75 * scale)
//            .withTransposition (transposition)
//            .withPans ({ -1, -0.5, 0, 0.5, 1, 1, 0.5, 0, -0.5, -1, 0, 0, 0, 0, 0 });
//        
//        spatialPatternGenerator.setPattern (theWall.noiseNotes());
//    }
    
//    // The Wall VI (dynamic)
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float ratioInOctaves = std::log2 (freqAbove / freqBelow);
//        float scale = ratioInOctaves / 3.0;
//        std::vector<float> melody { -18, -9, 0, 9, 18, -18, -9, 0, 9, 18, -18, -9, 0, 9, 18 };
//        for (int i = 0; i < melody.size(); ++i)
//            melody[i] *= scale;
//        int transposition = 0;
//        MelodicNotes theWall =
//        MelodicNotes(melody, freq)
//            .withBandwidth (3.75 * scale)
//            .withTransposition (transposition)
//            .withPans ({ -1, -0.5, 0, 0.5, 1, 1, 0.5, 0, -0.5, -1, 0, 0, 0, 0, 0 });
//        
//        spatialPatternGenerator.setPattern (theWall.noiseNotes());
//    }
    
//    // The Wall VI (dynamic)
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
////        float ratioInOctaves = std::log2 (freqAbove / freqBelow);
////        float scale = ratioInOctaves / 3.0;
////        std::vector<float> melody { -18, -9, 0, 9, 18, -18, -9, 0, 9, 18, -18, -9, 0, 9, 18 };
////        for (int i = 0; i < melody.size(); ++i)
////            melody[i] *= scale;
////        int transposition = 0;
////        MelodicNotes theWall =
////        MelodicNotes(melody, freq)
////            .withBandwidth (3.75 * scale)
////            .withTransposition (transposition)
////            .withPans ({ -1, -0.5, 0, 0.5, 1, 1, 0.5, 0, -0.5, -1, 0, 0, 0, 0, 0 });
//        float bandwidth = 3.0f;
//        float durationInSamples = 10000;
//        float pan = 0.0f;
//        std::pair<float, float> envelope = { 1.0f, 1.0f };
//        std::vector<NoiseNote> notes {
//            NoiseNote (freqBelow, bandwidth, durationInSamples, pan, envelope, false),
//            NoiseNote (freq, bandwidth, durationInSamples, pan, envelope, false),
//            NoiseNote (freqAbove, bandwidth, durationInSamples, pan, envelope, false)
//        };
//        
//        spatialPatternGenerator.setPattern (notes);
//    }
    
//    // The Wall VII (dynamic)
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
////        float ratioInOctaves = std::log2 (freqAbove / freqBelow);
////        float scale = ratioInOctaves / 3.0;
////        std::vector<float> melody { -18, -9, 0, 9, 18, -18, -9, 0, 9, 18, -18, -9, 0, 9, 18 };
////        for (int i = 0; i < melody.size(); ++i)
////            melody[i] *= scale;
////        int transposition = 0;
////        MelodicNotes theWall =
////        MelodicNotes(melody, freq)
////            .withBandwidth (3.75 * scale)
////            .withTransposition (transposition)
////            .withPans ({ -1, -0.5, 0, 0.5, 1, 1, 0.5, 0, -0.5, -1, 0, 0, 0, 0, 0 });
//        float bandwidth = 3.0f;
//        float durationInSamples = 10000;
//        float pan = 0.0f;
//        std::pair<float, float> envelope = { 1.0f, 1.0f };
//        NoiseNote referenceNoiseNote (1000, bandwidth, durationInSamples, pan, envelope, false);
//
//        std::vector<NoiseNote> notes {
//            NoiseNote (freqBelow, bandwidth, durationInSamples, pan, envelope, false),
//            referenceNoiseNote,
//            NoiseNote (freq, bandwidth, durationInSamples, pan, envelope, false),
//            referenceNoiseNote,
//            NoiseNote (freqAbove, bandwidth, durationInSamples, pan, envelope, false),
//            referenceNoiseNote,
//        };
//        
//        spatialPatternGenerator.setPattern (notes);
//    }
    
    // The Wall VII (dynamic)
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float bandwidth = 4.5f;
//        float durationInSamples = 10000;
//        float pan = 0.0f;
//        std::pair<float, float> envelope = { 1.0f, 1.0f };
//        NoiseNote referenceNoiseNote (1000, bandwidth, durationInSamples, pan, envelope, false);
//
//        std::vector<NoiseNote> notes {
//            NoiseNote (freqBelow, bandwidth, durationInSamples, pan, envelope, false),
//            referenceNoiseNote,
//            NoiseNote (freq, bandwidth, durationInSamples, pan, envelope, false),
//            referenceNoiseNote,
//            NoiseNote (freqAbove, bandwidth, durationInSamples, pan, envelope, false),
//            referenceNoiseNote,
//        };
//        
//        spatialPatternGenerator.setPattern (notes);
//    }
//    
    
//    // The Wall X (dynamic)
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        MelodicNotes melodicNotes =
//        MelodicNotes::withFreqs ({ freqBelow, freq, freqAbove })
//            .withBandwidth (3.0f)
//            .withCyclingPans ({ -1, 0, 1, 0 });
//
//        spatialPatternGenerator.setPattern (melodicNotes.noiseNotesWithInterspersedReference (500));
//    }
    
//    // The Wall XI (dynamic)
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octaveDiff = std::log2 (freqAbove / freqBelow);
//        
//        MelodicNotes melodicNotes =
//        MelodicNotes::withFreqs ({ freqBelow, freq, freqAbove })
//            .withBandwidth (octaveDiff)
//            .withCyclingPans ({ -1, 0, 1, 0 });
//
//        spatialPatternGenerator.setPattern (melodicNotes.noiseNotesWithInterspersedReference (500));
//    }
    
//    // SineWall (dynamic)
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//    
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octaveDiff = std::log2 (freqAbove / freqBelow);
//        
//        MelodicNotes melodicNotes =
//        MelodicNotes::withFreqs ({ freq, 1000 })
//            .withBandwidth (0.1)
//            .withCyclingPans ({ -1, -1, 0, 0, 1, 1 });
//
//        spatialPatternGenerator.setPattern (melodicNotes.noiseNotes());
//    }
    
    // SineWall 2 (dynamic)
    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
    
    if (nodeBelow.has_value() && nodeAbove.has_value())
    {
        auto [freqBelow, _] = nodeBelow.value();
        auto [freqAbove, __] = nodeAbove.value();
        
        float octaveDiff = std::log2 (freqAbove / freqBelow);
        
        MelodicNotes melodicNotes =
        MelodicNotes::withFreqs ({ 1000, freq })
            .withBandwidth (0.1);
//            .withCyclingPans ({ -1, -1, 0, 0, 1, 1 });

        spatialPatternGenerator.setPattern (melodicNotes.noiseNotes());
    }
    
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
    // TODO Later
}

void PlaybackManager::updatePanCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // TODO Later
}

void PlaybackManager::startPhaseCalibration(float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // TODO Later
}

void PlaybackManager::updatePhaseCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // TODO Later
}

void PlaybackManager::startProbingFreq (float freq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
{
    // TODO Later
}

void PlaybackManager::updateProbingFreq (float freq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
{
    // TODO Later
}

void PlaybackManager::stopProbing()
{
    // TODO Later
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
    auto [leftSample5, rightSample5] = spatialPatternGenerator5.getNextSample();
//    auto [leftSample6, rightSample6] = spatialPatternGenerator6.getNextSample();
    return { leftSample1 + leftSample2 + leftSample3 + leftSample4 + leftSample5, rightSample1 + rightSample2 + rightSample3 + rightSample4 + rightSample5 };
}
