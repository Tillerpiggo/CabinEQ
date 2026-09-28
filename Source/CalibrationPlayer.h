/*
  ==============================================================================

    CalibrationPlayer.h

    Plays pink noise bursts at each position of a grid, in reading order: left to
    right, top to bottom. Columns are where the sound is between the ears, rows are
    how high it is (the top row is the highest band of the spectrum). Each burst
    starts fast and dies away slowly, so its tail overlaps the next one.

    The setters can be called from any thread; process() is for the audio thread.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include "FilterDesign.h"

class CalibrationPlayer
{
public:
    static constexpr int maxRows = 8;
    static constexpr int maxColumns = 12;
    static constexpr int defaultRows = 3;
    static constexpr int defaultColumns = 5;

    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate;
        attackSamples = std::max (1, (int) (0.004 * sampleRate));
        releaseCoefficient = (float) std::exp (-1.0 / (0.16 * sampleRate)); // ~1 s to fade out fully
        for (auto& voice : voices)
            voice.active = false;
        samplesUntilNext = 0;
        position = -1;
    }

    void setGrid (int rows, int columns)
    {
        numRows = juce::jlimit (1, maxRows, rows);
        numColumns = juce::jlimit (1, maxColumns, columns);
    }

    void setLevelDb (float db)          { level = juce::Decibels::decibelsToGain (db); }
    void setPlaying (bool shouldPlay)   { playing = shouldPlay; }
    void setSolo (int index)            { solo = index; } // a position to repeat, or -1 for all of them
    bool isPlaying() const              { return playing; }
    int getSolo() const                 { return solo; }

    /// The position that played most recently, or -1 when stopped.
    int getCurrentPosition() const      { return playing ? currentPosition.load() : -1; }

    /// Audio thread. Adds the bursts to whatever's in the buffer, before the EQ.
    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        const bool isPlaying = playing.load();
        bool anyVoice = false;
        for (const auto& voice : voices)
            anyVoice |= voice.active;
        if (! isPlaying && ! anyVoice)
        {
            samplesUntilNext = 0;
            position = -1;
            return;
        }

        const int numChannels = buffer.getNumChannels();
        auto* left = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (numChannels > 1 ? 1 : 0);
        const float gain = level.load();
        const int interval = (int) (burstSeconds * sampleRate);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            if (isPlaying && --samplesUntilNext <= 0)
            {
                trigger();
                samplesUntilNext = interval;
            }

            const float noise = nextPink();
            float l = 0.0f, r = 0.0f;
            for (auto& voice : voices)
            {
                if (! voice.active)
                    continue;

                // Quick linear attack, then an exponential release
                float envelope;
                if (voice.age < attackSamples)
                    envelope = voice.peak * (float) voice.age / (float) attackSamples;
                else
                    envelope = (voice.peak *= releaseCoefficient);
                ++voice.age;

                if (voice.age > attackSamples && voice.peak < 1.0e-4f)
                {
                    voice.active = false;
                    continue;
                }

                const float sample = voice.bandLimited ? voice.filter (noise) : noise;
                l += sample * envelope * voice.leftGain;
                r += sample * envelope * voice.rightGain;
            }

            left[i] += l * gain;
            if (numChannels > 1)
                right[i] += r * gain;
            else
                left[i] += r * gain;
        }
    }

private:
    struct Biquad
    {
        FilterDesign::Biquad c;
        double s1 = 0, s2 = 0;
        float process (float x) noexcept
        {
            const double y = c.b0 * x + s1;
            s1 = c.b1 * x - c.a1 * y + s2;
            s2 = c.b2 * x - c.a2 * y;
            return (float) y;
        }
    };

    struct Voice
    {
        bool active = false, bandLimited = false;
        int age = 0;
        float peak = 0, leftGain = 0, rightGain = 0;
        std::array<Biquad, 4> stages; // two high-passes and two low-passes, for steeper band edges

        float filter (float x) noexcept
        {
            for (auto& stage : stages)
                x = stage.process (x);
            return x;
        }
    };

    void trigger() noexcept
    {
        const int rows = numRows.load(), columns = numColumns.load();
        const int count = rows * columns;
        const int soloIndex = solo.load();
        position = (soloIndex >= 0 && soloIndex < count) ? soloIndex : (position + 1) % count;
        currentPosition = position;

        const int row = position / columns, column = position % columns;

        // Take a free voice, or the quietest one
        Voice* voice = &voices[0];
        for (auto& candidate : voices)
        {
            if (! candidate.active) { voice = &candidate; break; }
            if (candidate.peak < voice->peak) voice = &candidate;
        }

        // Equal-power pan: left column fully left, right column fully right
        const float pan = columns > 1 ? (float) column / (float) (columns - 1) : 0.5f;
        voice->leftGain = std::cos (pan * juce::MathConstants<float>::halfPi);
        voice->rightGain = std::sin (pan * juce::MathConstants<float>::halfPi);

        // Split 60 Hz to 16 kHz into equal-octave bands, highest at the top.
        // Narrower bands carry less of the noise's power, so make up for it.
        voice->bandLimited = rows > 1;
        voice->peak = rows > 1 ? std::sqrt ((float) rows) : 1.0f;
        if (rows > 1)
        {
            const double logLow = std::log (60.0), logHigh = std::log (16000.0);
            const int band = rows - 1 - row;
            const double low = std::exp (logLow + (logHigh - logLow) * band / rows);
            const double high = std::exp (logLow + (logHigh - logLow) * (band + 1) / rows);
            const auto highPass = FilterDesign::design (Band::Shape::lowCut, low, 0.0, 0.7071, sampleRate);
            const auto lowPass = FilterDesign::design (Band::Shape::highCut, std::min (high, FilterDesign::maxFrequency (sampleRate)), 0.0, 0.7071, sampleRate);
            voice->stages = { Biquad { highPass }, Biquad { highPass }, Biquad { lowPass }, Biquad { lowPass } };
        }

        voice->age = 0;
        voice->active = true;
    }

    /// Paul Kellet's pink noise, scaled to 0 dB RMS (its raw RMS is about 1.74), so the level is the burst's RMS
    float nextPink() noexcept
    {
        const float white = random.nextFloat() * 2.0f - 1.0f;
        b0 = 0.99886f * b0 + white * 0.0555179f;
        b1 = 0.99332f * b1 + white * 0.0750759f;
        b2 = 0.96900f * b2 + white * 0.1538520f;
        b3 = 0.86650f * b3 + white * 0.3104856f;
        b4 = 0.55000f * b4 + white * 0.5329522f;
        b5 = -0.7616f * b5 - white * 0.0168980f;
        const float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
        b6 = white * 0.115926f;
        return pink * (1.0f / 1.745f);
    }

    static constexpr double burstSeconds = 0.3; // time between bursts

    double sampleRate = 48000.0;
    int attackSamples = 192;
    float releaseCoefficient = 0.9999f;

    std::atomic<int> numRows { defaultRows }, numColumns { defaultColumns }, solo { -1 };
    std::atomic<float> level { juce::Decibels::decibelsToGain (-20.0f) };
    std::atomic<bool> playing { false };
    std::atomic<int> currentPosition { -1 };

    // Audio thread
    std::array<Voice, 6> voices {};
    int samplesUntilNext = 0, position = -1;
    juce::Random random { 42 };
    float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;
};
