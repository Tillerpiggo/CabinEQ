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
    std::cout << "start ampl calibration!" << std::endl;
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
    
////    // Soundstage V // useful to create a blocky and nice soundstage, but must be done manually and carefully
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
    
//    // Faux Music I
//    MelodicNotes kick =
//    MelodicNotes::withFreqs ({ 20, 20, 20, 20, 20, 20, 20, 20 })
//        .withBandwidths ({ 3.0, 0.0, 0.0, 3.0, 0.0, 0.0, 3.0, 0.0 });
//    
//    MelodicNotes vocals =
//    MelodicNotes::withFreqs ({ 500, 2000, 1000, 1500 })
//        .withBandwidth (3.0)
//        .withPans ({ -1.0, 1.0, 1.0, -1.0 });
//    
//    MelodicNotes guitars =
//    MelodicNotes::withFreqs ({ 2000, 2000 })
//        .withBandwidth (4.0)
//        .withCyclingPans ({ -1.0, 1.0 });
//    
//    MelodicNotes hats =
//    MelodicNotes::withFreqs ({ 8000, 7000, 8000, 7000 })
//        .withBandwidth (0.5)
//        .withCyclingPans ({ -0.5, 0.5 });
//    
//    spatialPatternGenerator.setPattern (kick.noiseNotes());
//    spatialPatternGenerator2.setPattern (vocals.noiseNotes());
//    spatialPatternGenerator3.setPattern (guitars.noiseNotes());
//    spatialPatternGenerator4.setPattern (hats.noiseNotes());
    
//    // Spatial 2XVI
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, -6, 0, 6, 12, 18, 24, 30, 36 }, freq)
//        .withBandwidth (4.5)
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1 })
//        .withNoteDurationInSeconds (0.15);
//    
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.5)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.2);
//    
//    spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
    
    // Spatial 2XVI
//    MelodicNotes centerMelody =
//    MelodicNotes({ 0, -5, 0, 7, 0, -12, 0, 12, 0, -24, 0, 24 }, freq)
//        .withBandwidth (4.5)
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (-1).noiseNotes());
    
    // Instead of spiral, at the same time
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (7.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.2);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
    // Instead of spiral, at the same time
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (7.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.2);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
    // Melody II
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, -12, 0, 0, 12, 0, 12, 12, 24, 12, 24, 24, 36, 24, 36 }, freq)
//        .withBandwidth (7.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.2);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
    // Melody III - much more solid and bassy results, not as much runaway high end
//    MelodicNotes centerMelody =
//    MelodicNotes({ 0, -12, 0, -6, 0, 6, 12, 0, 18, 0, 24, 0, 30, 0, 36 }, freq)
//        .withBandwidth (7.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.2);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
    // Melody IV - also more solid but not quite as good
//    MelodicNotes centerMelody =
//    MelodicNotes({ 0, 0, -6, 6, -12, 12, -18, 18, -24, 24, -30, 30, -36, 36 }, freq)
//        .withBandwidth (7.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.2);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
//    // Up and down
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, 0, 12, 24, 36, 24, 12, 0, -12 }, freq)
//        .withBandwidth (7.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.2);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
//    // Up and down fine grained
//    std::vector<float> semitones;
//    for (int i = -12; i <= 36; i += 3)
//        semitones.push_back (i);
//    MelodicNotes centerMelody =
//    MelodicNotes(semitones, freq)
//        .withCyclingBandwidths ({ 7.0, 0.5, 3.0, 0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
    // Literally just an arpeggio lol
//    std::vector<float> semitones;
//    for (int i = -24; i <= 24; i += 2)
//        semitones.push_back (i);
//    
//    for (int i = 22; i > -24; i -= 2)
//        semitones.push_back (i);
//    
//    MelodicNotes centerMelody =
//    MelodicNotes(semitones, freq)
//        .withCyclingBandwidths ({ 0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
//    // Up and down II
//    MelodicNotes centerMelody =
//    MelodicNotes({ -24, -12, 0, 12, 24, 12, 0, -12 }, freq)
//        .withBandwidth (7.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
    // Up and down III
