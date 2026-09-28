/*
  ==============================================================================

    PlaybackManager.h
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandProfile.h"
#include "FilterChain.h"
#include "CrossfeedProcessor.h"
#include "SpectrumAnalyzer.h"
#include "CalibrationPlayer.h"
#include "CurveFilter.h"

/// The audio path: calibration sounds (when they're playing), then the EQ (bands, or a curve's FIR filter),
/// then crossfeed, then gain,
/// with a click-free bypass. Last comes the master volume, which applies whether the EQ is on or off,
/// and a limiter that catches peaks a boost would push past full scale.
/// setBands() is for the message thread; the other setters are safe from any thread.
class PlaybackManager
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void processBlock (juce::AudioBuffer<float>& buffer) noexcept;

    void setBands (const std::vector<Band>& bands);
    /// Nothing when the profile uses bands; with `right`, the ears are split
    void setCurve (std::optional<std::vector<CurvePoint>> points, std::optional<CurveFilter::EarTweaks> tweaks = std::nullopt);
    void setGainDb (float gainDb); // preamp plus auto gain
    void setBypassed (bool shouldBeBypassed);
    void setVolumeDb (float volumeDb); // master volume, which can boost

    CrossfeedProcessor& getCrossfeed() { return crossfeed; }
    SpectrumAnalyzer& getAnalyzer() { return analyzer; }
    CalibrationPlayer& getCalibration() { return calibration; }

private:
    void processChunk (juce::AudioBuffer<float>& buffer, int start, int length) noexcept;
    void applyVolume (juce::AudioBuffer<float>& buffer, int start, int length) noexcept;

    FilterChain filter;
    CurveFilter curveFilter;
    CrossfeedProcessor crossfeed;
    CalibrationPlayer calibration;
    SpectrumAnalyzer analyzer;

    std::atomic<float> gainDb { 0.0f };
    std::atomic<bool> bypassed { false };
    std::atomic<float> volumeDb { 0.0f };
    juce::SmoothedValue<float> volume { 1.0f };
    float limiterGain = 1.0f, limiterRelease = 0.9995f;
    static constexpr float limiterCeiling = 0.97f; // about -0.3 dBFS

    juce::SmoothedValue<float> gain { 1.0f };
    juce::SmoothedValue<float> wetMix { 1.0f }; // 1 = EQ on, 0 = bypassed
    bool isFullyBypassed = false;

    juce::AudioBuffer<float> dryBuffer; // the input, for crossfading in and out of bypass
    std::vector<float> gainRamp, mixRamp;
    int maxChunkSize = 0;
};
