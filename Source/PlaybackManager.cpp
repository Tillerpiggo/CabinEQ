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
    
    if (isCalibrating)
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

void PlaybackManager::startAmplCalibration (float freq, float bandwidth)
{
    /*
    // JustinExperiments D
    auto nodeBelow = amplCurve.nodeBelowFreq (freq);
    auto nodeAbove = amplCurve.nodeAboveFreq (freq);

    if (nodeBelow.has_value() && nodeAbove.has_value())
    {
        auto [freqBelow, _] = nodeBelow.value();
        auto [freqAbove, __] = nodeAbove.value();
         
        float octDiff = std::log2 (freqAbove / freqBelow);
        float ratio = octDiff / 2.0f;
        
        float bandwidth = 1.5 * ratio;
        float semitonesAbove = ratio * 18.0f;
         
        MelodicNotes drums =
        MelodicNotes::withFreqs ({ 100, 400, 1600, 6400, 10000, 8000, 2400, 700, 200 })
            .withBandwidth (8.0)
            .withNoteDurationInSeconds (0.1);
         
        MelodicNotes details =
        MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -24, -18, -12, -6, 0, 6, 12, 24, 18, 12, 6, 0, -9, -15 }, freq, 3.0, { 0 })
            .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
            .withNoteDurationInSeconds (0.1);
        
        
        MelodicNotes sideDrums =
        MelodicNotes::withFreqs ({ 100, 100, 400, 400, 1600, 1600, 6400, 6400, 10000, 10000, 8000, 8000, 2400, 2400, 700, 700, 200, 200 })
            .withCyclingPans ({ -1, -0.5, 0.5, 1 })
            .withBandwidth (8.0)
            .withNoteDurationInSeconds (0.05);
        
        for (const auto& noiseNote : details.noiseNotes())
            std::cout << "NoiseNote(" << noiseNote.bandwidth << ", " << noiseNote.freqFactor << ", " << noiseNote.pan <<  std::endl;
         
        spatialPatternGenerator.setPattern (drums.noiseNotes());
        spatialPatternGenerator2.setPattern (details.noiseNotes());
        spatialPatternGenerator3.setPattern (sideDrums.noiseNotes());
     }
     */
}

void PlaybackManager::updateAmplCalibration (float freq, float bandwidth)
{
//    spatialPatternGenerator.setAmplCurve (amplCurve);
//    spatialPatternGenerator2.setAmplCurve (amplCurve);
//    spatialPatternGenerator3.setAmplCurve (amplCurve);
//    spatialPatternGenerator4.setAmplCurve (amplCurve);
//    spatialPatternGenerator5.setAmplCurve (amplCurve);
//    spatialPatternGenerator6.setAmplCurve (amplCurve);
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
        leftSample += leftSample2 * 2.0;
        rightSample += rightSample2 * 2.0;
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
