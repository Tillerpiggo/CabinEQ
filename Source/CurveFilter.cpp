/*
  ==============================================================================

    CurveFilter.cpp

  ==============================================================================
*/

#include "CurveFilter.h"

namespace
{
    using Complex = std::complex<float>;

    // juce::dsp::FFT may or may not scale its inverse; find out once, so the maths below doesn't care
    float inverseScale (juce::dsp::FFT& fft)
    {
        const int size = fft.getSize();
        std::vector<Complex> in ((size_t) size, Complex (1.0f, 0.0f)), out ((size_t) size);
        fft.perform (in.data(), out.data(), true);
        return std::abs (out[0].real() - 1.0f) < 1.0e-3f ? 1.0f : 1.0f / (float) size;
    }
}

CurveFilter::CurveFilter() : juce::Thread ("CabinEQ curve designer")
{
    startThread (juce::Thread::Priority::normal);
}

CurveFilter::~CurveFilter()
{
    signalThreadShouldExit();
    notify();
    stopThread (2000);
}

int CurveFilter::lengthFor (double rate)
{
    return juce::nextPowerOfTwo ((int) (rate * 0.3)); // 16384 at 44.1 and 48 kHz: enough for detail down to 20 Hz
}

juce::AudioBuffer<float> CurveFilter::design (const std::function<float (float)>& dbAt, double rate, int length)
{
    // Work in a grid four times the filter's length, so the cepstrum barely wraps around
    const int size = length * 4;
    juce::dsp::FFT fft (juce::roundToInt (std::log2 ((double) size)));
    const float scale = inverseScale (fft);

    // 1. The log of the magnitude we want, on both halves of the spectrum
    std::vector<Complex> spectrum ((size_t) size), cepstrum ((size_t) size);
    for (int k = 0; k <= size / 2; ++k)
    {
        const double frequency = juce::jlimit (10.0, rate * 0.5, (double) k * rate / size);
        const float logMagnitude = dbAt ((float) frequency) * std::log (10.0f) / 20.0f;
        spectrum[(size_t) k] = Complex (logMagnitude, 0.0f);
        if (k > 0 && k < size / 2)
            spectrum[(size_t) (size - k)] = spectrum[(size_t) k];
    }

    // 2. Its cepstrum, folded onto positive time: that's what makes it minimum phase
    fft.perform (spectrum.data(), cepstrum.data(), true);
    for (int n = 0; n < size; ++n)
    {
        const float value = cepstrum[(size_t) n].real() * scale;
        const float folded = (n == 0 || n == size / 2) ? value : (n < size / 2 ? 2.0f * value : 0.0f);
        cepstrum[(size_t) n] = Complex (folded, 0.0f);
    }

    // 3. Back to a spectrum, exponentiate (log magnitude and phase back to a response), and to time
    fft.perform (cepstrum.data(), spectrum.data(), false);
    for (auto& value : spectrum)
        value = std::exp (value);
    std::vector<Complex> response ((size_t) size);
    fft.perform (spectrum.data(), response.data(), true);

    // 4. Keep the start, which is where a minimum-phase filter's energy is, and fade out its very end
    juce::AudioBuffer<float> impulse (1, length);
    auto* samples = impulse.getWritePointer (0);
    const int fade = length / 8;
    for (int n = 0; n < length; ++n)
    {
        float window = 1.0f;
        if (n >= length - fade)
            window = 0.5f * (1.0f + std::cos (juce::MathConstants<float>::pi * (float) (n - (length - fade)) / (float) fade));
        samples[n] = response[(size_t) n].real() * scale * window;
    }
    return impulse;
}

int CurveFilter::splitLengthFor (double rate)
{
    return juce::nextPowerOfTwo ((int) (rate * 0.02)); // 1024 at 44.1 and 48 kHz
}

