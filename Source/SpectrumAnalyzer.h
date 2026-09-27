/*
  ==============================================================================

    SpectrumAnalyzer.h

    Hands the processor's output from the audio thread to the UI through a lock-free
    FIFO, and turns it into a smoothed spectrum for the graph to draw.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SpectrumAnalyzer
{
public:
    static constexpr int fftOrder = 12;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int numBins = fftSize / 2;

    SpectrumAnalyzer()
        : fifo (fifoSize), fft (fftOrder),
          window ((size_t) fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        fifoBuffer.resize ((size_t) fifoSize, 0.0f);
        fftData.resize ((size_t) fftSize * 2, 0.0f);
        history.resize ((size_t) fftSize, 0.0f);
        magnitudesDb.resize ((size_t) numBins, minDb);
    }

    /// The UI turns this on while it's showing, so the audio thread doesn't do extra work otherwise.
    void setActive (bool shouldBeActive) { active = shouldBeActive; }

    /// Audio thread. Takes the average of the first two channels.
    void push (const juce::AudioBuffer<float>& buffer) noexcept
    {
        if (! active || buffer.getNumChannels() == 0)
            return;

        const int numSamples = buffer.getNumSamples();
        const auto* left = buffer.getReadPointer (0);
        const auto* right = buffer.getReadPointer (buffer.getNumChannels() > 1 ? 1 : 0);

        int start1, size1, start2, size2;
        fifo.prepareToWrite (numSamples, start1, size1, start2, size2);
        for (int i = 0; i < size1; ++i)
            fifoBuffer[(size_t) (start1 + i)] = 0.5f * (left[i] + right[i]);
        for (int i = 0; i < size2; ++i)
            fifoBuffer[(size_t) (start2 + i)] = 0.5f * (left[size1 + i] + right[size1 + i]);
        fifo.finishedWrite (size1 + size2);
    }

    /// Message thread. Reads whatever has arrived and updates the spectrum. Returns true if it changed.
    bool update()
    {
        int numReady = fifo.getNumReady();
        if (numReady == 0)
            return decay();

        // Keep a rolling window of the most recent fftSize samples
        std::vector<float> incoming ((size_t) numReady);
        int start1, size1, start2, size2;
        fifo.prepareToRead (numReady, start1, size1, start2, size2);
        std::copy_n (fifoBuffer.begin() + start1, size1, incoming.begin());
        std::copy_n (fifoBuffer.begin() + start2, size2, incoming.begin() + size1);
        fifo.finishedRead (size1 + size2);

        if (numReady >= fftSize)
        {
            std::copy (incoming.end() - fftSize, incoming.end(), history.begin());
        }
        else
        {
            std::move (history.begin() + numReady, history.end(), history.begin());
            std::copy (incoming.begin(), incoming.end(), history.end() - numReady);
        }

        std::fill (fftData.begin(), fftData.end(), 0.0f);
        std::copy (history.begin(), history.end(), fftData.begin());
        window.multiplyWithWindowingTable (fftData.data(), (size_t) fftSize);
        fft.performFrequencyOnlyForwardTransform (fftData.data());

        // Hann window loses half the amplitude; normalise so a full-scale sine reads about 0 dB
        const float scale = 4.0f / (float) fftSize;
        for (int bin = 0; bin < numBins; ++bin)
        {
            const float db = juce::Decibels::gainToDecibels (fftData[(size_t) bin] * scale, minDb);
            auto& shown = magnitudesDb[(size_t) bin];
            shown = db > shown ? db : shown + (db - shown) * releaseAmount; // fast attack, slow release
        }
        return true;
    }

    /// Smoothed magnitude in dB at this frequency.
    float getMagnitudeDb (float frequency, double sampleRate) const
    {
        const float binPosition = (float) (frequency * fftSize / sampleRate);
        const int bin = (int) binPosition;
        if (bin < 1 || bin >= numBins - 1)
            return minDb;
        const float frac = binPosition - (float) bin;
        return juce::jmap (frac, magnitudesDb[(size_t) bin], magnitudesDb[(size_t) bin + 1]);
    }

    /// The loudest smoothed bin between two frequencies, so drawing one point per pixel doesn't skip peaks.
    float getMaxMagnitudeDb (float lowFrequency, float highFrequency, double sampleRate) const
    {
        const int low = (int) (lowFrequency * fftSize / sampleRate);
        const int high = (int) (highFrequency * fftSize / sampleRate);
        if (high <= low + 1)
            return getMagnitudeDb (0.5f * (lowFrequency + highFrequency), sampleRate);

        float loudest = minDb;
        for (int bin = std::max (1, low); bin <= std::min (numBins - 1, high); ++bin)
            loudest = std::max (loudest, magnitudesDb[(size_t) bin]);
        return loudest;
    }

    static constexpr float minDb = -100.0f;

private:
    bool decay()
    {
        bool changed = false;
        for (auto& value : magnitudesDb)
        {
            if (value > minDb)
            {
                value = std::max (minDb, value - 1.5f);
                changed = true;
            }
        }
        return changed;
    }

    static constexpr int fifoSize = fftSize * 4;
    static constexpr float releaseAmount = 0.25f;

    std::atomic<bool> active { false };
    juce::AbstractFifo fifo;
    std::vector<float> fifoBuffer;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;
    std::vector<float> fftData, history, magnitudesDb;
};
