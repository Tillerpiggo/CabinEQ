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
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
    // Pink Noise 2XIII
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
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
//    // Soundstage II // cool but thin and sparse (but nicely blocky!)
//    MelodicNotes width =
//    MelodicNotes ({ -3, 0, 3, 0 }, freq).withCyclingPans ({ -1, 1 });
//    
//    MelodicNotes halfWidth =
//    MelodicNotes ({ -2, 0, 2, 0 }, freq).withCyclingPans ({ 0.5, -0.5 });
//    
//    MelodicNotes center =
//    MelodicNotes ({ 0, 0 }, freq);
//    
//    spatialPatternGenerator.setPattern (width.noiseNotes());
//    spatialPatternGenerator2.setPattern (halfWidth.noiseNotes());
//    spatialPatternGenerator3.setPattern (center.noiseNotes());
    
//    // Soundstage III // useful to create a blocky and nice soundstage, but must be done manually and carefully
//    MelodicNotes width =
//    MelodicNotes ({ -8, 0, 8, 0 }, freq).withCyclingPans ({ -1, 1 });
//    
//    MelodicNotes halfWidth =
//    MelodicNotes ({ -2, 0, 2, 0 }, freq).withCyclingPans ({ 0.5, -0.5 });
//    
//    MelodicNotes center =
//    MelodicNotes ({ 0, 0 }, freq);
//    
//    spatialPatternGenerator.setPattern (width.noiseNotes());
//    spatialPatternGenerator2.setPattern (halfWidth.noiseNotes());
//    spatialPatternGenerator3.setPattern (center.noiseNotes());
    
//    // Soundstage IV // useful to create a blocky and nice soundstage, but must be done manually and carefully
//    MelodicNotes width =
//    MelodicNotes ({ -8, 0, 8, 0, -2, 0, 2, 0 }, freq).withCyclingPans ({ -1, 1, -1, 1, -0.5, 0.5, -0.5, 0.5 });
//    
//    MelodicNotes halfWidth =
//    MelodicNotes ({ 12, -12 }, freq).withCyclingPans ({ -1, -0.5, 0, 0.5, 1 }).withBandwidth (2.0);
//    
//    MelodicNotes width2 =
//    MelodicNotes ({ -8, 0, 8, 0, -2, 0, 2, 0 }, freq).withCyclingPans ({ -1, 1, -1, 1, -0.5, 0.5, -0.5, 0.5 }).withBandwidth (4.0f);
//    
//    MelodicNotes bandwidths =
//    MelodicNotes ({ 0, 0, 0, 0, 0, 0, 0, 0 }, freq)
//        .withBandwidths ({ 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0 })
//        .withCyclingPans ({ -1.0, 1.0 });
//    
//    spatialPatternGenerator.setPattern (width.noiseNotes());
//    spatialPatternGenerator2.setPattern (halfWidth.noiseNotes());
//    spatialPatternGenerator3.setPattern (width2.noiseNotes());
//    spatialPatternGenerator4.setPattern (bandwidths.noiseNotes());
    
//    // Soundstage V // useful to create a blocky and nice soundstage, but must be done manually and carefully
//    float semitoneFactor = 1.5;
//    float bandwidthFactor = 1.0;
//    std::vector<float> bandwidths;
//    for (int i = 0; i < 4; ++i)
//    {
//        bandwidths.push_back ((i + 1) * bandwidthFactor);
//        bandwidths.push_back ((i + 1) * bandwidthFactor);
//    }
//    MelodicNotes upperLeft =
//    MelodicNotes (std::vector<float> (8, 12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (-1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes upperRight =
//    MelodicNotes (std::vector<float> (8, 12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes lowerLeft =
//    MelodicNotes (std::vector<float> (8, -12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (-1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes lowerRight =
//    MelodicNotes (std::vector<float> (8, -12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    spatialPatternGenerator.setPattern (upperLeft.noiseNotes());
//    spatialPatternGenerator2.setPattern (upperRight.noiseNotes());
//    spatialPatternGenerator3.setPattern (lowerLeft.noiseNotes());
//    spatialPatternGenerator4.setPattern (lowerRight.noiseNotes());
    
    // Soundstage VI // useful to create a blocky and nice soundstage, but must be done manually and carefully
