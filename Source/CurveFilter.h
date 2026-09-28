/*
  ==============================================================================

    CurveFilter.h

    Plays the "Curve" EQ: a minimum-phase FIR filter designed from the curve's
    points, run with JUCE's partitioned convolution.

    Designing: the curve's magnitude is sampled on a fine frequency grid, and the
    cepstral (homomorphic) method turns it into the minimum-phase filter with that
    magnitude. Minimum phase keeps the delay near zero (unlike linear phase), so
    audio stays in sync with video.

    Real time: while you drag, the message thread asks for a new filter. A
    background thread designs it (a few milliseconds), and hands it to the audio
    thread through a slot it only ever try-locks. The audio thread loads it into
    juce::dsp::Convolution, which crossfades from the old filter to the new one,
    so it never clicks. Requests that come in faster than that replace each other.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <optional>
#include "CurveResponse.h"

class CurveFilter : private juce::Thread
{
public:
    CurveFilter();
    ~CurveFilter() override;

    /// Message thread. Nothing (std::nullopt) means the curve's not in use: it fades out and stops costing anything.
    void setCurve (std::optional<std::vector<CurvePoint>> points);

    /// While audio isn't running. Designs the current curve right away, so it's there from the first block.
    void prepare (const juce::dsp::ProcessSpec& spec);

    /// Audio thread
    void process (juce::dsp::AudioBlock<float>& block) noexcept;
    void reset() noexcept;

    /// The minimum-phase filter for a response, `length` samples long. Public for the tests.
    static juce::AudioBuffer<float> design (const std::function<float (float)>& dbAt, double sampleRate, int length);
    static int lengthFor (double sampleRate); // about a third of a second, as a power of two

private:
    void run() override;
    juce::AudioBuffer<float> designCurrent (double sampleRate);

    juce::dsp::Convolution convolution { juce::dsp::Convolution::NonUniform { 512 } };

    // What the message thread wants, for the design thread
    juce::CriticalSection requestLock;
    std::optional<std::vector<CurvePoint>> requested;
    bool hasNewRequest = false, hasEverBeenAsked = false;
    std::atomic<double> sampleRate { 48000.0 };

    // Finished filters, for the audio thread
    juce::SpinLock readyLock;
    juce::AudioBuffer<float> ready;
    bool hasReady = false;
    bool readyIsActive = false;

    // Audio thread
    bool isActive = false;
    int samplesSinceInactive = 0;
    int fadeSamples = 4800;
};