juce::AudioBuffer<float> CurveFilter::designLinearPhase (const std::function<float (float)>& dbAt, double rate, int length)
{
    juce::dsp::FFT fft (juce::roundToInt (std::log2 ((double) length)));
    const float scale = inverseScale (fft);

    // The magnitude with no phase at all, which is symmetric around time zero...
    std::vector<Complex> spectrum ((size_t) length), response ((size_t) length);
    for (int k = 0; k <= length / 2; ++k)
    {
        const double frequency = juce::jlimit (10.0, rate * 0.5, (double) k * rate / length);
        spectrum[(size_t) k] = Complex (juce::Decibels::decibelsToGain (dbAt ((float) frequency), -200.0f), 0.0f);
        if (k > 0 && k < length / 2)
            spectrum[(size_t) (length - k)] = spectrum[(size_t) k];
    }
    fft.perform (spectrum.data(), response.data(), true);

    // ...moved to the middle, and windowed so it ends smoothly: symmetric around length / 2, so linear phase
    juce::AudioBuffer<float> impulse (1, length);
    for (int n = 0; n < length; ++n)
    {
        const float window = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * (float) n / (float) length);
        impulse.setSample (0, n, response[(size_t) ((n + length / 2) % length)].real() * scale * window);
    }
    return impulse;
}

juce::AudioBuffer<float> CurveFilter::designSplit (const std::function<float (float)>& leftDb, const std::function<float (float)>& rightDb,
                                                  double rate)
{
    // What the ears share, as one minimum-phase filter, and how far each is from it, as a linear-phase one
    auto shared = design ([&] (float f) { return 0.5f * (leftDb (f) + rightDb (f)); }, rate, lengthFor (rate));
    const int splitLength = splitLengthFor (rate);
    const int length = shared.getNumSamples() + splitLength - 1;

    const int size = juce::nextPowerOfTwo (length);
    juce::dsp::FFT fft (juce::roundToInt (std::log2 ((double) size)));
    const float scale = inverseScale (fft);

    auto toSpectrum = [&] (const juce::AudioBuffer<float>& impulse)
    {
        std::vector<Complex> time ((size_t) size), spectrum ((size_t) size);
        for (int n = 0; n < impulse.getNumSamples(); ++n)
            time[(size_t) n] = Complex (impulse.getSample (0, n), 0.0f);
        fft.perform (time.data(), spectrum.data(), false);
        return spectrum;
    };
    const auto sharedSpectrum = toSpectrum (shared);

    juce::AudioBuffer<float> result (2, length);
    for (int ear = 0; ear < 2; ++ear)
    {
        const float sign = ear == 0 ? 1.0f : -1.0f;
        auto difference = designLinearPhase ([&] (float f) { return sign * 0.5f * (leftDb (f) - rightDb (f)); },
                                             rate, splitLength);
        auto spectrum = toSpectrum (difference);
        for (size_t k = 0; k < spectrum.size(); ++k)
            spectrum[k] *= sharedSpectrum[k];

        std::vector<Complex> time ((size_t) size);
        fft.perform (spectrum.data(), time.data(), true);
        for (int n = 0; n < length; ++n)
            result.setSample (ear, n, time[(size_t) n].real() * scale);
    }
    return result;
}

juce::AudioBuffer<float> CurveFilter::designCurrent (double rate)
{
    std::optional<Request> request;
    {
        const juce::ScopedLock lock (requestLock);
        request = requested;
        hasNewRequest = false;
    }

    // Always two channels, one per ear, so switching between split and not is just another filter
    auto bothEars = [] (juce::AudioBuffer<float> mono)
    {
        juce::AudioBuffer<float> stereo (2, mono.getNumSamples());
        stereo.copyFrom (0, 0, mono, 0, 0, mono.getNumSamples());
        stereo.copyFrom (1, 0, mono, 0, 0, mono.getNumSamples());
        return stereo;
    };

    if (! request.has_value())
    {
        // Not in use: a filter that does nothing, to fade to
        juce::AudioBuffer<float> identity (1, 1);
        identity.setSample (0, 0, 1.0f);
        return bothEars (std::move (identity));
    }

    CurveResponse curve (request->points);
    if (request->tweaks.has_value())
    {
        CurveResponse left ((*request->tweaks)[0]), right ((*request->tweaks)[1]);
        return designSplit ([&] (float f) { return curve.dbAtFrequency (f) + left.dbAtFrequency (f); },
                            [&] (float f) { return curve.dbAtFrequency (f) + right.dbAtFrequency (f); }, rate);
    }
    return bothEars (design ([&curve] (float frequency) { return curve.dbAtFrequency (frequency); }, rate, lengthFor (rate)));
}

