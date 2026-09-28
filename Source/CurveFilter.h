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

    Split ears: each ear's filter is the same minimum-phase filter for the average
    of the two curves, then a short linear-phase filter for that ear's difference
    from the average. Both ears get exactly the same phase, so the timing between
    them (which is how you hear where low sounds come from) is untouched; the
    price is a constant delay of half the linear-phase filter, about 11 ms, for as
    long as the ears are split.

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
    /// With `right` as well, `points` is the left ear's curve and `right` the right's.
    void setCurve (std::optional<std::vector<CurvePoint>> points, std::optional<std::vector<CurvePoint>> right = std::nullopt);

    /// While audio isn't running. Designs the current curve right away, so it's there from the first block.
    void prepare (const juce::dsp::ProcessSpec& spec);

    /// Audio thread
    void process (juce::dsp::AudioBlock<float>& block) noexcept;
    void reset() noexcept;

    /// The minimum-phase filter for a response, `length` samples long. Public for the tests.
    static juce::AudioBuffer<float> design (const std::function<float (float)>& dbAt, double sampleRate, int length);
    static int lengthFor (double sampleRate); // about a third of a second, as a power of two

    /// A linear-phase filter for a response: symmetric, `length` samples, so it delays by length / 2.
    static juce::AudioBuffer<float> designLinearPhase (const std::function<float (float)>& dbAt, double sampleRate, int length);
    static int splitLengthFor (double sampleRate); // about 21 ms, so the delay is about 11 ms
    /// Stereo filters for the two ears, with matched phase. Public for the tests.
    static juce::AudioBuffer<float> designSplit (const CurveResponse& left, const CurveResponse& right, double sampleRate);

private:
    void run() override;
    juce::AudioBuffer<float> designCurrent (double sampleRate);

    juce::dsp::Convolution convolution { juce::dsp::Convolution::NonUniform { 512 } };

    // What the message thread wants, for the design thread
    juce::CriticalSection requestLock;
    struct Request
    {
        std::vector<CurvePoint> left;
        std::optional<std::vector<CurvePoint>> right;
    };
    std::optional<Request> requested;
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