//    MelodicNotes centerMelody =
//    MelodicNotes({ -24, -12, 0, 12, 24, 12, 0, -12 }, freq)
//        .withBandwidth (6.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.1);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
//    // Melody V (VERY GOOD RESULTS! LARGE AND COMFORTABLE)
//    MelodicNotes centerMelody =
//    MelodicNotes({ -18, -9, 0, 9, 18, 9, 0, -9 }, freq)
//        .withBandwidth (7.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.15);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
//    // Melody VI - too much high end but very clear
//    MelodicNotes centerMelody =
//    MelodicNotes({ -12, -6, 0, 6, 12, 6, 0, -6 }, freq)
//        .withBandwidth (7.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.15);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
//    // Melody VII - great but doesn't always get centered in the right place
//    MelodicNotes centerMelody =
//    MelodicNotes({ -16, -8, 0, 8, 16, 8, 0, -8 }, freq)
//        .withBandwidth (7.0)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withNoteDurationInSeconds (0.15);
//    
//    spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//    spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//    spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
    
//    // Dynamic Melody I - great but still a bit easy to not center quite right
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 4.0f;
//        
//        std::vector<float> semitones { -16, -8, 0, 8, 16, 8, 0, -8 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
////    // Dynamic Melody II - almost uncomfortably large, super "fast", and very engaging. A bit sibilant, sometimes hard to tell when to add thin peaks/dips
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.0f;
//        
//        std::vector<float> semitones { -16, -8, 0, 8, 16, 8, 0, -8 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
    // Dynamic Melody III - almost uncomfortably large, super "fast", and very engaging. A bit sibilant, sometimes hard to tell when to add thin peaks/dips
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 2.0f;
//        
//        std::vector<float> semitones { -16, -8, 0, 8, 16, 8, 0, -8 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Dynamic Melody IV - really good but sibilant and fatiguing
//      auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//      auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//      if (nodeBelow.has_value() && nodeAbove.has_value())
//      {
//          auto [freqBelow, _] = nodeBelow.value();
//          auto [freqAbove, __] = nodeAbove.value();
//          
//          float octDiff = std::log2 (freqAbove / freqBelow);
//          
//          float ratio = octDiff / 4.0f;
//          
//          std::vector<float> semitones { -16, -8, 0, 8, 16, 8, 0, -8 };
//          for (int i = 0; i < semitones.size(); ++i)
//              semitones[i] *= ratio;
//          
//          MelodicNotes centerMelody =
//          MelodicNotes(semitones, freq)
//              .withBandwidth (7.0 * ratio)
//              .withNoteDurationInSeconds (0.15);
//          
//          spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//          spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//          spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//      }
    
//    // Dynamic Melody V - really good but sibilant and fatiguing
//      auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//      auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//      if (nodeBelow.has_value() && nodeAbove.has_value())
//      {
//          auto [freqBelow, _] = nodeBelow.value();
//          auto [freqAbove, __] = nodeAbove.value();
//          
//          float octDiff = std::log2 (freqAbove / freqBelow);
//          
//          float ratio = octDiff / 4.0f;
//          
//          std::vector<float> semitones { -16, -8, 0, 8, 16, 8, 0, -8 };
//          for (int i = 0; i < semitones.size(); ++i)
//              semitones[i] *= ratio;
//          
//          MelodicNotes centerMelody =
//          MelodicNotes(semitones, freq)
//              .withBandwidth (8.0 * ratio)
//              .withNoteDurationInSeconds (0.15);
//          
//          spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//          spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//          spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//      }
    
