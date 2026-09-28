/*
  ==============================================================================

    CrossfeedProcessor.h
    Created: 29 Oct 2024 10:00:00am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/// Headphone crossfeed: mixes a delayed, low-passed copy of each channel into the other,
/// the way each ear hears both speakers in a room. The setters can be called from any thread.
class CrossfeedProcessor
{
public:
    static constexpr float maxDelayMs = 2.0f;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void process (juce::dsp::AudioBlock<float>& block) noexcept;

    void setEnabled (bool enabled);
    void setLevelDb (float levelDb); // how loud the crossfed signal is, relative to the direct one
    void setDelayMs (float delayMs);

private:
    float delayInSamples() const;

    std::atomic<bool> isEnabled { false };
    std::atomic<float> levelDb { -9.0f };
    std::atomic<float> delayMs { 0.3f };

    juce::AudioBuffer<float> delayBuffer; // one channel per side
    int writePosition = 0;
    double sampleRate = 44100.0;

    juce::SmoothedValue<float> gain; // 0 when disabled
    juce::SmoothedValue<float> delaySamples; // glides, and is read between samples, so changing it doesn't click
    std::array<float, 2> lowpassState { 0.0f, 0.0f };
    float lowpassCoefficient = 0.0f;

    static constexpr float lowpassHz = 700.0f;
};
