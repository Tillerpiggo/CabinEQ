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

/// The audio path: EQ bands, then crossfeed, then gain, with a click-free bypass.
/// setBands() is for the message thread; the other setters are safe from any thread.
class PlaybackManager
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void processBlock (juce::AudioBuffer<float>& buffer) noexcept;

    void setBands (const std::vector<Band>& bands);
    void setGainDb (float gainDb); // preamp plus auto gain
    void setBypassed (bool shouldBeBypassed);

    CrossfeedProcessor& getCrossfeed() { return crossfeed; }
    SpectrumAnalyzer& getAnalyzer() { return analyzer; }

private:
    void processChunk (juce::AudioBuffer<float>& buffer, int start, int length) noexcept;

    FilterChain filter;
    CrossfeedProcessor crossfeed;
    SpectrumAnalyzer analyzer;

    std::atomic<float> gainDb { 0.0f };
    std::atomic<bool> bypassed { false };

    juce::SmoothedValue<float> gain { 1.0f };
    juce::SmoothedValue<float> wetMix { 1.0f }; // 1 = EQ on, 0 = bypassed
    bool isFullyBypassed = false;

    juce::AudioBuffer<float> dryBuffer; // the input, for crossfading in and out of bypass
    std::vector<float> gainRamp, mixRamp;
    int maxChunkSize = 0;
};