//    // Dynamic Melody VI - really good but sibilant and fatiguing
//      auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//      auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//      if (nodeBelow.has_value() && nodeAbove.has_value())
//      {
//          auto [freqBelow, _] = nodeBelow.value();
//          auto [freqAbove, __] = nodeAbove.value();
//          
//          float octDiff = std::log2 (freqAbove / freqBelow);
//          
//          float ratio = octDiff / 4.0f;
//          
//          std::vector<float> semitones { -16, -8, 0, 8, 16, 8, 0, -8 };
//          for (int i = 0; i < semitones.size(); ++i)
//              semitones[i] *= ratio;
//          
//          MelodicNotes centerMelody =
//          MelodicNotes(semitones, freq)
//              .withBandwidth (8.0 * ratio)
//              .withNoteDurationInSeconds (0.15);
//          
//          spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).withSubdivisions (3, 0).noiseNotes());
//          spatialPatternGenerator.setPattern (centerMelody.withPan (0).withSubdivisions (3, 1).noiseNotes());
//          spatialPatternGenerator3.setPattern (centerMelody.withPan (1).withSubdivisions (3, 2).noiseNotes());
//      }
    
    // Dynamic Melody VII - step in the wrong direction. Leads to less change. Bleh. Lame. Still good but I want insane
//      auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//      auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//      if (nodeBelow.has_value() && nodeAbove.has_value())
//      {
//          auto [freqBelow, _] = nodeBelow.value();
//          auto [freqAbove, __] = nodeAbove.value();
//          
//          float octDiff = std::log2 (freqAbove / freqBelow);
//          
//          float ratio = octDiff / 4.0f;
//          
//          std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//          for (int i = 0; i < semitones.size(); ++i)
//              semitones[i] *= ratio;
//          
//          MelodicNotes centerMelody =
//          MelodicNotes(semitones, freq)
//              .withBandwidth (8.0 * ratio)
//              .withNoteDurationInSeconds (0.15);
//          
//          spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//          spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//          spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//      }
    
//    // Melody VIII Cool but too variable in setting. Too loosy goosy
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 4.0f;
//        
//        std::vector<float> semitones { -14, -7, 0, 7, 14, 7, 0, -7 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody IX Cool but too variable in setting. Too loosy goosy
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 4.0f;
//        
//        std::vector<float> semitones { -17, -8.5, 0, 8.5, 17, 8.5, 0, -8.5 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody X Really really good but not quite perfect
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 4.0f;
//        
//        std::vector<float> semitones { -17.5, -8.75, 0, 8.75, 17.5, 8.75, 0, -8.75 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody XI - so much energy and clarity, but still some sibilance as a tradeoff.
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.5f;
//        
//        std::vector<float> semitones { -17.5, -8.75, 0, 8.75, 17.5, 8.75, 0, -8.75 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody XII - MASSIVE. Still a bit sibilant
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.5f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody XIII - Really really clear on laptop speakers. Nothing crazy and maybe still slightly sibilant
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.25f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody XIV - LUSCIOUS ON LAPTOP SPEAKERS
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.0f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody XV - I think I overshot a little. Still good, but not *it*
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 2.5f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody XVI - I think I overshot a little. Still good, but not *it*
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 2.75f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody XVII - Really loving this on laptop speakers as well
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 2.9f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody XVIII - Great, just fantastic, but could have an ounce more juice
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.1f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody XIX - Really loving this on laptop speakers as well
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.05f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
////    // Melody 2X - Great but too glossy. Really really good separation of instruments though...
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.05f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.3 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 2XI - WOAH WTF THIS IS GREAT
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.05f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 2XII - Eh a bit worse than 2XI
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.05f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.0 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 2XIII - Also so good, can't tell if 2XI is better or this is but my gut is this is.
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.05f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.75 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 2XIV - Can't really tell, there's pros and cons, but I think it might slightly edge out?
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.05f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.625 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 2XV - thought it was gonna be ass and then it was the best I've ever heard these laptop speakers
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.3f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.625 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 2XVI - a little worse, I think it might be too loosy goosy
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.5f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.625 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 2XVII - also luscious. Tied or maybe slightly worse (or maybe slightly better?) than 2XV
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.15f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.625 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 2XVIII - I think this is the best yet, but it's slightly glossy
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.2f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.625 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 2XIX - slightly reduced gloss, making this even better!
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.2f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 3X - a step in the wrong direction - edit: it's actually the best...
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.3f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 3XI - a step in the right direction, but maybe a step too far?
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.1f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 3XI - a step in the right direction, but maybe a step too far?
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.15f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
    // Melody 3XII - a little worse...
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.4f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 3XIV - the best, most well rounded, grounded. Could, in a perfect world, use a tad more energy
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.25f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//     Melody 3XV - this is about as good as it's gonna get for tonight
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.225f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//   // Melody 3XVI - this is about as good as it's gonna get for tonight
//   auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//   auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//   if (nodeBelow.has_value() && nodeAbove.has_value())
//   {
//       auto [freqBelow, _] = nodeBelow.value();
//       auto [freqAbove, __] = nodeAbove.value();
//       
//       float octDiff = std::log2 (freqAbove / freqBelow);
//       
//       float ratio = octDiff / 3.235f;
//       
//       std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//       for (int i = 0; i < semitones.size(); ++i)
//           semitones[i] *= ratio;
//       
//       MelodicNotes centerMelody =
//       MelodicNotes(semitones, freq)
//           .withBandwidth (7.5 * ratio)
//           .withNoteDurationInSeconds (0.15);
//       
//       spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//       spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//       spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//   }
    
