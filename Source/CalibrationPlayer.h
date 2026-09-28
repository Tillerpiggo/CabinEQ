/*
  ==============================================================================

    CalibrationPlayer.h

    Plays pink noise bursts at each position of a grid, in reading order: left to
    right, top to bottom. Columns are where the sound is between the ears. Rows set
    where the sound starts: n rows draw n lines across 20 Hz to 20 kHz, evenly in
    octaves, cutting it into n + 1 sections, and each row's steep high-pass sits on
    one of those lines, the lowest line for the bottom row. So with 3 rows the cuts
    are at about 112 Hz, 632 Hz and 3.6 kHz. Each burst starts fast and dies away
    slowly, so its tail overlaps the next one.

    If some positions are selected, only those play.

    The setters can be called from any thread; process() is for the audio thread.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <set>
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
    bool isPlaying() const              { return playing; }

    /// The positions to play; none selected means all of them.
    void setSelection (const std::set<int>& positions)
    {
        juce::uint64 low = 0, high = 0;
        for (int position : positions)
        {
            if (position >= 0 && position < 64)        low |= (juce::uint64) 1 << position;
            else if (position >= 64 && position < 128) high |= (juce::uint64) 1 << (position - 64);
        }
        selectedLow = low;
        selectedHigh = high;
    }

    std::set<int> getSelection() const
    {
        std::set<int> positions;
        for (int position = 0; position < maxRows * maxColumns; ++position)
            if (isSelected (position))
                positions.insert (position);
        return positions;
    }

    bool isSelected (int position) const noexcept
    {
        if (position < 0 || position >= 128)
            return false;
        const auto bits = position < 64 ? selectedLow.load() : selectedHigh.load();
        return ((bits >> (position % 64)) & 1) != 0;
    }

    /// The high-pass cutoff for a row (0 is the top): the rows' lines cut 20 Hz to 20 kHz into rows + 1 sections.
    static double cutoffForRow (int row, int rows)
    {
        const int line = rows - row; // 1 for the bottom row, up to rows for the top
        return 20.0 * std::pow (1000.0, (double) line / (double) (rows + 1));
    }

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
        std::array<Biquad, 4> stages; // an 8th-order Butterworth high-pass, for a sharp cut

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

        // The next position in reading order, of the selected ones if any are
        bool anySelected = false;
        for (int p = 0; p < count && ! anySelected; ++p)
            anySelected = isSelected (p);
        for (int step = 1; step <= count; ++step)
        {
            const int candidate = (std::max (position, -1) + step) % count;
            if (! anySelected || isSelected (candidate))
            {
                position = candidate;
                break;
            }
        }
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

        // Each row up cuts off more of the lows. Pink noise has the same power in every octave,
        // so cutting some octaves off makes it quieter; make up for that so every row is as loud.
        const double cutoff = cutoffForRow (row, rows);
        voice->bandLimited = cutoff > 0.0;
        voice->peak = 1.0f;
        if (voice->bandLimited)
        {
            const double octavesLeft = std::log2 (20000.0 / cutoff), octavesAll = std::log2 (20000.0 / 20.0);
            voice->peak = (float) std::sqrt (octavesAll / std::max (0.5, octavesLeft));

            // Butterworth: four biquads with these Qs make a flat 8th-order high-pass
            const double qs[] { 0.5098, 0.6013, 0.9000, 2.5629 };
            for (size_t i = 0; i < voice->stages.size(); ++i)
                voice->stages[i] = Biquad { FilterDesign::design (Band::Shape::lowCut, cutoff, 0.0, qs[i], sampleRate) };
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

    std::atomic<int> numRows { defaultRows }, numColumns { defaultColumns };
    std::atomic<juce::uint64> selectedLow { 0 }, selectedHigh { 0 };
    std::atomic<float> level { juce::Decibels::decibelsToGain (-20.0f) };
    std::atomic<bool> playing { false };
    std::atomic<int> currentPosition { -1 };

    // Audio thread
    std::array<Voice, 6> voices {};
    int samplesUntilNext = 0, position = -1;
    juce::Random random { 42 };
    float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;
};