//    MelodicNotes centered =
//    MelodicNotes ({ 0, 0, 0, 0, 0, 0, 0, 0 }, freq)
//        .withBandwidths ({ 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (centered.noiseNotes());
    
    // Soundstage VII
//    MelodicNotes centered =
//    MelodicNotes ({ 0, 0, 0, 0, 0, 0, 0, 0 }, freq)
//        .withBandwidths ({ 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes left =
//    MelodicNotes ({ 0, 0, 0, 0, 0, 0, 0, 0 }, freq)
//        .withBandwidths ({ 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0 })
//        .withPan (-1)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes right =
//    MelodicNotes ({ 0, 0, 0, 0, 0, 0, 0, 0 }, freq)
//        .withBandwidths ({ 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0 })
//        .withPan (1)
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (centered.noiseNotes());
//    spatialPatternGenerator2.setPattern (left.noiseNotes());
//    spatialPatternGenerator3.setPattern (right.noiseNotes());
    
//    // Soundstage VIII
//    MelodicNotes centered =
//    MelodicNotes ({ -12, 0, 12 }, freq)
//        .withBandwidth (3.0)
//        .withNoteDurationInSeconds (0.2);
//    
//    MelodicNotes centered2 =
//    MelodicNotes ({ -12, 0, 12 }, freq)
//        .withBandwidth (1.0)
//        .withNoteDurationInSeconds (0.2);
//    
//    MelodicNotes centered3 =
//    MelodicNotes ({ -12, 0, 12 }, freq)
//        .withBandwidth (5.0)
//        .withNoteDurationInSeconds (0.2);
//    
//    MelodicNotes left =
//    MelodicNotes ({ 0, 0, 0, 0, 0, 0, 0, 0 }, freq)
//        .withBandwidths ({ 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0 })
//        .withPan (-1)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes right =
//    MelodicNotes ({ 0, 0, 0, 0, 0, 0, 0, 0 }, freq)
//        .withBandwidths ({ 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0 })
//        .withPan (1)
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (centered.noiseNotes());
//    spatialPatternGenerator2.setPattern (centered2.noiseNotes());
//    spatialPatternGenerator3.setPattern (centered3.noiseNotes());
//    spatialPatternGenerator2.setPattern (left.noiseNotes());
//    spatialPatternGenerator3.setPattern (right.noiseNotes());
    
    // the wall vi
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
//        float freqBelowBetween = std::sqrt (freqBelow * freq);
//        float freqAboveBetween = std::sqrt (freqAbove * freq);
//        MelodicNotes wallPattern =
//        MelodicNotes::withFreqs({ freqBelow, freqBelowBetween, freqAboveBetween, freqAbove });
//        
//        spatialPatternGenerator.setPattern (wallPattern.noiseNotes());
//    }
    
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
//        float freqBelowBetween = std::sqrt (freqBelow * freq);
//        float freqAboveBetween = std::sqrt (freqAbove * freq);
//        MelodicNotes wallPattern =
//        MelodicNotes::withFreqs({ freqBelow, freqBelowBetween, freqAboveBetween, freqAbove });
//        
//        spatialPatternGenerator.setPattern (wallPattern.noiseNotes());
//    }
    
//     Soundstage IX (goal: find what's wrong with the current headphones and make it audible
//    MelodicNotes centered =
//    MelodicNotes ({ -12, -12, -6, -6, 0, 0, 6, 6, 12, 12}, freq)
//        .withBandwidth (2.5)
//        .withCyclingPans ({ -0.5, 0.5 })
//        .withNoteDurationInSeconds (0.2);
//    
//    MelodicNotes centered2 =
//    MelodicNotes ({ -12, -6, 0, 6, 12 }, freq)
//        .withBandwidth (2.5)
//        .withPan (-1)
//        .withNoteDurationInSeconds (0.2);
//    
//    MelodicNotes centered3 =
//    MelodicNotes ({ -12, -6, 0, 6, 12 }, freq)
//        .withBandwidth (2.5)
//        .withPan (1)
//        .withNoteDurationInSeconds (0.2);
//    
////    MelodicNotes left =
////    MelodicNotes ({ 0, 0, 0, 0, 0, 0, 0, 0 }, freq)
////        .withBandwidths ({ 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0 })
////        .withPan (-1)
////        .withNoteDurationInSeconds (0.1);
////
////    MelodicNotes right =
////    MelodicNotes ({ 0, 0, 0, 0, 0, 0, 0, 0 }, freq)
////        .withBandwidths ({ 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0 })
////        .withPan (1)
////        .withNoteDurationInSeconds (0.1);
////
//    spatialPatternGenerator.setPattern (centered.noiseNotes());
//    spatialPatternGenerator2.setPattern (centered2.noiseNotes());
//    spatialPatternGenerator3.setPattern (centered3.noiseNotes());
    
   // Soundstage X (goal: find what's wrong with the current headphones and make it audible