void CurveFilter::setCurve (std::optional<std::vector<CurvePoint>> points, std::optional<EarTweaks> tweaks)
{
    auto samePoints = [] (const std::vector<CurvePoint>& a, const std::vector<CurvePoint>& b)
    {
        return std::equal (a.begin(), a.end(), b.begin(), b.end(), [] (const CurvePoint& x, const CurvePoint& y)
        {
            return x.freq == y.freq && x.gain == y.gain;
        });
    };
    auto same = [&] (const std::optional<Request>& a, const std::optional<Request>& b)
    {
        if (a.has_value() != b.has_value())
            return false;
        if (! a.has_value())
            return true;
        if (a->tweaks.has_value() != b->tweaks.has_value())
            return false;
        return samePoints (a->points, b->points)
            && (! a->tweaks.has_value() || (samePoints ((*a->tweaks)[0], (*b->tweaks)[0]) && samePoints ((*a->tweaks)[1], (*b->tweaks)[1])));
    };

    std::optional<Request> request;
    if (points.has_value())
        request = Request { std::move (*points), std::move (tweaks) };

    {
        // Everything that changes the plugin's state asks; only redesign when the curve really changed
        const juce::ScopedLock lock (requestLock);
        if (hasEverBeenAsked && same (requested, request))
            return;
        hasEverBeenAsked = true;
        requested = std::move (request);
        hasNewRequest = true;
    }
    notify();
}

void CurveFilter::run()
{
    while (! threadShouldExit())
    {
        wait (-1);

        while (! threadShouldExit())
        {
            bool isNew, active;
            {
                const juce::ScopedLock lock (requestLock);
                isNew = hasNewRequest;
                active = requested.has_value();
            }
            if (! isNew)
                break;

            auto impulse = designCurrent (sampleRate.load());

            // Hand it over; if the audio thread hasn't taken the last one yet, this one replaces it
            const juce::SpinLock::ScopedLockType lock (readyLock);
            ready = std::move (impulse);
            hasReady = true;
            readyIsActive = active;
        }
    }
}

void CurveFilter::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    fadeSamples = (int) (spec.sampleRate * 0.1);

    // Design what's wanted now, here, so it's loaded before the first block
    bool active;
    {
        const juce::ScopedLock lock (requestLock);
        active = requested.has_value();
    }
    auto impulse = designCurrent (spec.sampleRate);
    convolution.loadImpulseResponse (std::move (impulse), spec.sampleRate, juce::dsp::Convolution::Stereo::yes,
                                     juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
    convolution.prepare (spec);

    {
        const juce::SpinLock::ScopedLockType lock (readyLock);
        hasReady = false;
    }
    isActive = active;
    samplesSinceInactive = isActive ? 0 : fadeSamples;
}

void CurveFilter::reset() noexcept
{
    convolution.reset();
}

void CurveFilter::process (juce::dsp::AudioBlock<float>& block) noexcept
{
    // Take a newly designed filter, if there is one and the design thread isn't mid-handover
    {
        const juce::SpinLock::ScopedTryLockType lock (readyLock);
        if (lock.isLocked() && hasReady)
        {
            const bool wasActive = isActive;
            isActive = readyIsActive;
            if (isActive && ! wasActive && samplesSinceInactive >= fadeSamples)
                convolution.reset(); // it's been idle; start from silence rather than stale history

            // Wait-free: the Convolution takes ownership, and does its own preparing in the background
            convolution.loadImpulseResponse (std::move (ready), sampleRate.load(), juce::dsp::Convolution::Stereo::yes,
                                             juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
            hasReady = false;
        }
    }

    // When the curve isn't in use, keep going just long enough to fade to the do-nothing filter
    if (! isActive)
    {
        if (samplesSinceInactive >= fadeSamples)
            return;
        samplesSinceInactive += (int) block.getNumSamples();
    }
    else
    {
        samplesSinceInactive = 0;
    }

    juce::dsp::ProcessContextReplacing<float> context (block);
    convolution.process (context);
}