//    // Melody 3XVII - this is about as good as it's gonna get for tonight
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.24f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 3XIX - this is about as good as it's gonna get for tonight
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.35f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 4X - almost as good as 3XVI, but a bit glossy
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.32f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 4XI - Still very good, but not as good as 3XIV
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.31f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 4XII - The DEPTH
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.29f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 4XIII - Also just amazing
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.28f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 4XIV - worse than 4XIII... nvm I actually prefer it
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.27f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 4XV - also a little worse. a little less bass
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.285f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 4XVI - also a little worse. a little less bass
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.275f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 4XVII - best so far
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.26f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 4XVIII - has just a little less flavor than 4XVII
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.255f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 4XIX - also a little less flavor
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.265f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 5X - a little more high end, a little less oomph
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.2625f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 5XI - same but getting closer
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.2615f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Melody 5XII - the separation is just ridiculous
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
////        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
////        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Transposition I
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        float transposition = octDiff * -1.8;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//        {
//            semitones[i] += transposition;
//            semitones[i] *= ratio;
//        }
//        
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Transposition II
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        float transposition = -3;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//        {
//            semitones[i] += transposition;
//            semitones[i] *= ratio;
//        }
//        
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Pink Noise I - Fantastic but blocky and a bit harsh
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.05f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
////    // Pink Noise II - Really big, just great
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.3f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Pink Noise III - Fantastic but blocky and a bit harsh
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.2f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
    // Pink Noise IV - Fantastic but blocky and a bit harsh
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (8.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
//        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Justin I - melody on noise
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (2.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        MelodicNotes backgroundNoise =
//        MelodicNotes::withFreqs ({ 1000 })
//            .withBandwidth (10000)
//            .withNoteDurationInSeconds (0.5);
//        
////        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//        spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
////        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Justin II - melody on noise
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        
//        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (2 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
//        MelodicNotes backgroundNoise =
//        MelodicNotes ({ 0 }, freq)
//            .withBandwidth (7.0)
//            .withNoteDurationInSeconds (0.6);
//        
////        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.noiseNotes());
//        spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
////        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // Patterns III
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = 10;
//        
//        MelodicNotes hihats =
//        MelodicNotes({ 0 }, freq)
//            .withBandwidth (ratio)
//            .withNoteDurationInSeconds (0.3);
//        
//        MelodicNotes claps =
//        MelodicNotes({ -24 }, freq)
//            .withBandwidth (ratio * 8)
//            .withNoteDurationInSeconds (0.6);
//        
//        
//        spatialPatternGenerator.setPattern (hihats.withSubdivisions (2, 0).noiseNotes());
//        spatialPatternGenerator2.setPattern (claps.withSubdivisions (4, 0).noiseNotes());
//    }
    
//    // Patterns IV
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        float ratio = octDiff / 2.0f;
//        
//        float bandwidth = 1.0 * ratio;
//        float freqRatio = 2.0 * ratio;
//        
//        MelodicNotes hihats = MelodicNotes::withPattern ({ 1, 0, 1, 0 }, freq * freqRatio, bandwidth * 2);
//        MelodicNotes claps = MelodicNotes::withPattern ({ 1, 0, 0, 1, 0, 0, 1, 0 }, freq, bandwidth);
//        MelodicNotes kicks = MelodicNotes::withPattern ({ 1, 1, 0, 0 }, freq / freqRatio, bandwidth * 2);
//        
//        spatialPatternGenerator.setPattern (hihats.noiseNotes());
//        spatialPatternGenerator2.setPattern (claps.withRepeatedTranspositions ({ -1, 0, 1 }).noiseNotes());
//        spatialPatternGenerator3.setPattern (kicks.noiseNotes());
//    }
    
////    // Patterns V
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        float ratio = octDiff / 2.0f;
//        
//        float bandwidth = 1.0 * ratio;
//        float freqRatio = 2.0 * ratio;
//        
//        MelodicNotes hihats = 
//        MelodicNotes::withPattern ({ 1, 0, 1, 0 }, freq * freqRatio, bandwidth * 2)
//            .withNoteDurationInSeconds (0.2);
//        
//        MelodicNotes claps = MelodicNotes::withPattern ({ 1, 0, 0, 1, 0, 0, 1, 0 }, freq, bandwidth * 2)
//            .withNoteDurationInSeconds (0.1);
//        
//        MelodicNotes kicks = MelodicNotes::withPattern ({ 1, 1, 0, 0 }, freq / freqRatio, bandwidth * 2)
//            .withNoteDurationInSeconds (0.2);
//        
//        spatialPatternGenerator.setPattern (hihats.noiseNotes());
//        spatialPatternGenerator2.setPattern (claps.noiseNotes());
//        spatialPatternGenerator3.setPattern (kicks.noiseNotes());
//    }
    
//    // Patterns VI
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        float ratio = octDiff / 2.0f;
//        
//        float bandwidth = 1.0 * ratio;
//        float freqRatio = 2.0 * ratio;
//        
//        MelodicNotes hihats =
//        MelodicNotes::withPattern ({ 1, 0, 1, 0 }, freq * freqRatio, bandwidth * 2)
//            .withNoteDurationInSeconds (0.2);
//        
//        MelodicNotes claps = MelodicNotes::withPattern ({ 1, 0, 0, 1, 0, 0, 1, 0 }, freq, bandwidth * 2)
//            .withNoteDurationInSeconds (0.1);
//        
//        MelodicNotes kicks = MelodicNotes::withPattern ({ 1, 1, 0, 0 }, freq / freqRatio, bandwidth * 2)
//            .withNoteDurationInSeconds (0.2);
//        
//        spatialPatternGenerator.setPattern (hihats.noiseNotes());
//        spatialPatternGenerator2.setPattern (claps.noiseNotes());
//        spatialPatternGenerator3.setPattern (kicks.noiseNotes());
//    }
    
//    // EasyMel I
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        
//        std::vector<float> semitones { -3, 4, 0, -3, 2, -3, 0, -3 };
////        std::vector<float> semitones { -18, -9, 0, 9, 18, 9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio * 5;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
////        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
////        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // EasyMel II - great results on speakers
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        
////        std::vector<float> semitones { -3, 4, 0, -3, 2, -3, 0, -3 };
//        std::vector<float> semitones { -18, 9, 0, 18, 9, -9, 0, -9 };
//        for (int i = 0; i < semitones.size(); ++i)
//            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
////        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
////        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // EasyMel III - the best so far
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        
////        std::vector<float> semitones { -3, 4, 0, -3, 2, -3, 0, -3 };
//        std::vector<float> semitones { -5, 7, 4, 0, -6, 6, 2, -1 };
////        for (int i = 0; i < semitones.size(); ++i)
////            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (7.5 * ratio)
//            .withNoteDurationInSeconds (0.15);
//        
////        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
////        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
//    // EasyMel IV - bandwidth too small, hard to even hear, and the sine tones don't line up well
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        
////        std::vector<float> semitones { -3, 4, 0, -3, 2, -3, 0, -3 };
//        std::vector<float> semitones { -5, 7, 4, 0, -6, 6, 2, -1 };
////        for (int i = 0; i < semitones.size(); ++i)
////            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (0.3)
//            .withNoteDurationInSeconds (0.15);
//        
////        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
////        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
    // EasyMel V - bandwidth too small, hard to even hear, and the sine tones don't line up well
//    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
//    auto nodeAbove = amplCurve.nodeAboveFreq (freq);
//
//    if (nodeBelow.has_value() && nodeAbove.has_value())
//    {
//        auto [freqBelow, _] = nodeBelow.value();
//        auto [freqAbove, __] = nodeAbove.value();
//        
//        float octDiff = std::log2 (freqAbove / freqBelow);
//        
//        float ratio = octDiff / 3.262f;
//        
////        std::vector<float> semitones { -3, 4, 0, -3, 2, -3, 0, -3 };
//        std::vector<float> semitones { 0, 0, 7, 7, 9, 9, 7, 7, 5, 5, 4, 4, 2, 2, 0, 0, 5, 5, 4, 4, 3, 3, 2, 2, 5, 5, 4, 4, 3, 3, 2, 2, 0, 0, 7, 7, 9, 9, 7, 7, 5, 5, 4, 4, 2, 2, 0, 0 };
////        for (int i = 0; i < semitones.size(); ++i)
////            semitones[i] *= ratio;
//        
//        MelodicNotes centerMelody =
//        MelodicNotes(semitones, freq)
//            .withBandwidth (1.0)
//            .withTransposition (-5)
//            .withNoteDurationInSeconds (0.3);
//        
////        spatialPatternGenerator2.setPattern (centerMelody.withPan (-1).noiseNotes());
//        spatialPatternGenerator.setPattern (centerMelody.withPan (0).noiseNotes());
////        spatialPatternGenerator3.setPattern (centerMelody.withPan (1).noiseNotes());
//    }
    
    // JustinPatterns I
    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
    auto nodeAbove = amplCurve.nodeAboveFreq (freq);

    if (nodeBelow.has_value() && nodeAbove.has_value())
    {
        auto [freqBelow, _] = nodeBelow.value();
        auto [freqAbove, __] = nodeAbove.value();
         
        float octDiff = std::log2 (freqAbove / freqBelow);
        float ratio = octDiff / 2.0f;
        
        float bandwidth = 1.0 * ratio;
        float freqRatio = 2.0 * ratio;
         
        MelodicNotes drums =
        MelodicNotes::withFreqs ({ 200, 5000, 1000, 5000, 1000 })
            .withBandwidth (7.0)
            .withNoteDurationInSeconds (0.2);
         
        MelodicNotes details = MelodicNotes::withPattern ({ 1, 0, 0, 1, 0, 0, 1, 0 }, freq, bandwidth * 4)
            .withCyclingPans ({ -1, -0.75, -0.5, -0.25, 0.0, 0.25, 0.5, 0.75, 1.0, 0.75, 0.5, 0.25, 0.0, -0.25, -0.5, -0.75, -1 })
            .withNoteDurationInSeconds (0.1);
         
//        MelodicNotes kicks = MelodicNotes::withPattern ({ 1, 1, 0, 0 }, freq / freqRatio, bandwidth * 2)
//            .withNoteDurationInSeconds (0.2);
         
        spatialPatternGenerator.setPattern (drums.noiseNotes());
        spatialPatternGenerator2.setPattern (details.noiseNotes());
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