//   MelodicNotes centered =
//   MelodicNotes ({ -12, -12, -12, -12, -12, 0, 0, 0, 0, 0, 12, 12, 12, 12, 12  }, freq)
//       .withBandwidth (2.5)
//       .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 })
//       .withNoteDurationInSeconds (0.2);
//   
//   MelodicNotes centered2 =
//   MelodicNotes ({ -12, -12, -12, -12, -12 }, freq)
//       .withBandwidth (2.5)
//       .withCyclingPans ({ 1, 0.5, 0, -0.5, -1 })
//       .withNoteDurationInSeconds (0.2);
//   
//   MelodicNotes centered3 =
//   MelodicNotes ({ 12, 12, 12, 12, 12 }, freq)
//       .withBandwidth (2.5)
//       .withCyclingPans ({ 1, 0.5, 0, -0.5, -1 })
//       .withNoteDurationInSeconds (0.2);
//   
//   spatialPatternGenerator.setPattern (centered.noiseNotes());
//   spatialPatternGenerator2.setPattern (centered2.noiseNotes());
//   spatialPatternGenerator3.setPattern (centered3.noiseNotes());
    
    // Soundstage X
//    MelodicNotes samples =
//    MelodicNotes::withFreqs ({ 30, 1000, 10000 });
    
//    // Sweeps
//    MelodicNotes upwardsSweep =
//    MelodicNotes ({ -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
//        5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6 }, freq)
//        .withBandwidth (2.0)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes upwardsSweepThin =
//    MelodicNotes ({ -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
//        5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6 }, freq)
//        .withBandwidth (0.5)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes upwardsSweepWide =
//    MelodicNotes ({ -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
//        5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6 }, freq)
//        .withBandwidth (4.0)
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (upwardsSweep.noiseNotes());
//    spatialPatternGenerator2.setPattern (upwardsSweepThin.noiseNotes());
//    spatialPatternGenerator3.setPattern (upwardsSweepWide.noiseNotes());
    
    // Sweeps
//    MelodicNotes upwardsSweep =
//    MelodicNotes ({ -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
//        5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6 }, freq)
//        .withBandwidth (2.0)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes upwardsSweepThin =
//    MelodicNotes ({ -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
//        5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6 }, freq)
//        .withBandwidth (0.5)
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes upwardsSweepWide =
//    MelodicNotes ({ -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
//        5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6 }, freq)
//        .withBandwidth (4.0)
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (upwardsSweep.noiseNotes());
//    spatialPatternGenerator2.setPattern (upwardsSweepThin.noiseNotes());
//    spatialPatternGenerator3.setPattern (upwardsSweepWide.noiseNotes());
    
//    // Soundstage VIII
//     MelodicNotes centered =
//     MelodicNotes ({ -12, 0, 12, -8, 4, 16, -4, 8, 20 }, freq)
//         .withBandwidth (3.0)
//        .withTransposition(-4)
//         .withNoteDurationInSeconds (0.2);
//     
//     MelodicNotes centered2 =
//     MelodicNotes ({ -12, 0, 12 }, freq)
//         .withBandwidth (1.0)
//         .withNoteDurationInSeconds (0.2);
//     
//     MelodicNotes centered3 =
//     MelodicNotes ({ -12, 0, 12 }, freq)
//         .withBandwidth (5.0)
//         .withNoteDurationInSeconds (0.2);
//     
//     spatialPatternGenerator.setPattern (centered.noiseNotes());
//     spatialPatternGenerator2.setPattern (centered2.noiseNotes());
//     spatialPatternGenerator3.setPattern (centered3.noiseNotes());
    
//    // Soundstage Y
//    MelodicNotes centered =
//    MelodicNotes::withFreqs ({ freq, freq, freq, freq, freq, 1000, 1000, 1000, 1000, 1000 })
//        .withBandwidth (1.5)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 })
//        .withNoteDurationInSeconds (0.2);
//     
//    spatialPatternGenerator.setPattern (centered.noiseNotes());
    
//    // Soundstage Y
//    MelodicNotes centered =
//    MelodicNotes::withFreqs ({ freq, freq, freq, freq, freq, 1000, 1000, 1000, 1000, 1000 })
//        .withBandwidth (1.5)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 })
//        .withNoteDurationInSeconds (0.2);
//     
//    spatialPatternGenerator.setPattern (centered.noiseNotes());
    
    // Soundstage Z
//    float semitoneFactor = 1.5;
//    float bandwidthFactor = 1.0;
//    std::vector<float> bandwidths;
//    for (int i = 0; i < 4; ++i)
//    {
//        bandwidths.push_back ((i + 1) * bandwidthFactor);
//        bandwidths.push_back ((i + 1) * bandwidthFactor);
//    }
//    MelodicNotes left =
//    MelodicNotes (std::vector<float> (8, 0 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (-1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes right =
//    MelodicNotes (std::vector<float> (8, 0 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes center =
//    MelodicNotes (std::vector<float> (8, 0 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (0.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    spatialPatternGenerator.setPattern (left.noiseNotes());
//    spatialPatternGenerator2.setPattern (right.noiseNotes());
//    spatialPatternGenerator3.setPattern (center.noiseNotes());
    
//    // Parallelism I
//    MelodicNotes diagonal =
//    MelodicNotes ({ -12, -9, -6, -3, 0, 3, 6, 9, 12 }, freq)
//        .withBandwidth (2.0)
//        .withPans ({ -1.0, -0.66, -0.33, 0.0, 0.33, 0.66, 1.0 })
//        .withNoteDurationInSeconds (0.1)
//        .withRepeatedTranspositions ({ -4, 0, 4 });
//    
//    spatialPatternGenerator.setPattern (diagonal.noiseNotes());
    
//    // Diamond Separation I
//    MelodicNotes sides =
//    MelodicNotes ({ 0, 0 }, freq)
//        .withBandwidth (1.0)
//        .withPans ({ -1.0, 1.0 });
//    
//    MelodicNotes diamond =
//    MelodicNotes ({ -6, 6, -6, 6 }, freq)
//        .withBandwidth (1.0)
//        .withPans ({ -1.0, 1.0, 1.0, -1.0 });
//    
//    MelodicNotes topBottom =
//    MelodicNotes ({ -15, 15 }, freq)
//        .withBandwidth (1.0);
//    
//    spatialPatternGenerator.setPattern (sides.noiseNotes());
//    spatialPatternGenerator2.setPattern (diamond.noiseNotes());
//    spatialPatternGenerator3.setPattern (topBottom.noiseNotes());
    
    // Faux Music I
    MelodicNotes kick =
    MelodicNotes::withFreqs ({ 20, 20, 20, 20, 20, 20, 20, 20 })
        .withBandwidths ({ 3.0, 0.0, 0.0, 3.0, 0.0, 0.0, 3.0, 0.0 });
    
    MelodicNotes vocals =
    MelodicNotes::withFreqs ({ 500, 2000, 1000, 1500 })
        .withBandwidth (3.0)
        .withPans ({ -1.0, 1.0, 1.0, -1.0 });
    
    MelodicNotes guitars =
    MelodicNotes::withFreqs ({ 2000, 2000 })
        .withBandwidth (4.0)
        .withCyclingPans ({ -1.0, 1.0 });
    
    MelodicNotes hats =
    MelodicNotes::withFreqs ({ 8000, 7000, 8000, 7000 })
        .withBandwidth (0.5)
        .withCyclingPans ({ -0.5, 0.5 });
    
    spatialPatternGenerator.setPattern (kick.noiseNotes());
    spatialPatternGenerator2.setPattern (vocals.noiseNotes());
    spatialPatternGenerator3.setPattern (guitars.noiseNotes());
    spatialPatternGenerator4.setPattern (hats.noiseNotes());
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
    
    
//    // Pans I
//    MelodicNotes sweep =
//    MelodicNotes ({ 0 }, freq)
//        .withBandwidth (2.0)
//        .withCyclingPans ({ -1.0, -0.66, -0.33, 0.0, 0.33, 0.66, 1.0, 0.66, 0.33, 0.0, -0.33, -0.66, -1.0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (sweep.noiseNotes());
    
//    // Pans II
//    MelodicNotes sweep =
//    MelodicNotes ({ 0, 0, 0, 0, 0, 0 }, freq)
//        .withBandwidths ({ 5.0, 5.0, 3.0, 3.0, 1.0, 1.0 })
//        .withCyclingPans ({ -1.0, 1.0 });
//    
//    spatialPatternGenerator.setPattern (sweep.noiseNotes());
    
    // Pans III
//    MelodicNotes rising =
//    MelodicNotes ({ -6, 0, 6 }, freq)
//        .withBandwidth (2.0);
//    spatialPatternGenerator.setPattern (rising.noiseNotes());
    
    // Pans IV
//    float semitoneFactor = 1.5;
//    float bandwidthFactor = 1.0;
//    std::vector<float> bandwidths;
//    for (int i = 0; i < 4; ++i)
//    {
//        bandwidths.push_back ((i + 1) * bandwidthFactor);
//        bandwidths.push_back ((i + 1) * bandwidthFactor);
//    }
//    MelodicNotes upperLeft =
//    MelodicNotes (std::vector<float> (8, 12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (-1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes upperRight =
//    MelodicNotes (std::vector<float> (8, 12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes lowerLeft =
//    MelodicNotes (std::vector<float> (8, -12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (-1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes lowerRight =
//    MelodicNotes (std::vector<float> (8, -12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    spatialPatternGenerator.setPattern (upperLeft.noiseNotes());
//    spatialPatternGenerator2.setPattern (upperRight.noiseNotes());
//    spatialPatternGenerator3.setPattern (lowerLeft.noiseNotes());
//    spatialPatternGenerator4.setPattern (lowerRight.noiseNotes());
    
    // Pans V
//    float semitoneFactor = 1.5;
//    float bandwidthFactor = 1.0;
//    std::vector<float> bandwidths;
//    for (int i = 0; i < 4; ++i)
//    {
//        bandwidths.push_back ((i + 1) * bandwidthFactor);
//        bandwidths.push_back ((i + 1) * bandwidthFactor);
//    }
//    MelodicNotes upperLeft =
//    MelodicNotes (std::vector<float> (8, 12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (-1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes upperRight =
//    MelodicNotes (std::vector<float> (8, 12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes lowerLeft =
//    MelodicNotes (std::vector<float> (8, -12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (-1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    MelodicNotes lowerRight =
//    MelodicNotes (std::vector<float> (8, -12 * semitoneFactor), freq)
//        .withBandwidths (bandwidths)
//        .withPan (1.0)
//        .withNoteDurationInSeconds (0.15);
//    
//    spatialPatternGenerator.setPattern (upperLeft.noiseNotes());
//    spatialPatternGenerator2.setPattern (upperRight.noiseNotes());
//    spatialPatternGenerator3.setPattern (lowerLeft.noiseNotes());
//    spatialPatternGenerator4.setPattern (lowerRight.noiseNotes());
    
    // Pink Noise 2XIII
    float bandwidth = 3.0;
    float freqFactor = 2.0;
    int durationInSamples = 10000;
    std::vector<NoiseNote> noisePattern {
        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 3), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
    };
    std::vector<NoiseNote> panPattern {
        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
    };
    
    std::vector<float> factors { -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
    std::vector<float> pans { -1.0, -0.5, 0.0, 0.5, 1.0 };
    std::vector<NoiseNote> panPattern2;
    
    for (const auto& pan : pans)
    {
        for (const auto& factor : factors)
        {
            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 0.1, durationInSamples / 4.0, pan, { 1.0, 1.0 }));
        }
    }
    
    std::vector<NoiseNote> panPattern3 {
        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth * 3.0, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth * 3.0, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, 0.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth * 3.0, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, 0.0, { 1.0, 1.0 }),
    };
    
    spatialPatternGenerator.setPattern (noisePattern);
    spatialPatternGenerator2.setPattern (panPattern);
    spatialPatternGenerator3.setPattern (panPattern2);
    spatialPatternGenerator4.setPattern (panPattern3);
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
