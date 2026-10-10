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

    With a depth above 1, each position plays that many times before moving on, getting
    louder in even steps from the floor up to full level: depth 3 with a floor of -20 dB
    plays it at -20, -10, then 0 dB.

    If some positions are selected, only those play.

    In spots mode it plays 2 to 4 spots instead of the grid, which you place on the EQ
    graph. Each spot plays from its own frequency (a sharp low cut) up to 20 kHz. They
    share a pan range: with more than one pan step, each spot plays at that many
    positions across it, left to right, and at each one does its whole depth run.

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
    static constexpr int maxDepth = 5;
    static constexpr float minRate = 0.5f, maxRate = 8.0f, defaultRate = 2.5f; // bursts per second
    static constexpr float maxReleaseMs = 3000.0f, defaultReleaseMs = 1500.0f; // how long a burst takes to fade out (by 60 dB)
    static constexpr float maxAttackMs = 200.0f, defaultAttackMs = 4.0f; // how long a burst takes to reach full level
    static constexpr int maxSpots = 4;
    static constexpr int maxPanSteps = 5;

    enum class Mode { grid = 0, spots = 1 };
    static constexpr float minFloorDb = -60.0f, defaultFloorDb = -20.0f; // the quietest of a depth run, below the volume

    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate;
        {
            // Audio isn't running, so the tilted noise can go straight in
            const juce::ScopedLock build (buildLock);
            makePinkSpectrum();
            builtTilt = wantedTilt();
            tiltedNoise = makeTiltedNoise (builtTilt);
            const juce::SpinLock::ScopedLockType lock (pendingLock);
            pendingNoise.clear();
            hasPending = false;
        }
        tiltedIndex = 0;
        for (auto& voice : voices)
            voice.active = false;
        samplesUntilNext = 0;
        position = -1;
        repeat = 0;
    }

    void setDepth (int newDepth)        { depth = juce::jlimit (1, maxDepth, newDepth); }
    /// How far below the volume a depth run starts. The repeats climb from there to the volume in even steps.
    void setFloorDb (float newFloorDb)  { floorDb = juce::jlimit (minFloorDb, 0.0f, newFloorDb); }

    /// What the bursts are made of. Pink falls 3 dB an octave. The other two are pink through a filter that
    /// tilts it further (level with pink at 20 Hz), so each burst falls away faster: by its mode's slope, which
    /// can be changed. What a mode fixes is its top line: how fast the bursts' bottom edges fall from burst
    /// to burst. Each burst is turned up, by its lowest frequency, by whatever keeps them on that line.
    ///   minus4_5  top line 3 dB an octave, as pink's is. Its slope starts at 4.5.
    ///   minus6    top line 4.5 dB an octave. Its slope starts at 6.
    enum class Noise { pink = 0, minus4_5 = 1, minus6 = 2 };
    static constexpr float maxSlopeDbPerOctave = 12.0f;
    static constexpr float topLineDbPerOctave (Noise noise)      { return noise == Noise::minus6 ? 4.5f : 3.0f; }
    static constexpr float defaultSlopeDbPerOctave (Noise noise) { return noise == Noise::minus6 ? 6.0f : noise == Noise::minus4_5 ? 4.5f : 3.0f; }

    /// Message thread. A change of tilt makes new noise here, and the audio picks it up at its next block.
    void setNoise (Noise newNoise)
    {
        noiseType = juce::jlimit (0, 2, (int) newNoise);
        updateTiltedNoise();
    }
    Noise getNoise() const { return (Noise) noiseType.load(); }

    /// How fast each burst falls away, in dB an octave (as a positive number). No shallower than the mode's top line.
    void setSlope (Noise noise, float dbPerOctave)
    {
        if (noise == Noise::pink)
            return;
        slopes[(size_t) noise - 1] = juce::jlimit (topLineDbPerOctave (noise), maxSlopeDbPerOctave, dbPerOctave);
        updateTiltedNoise();
    }
    float getSlope (Noise noise) const { return noise == Noise::pink ? 3.0f : slopes[(size_t) noise - 1].load(); }

    /// How much each burst is turned up per octave its lowest frequency is above 20 Hz: what the slope loses
    /// that the top line doesn't
    float burstBoostDbPerOctave() const { return getSlope (getNoise()) - topLineDbPerOctave (getNoise()); }
    void setRate (float burstsPerSecond) { rate = juce::jlimit (minRate, maxRate, burstsPerSecond); }
    /// How long each burst takes to fade out after its attack. 0 cuts it off straight away, leaving just the attack.
    void setReleaseMs (float milliseconds) { releaseMs = juce::jlimit (0.0f, maxReleaseMs, milliseconds); }
    /// How long each burst takes to rise to full level. Longer is softer; 0 starts at full level.
    void setAttackMs (float milliseconds) { attackMs = juce::jlimit (0.0f, maxAttackMs, milliseconds); }

    void setMode (Mode newMode)          { mode = (int) newMode; }
    Mode getMode() const                 { return (Mode) mode.load(); }
    void setSpotCount (int count)        { spotCount = juce::jlimit (1, maxSpots, count); }

    /// A spot's base frequency: its low cut (20 Hz or below means none)
    void setSpot (int index, float frequency)
    {
        if (index >= 0 && index < maxSpots)
            spotFrequency[(size_t) index] = frequency;
    }

    /// The pan range the spots play across, -1 (left) to 1 (right), and in how many steps
    void setPanRange (float low, float high)
    {
        panLow = juce::jlimit (-1.0f, 1.0f, std::min (low, high));
        panHigh = juce::jlimit (-1.0f, 1.0f, std::max (low, high));
    }
    void setPanSteps (int steps)         { panSteps = juce::jlimit (1, maxPanSteps, steps); }

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
            repeat = 0;
            return;
        }

        const int numChannels = buffer.getNumChannels();
        auto* left = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (numChannels > 1 ? 1 : 0);
        const float gain = level.load();
        const int interval = std::max (1, (int) (sampleRate / rate.load()));
        // Take newly made noise, if there is some and the message thread isn't mid-handover
        if (hasPending.load())
        {
            const juce::SpinLock::ScopedTryLockType lock (pendingLock);
            if (lock.isLocked())
            {
                std::swap (tiltedNoise, pendingNoise); // the old one's freed by the message thread, later
                hasPending = false;
                if (tiltedIndex >= tiltedNoise.size())
                    tiltedIndex = 0;
            }
        }
        const std::vector<float>* tilted = getNoise() != Noise::pink && ! tiltedNoise.empty() ? &tiltedNoise : nullptr;

        const int attackSamples = std::max (1, (int) (attackMs.load() * 0.001 * sampleRate));

        // The release falls 60 dB (6.9 time constants) in the time asked for; never faster than 1 ms, so "0" doesn't click
        const double releaseSeconds = std::max (0.001, (double) releaseMs.load() * 0.001);
        const float releaseCoefficient = (float) std::exp (-6.908 / (releaseSeconds * sampleRate));

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            if (isPlaying && --samplesUntilNext <= 0)
            {
                trigger();
                samplesUntilNext = interval;
            }
            else if (samplesUntilNext > interval)
            {
                samplesUntilNext = interval; // sped up mid-wait
            }

            float noise;
            if (tilted != nullptr)
            {
                noise = (*tilted)[tiltedIndex];
                if (++tiltedIndex >= tilted->size())
                    tiltedIndex = 0;
            }
            else
            {
                noise = nextPink();
            }
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
        const bool spots = mode.load() == (int) Mode::spots;
        const int rows = numRows.load(), columns = numColumns.load();
        const int count = spots ? spotCount.load() : rows * columns;

        bool anySelected = false;
        for (int p = 0; ! spots && p < count && ! anySelected; ++p)
            anySelected = isSelected (p);

        // Play the same position again, louder, until it's played `depth` times
        const int depthRuns = depth.load();
        const int panPositions = spots ? panSteps.load() : 1;
        const int timesEach = depthRuns * panPositions; // a depth run at each pan position
        const bool canRepeat = position >= 0 && position < count && (! anySelected || isSelected (position));
        if (canRepeat && repeat + 1 < timesEach)
        {
            ++repeat;
        }
        else
        {
            // Otherwise the next position in reading order, of the selected ones if any are
            repeat = 0;
            for (int step = 1; step <= count; ++step)
            {
                const int candidate = (std::max (position, -1) + step) % count;
                if (! anySelected || isSelected (candidate))
                {
                    position = candidate;
                    break;
                }
            }
        }
        currentPosition = position;

        // Where it is between the ears (0 = left, 1 = right), and where its low cut is
        float pan;
        double cutoff;
        if (spots)
        {
            // Left to right across the range, one pan position per depth run
            const float low = panLow.load(), high = panHigh.load();
            const int panIndex = repeat / depthRuns;
            const float spread = panPositions > 1 ? low + (high - low) * (float) panIndex / (float) (panPositions - 1)
                                                  : 0.5f * (low + high);
            pan = 0.5f * (spread + 1.0f);
            cutoff = spotFrequency[(size_t) position].load();
            if (cutoff <= 20.0)
                cutoff = 0.0;
        }
        else
        {
            const int row = position / columns, column = position % columns;
            pan = columns > 1 ? (float) column / (float) (columns - 1) : 0.5f;
            cutoff = cutoffForRow (row, rows);
        }

        // Take a free voice, or the quietest one
        Voice* voice = &voices[0];
        for (auto& candidate : voices)
        {
            if (! candidate.active) { voice = &candidate; break; }
            if (candidate.peak < voice->peak) voice = &candidate;
        }

        // Equal-power pan
        voice->leftGain = std::cos (pan * juce::MathConstants<float>::halfPi);
        voice->rightGain = std::sin (pan * juce::MathConstants<float>::halfPi);

        // Pink noise has the same power in every octave, so cutting some octaves off makes it
        // quieter; make up for that so every row (or spot) is as loud.
        cutoff = std::min (cutoff, FilterDesign::maxFrequency (sampleRate) * 0.9);
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

        // Tilted noise: up for every octave this burst's lowest frequency is above 20 Hz, to keep it on the top line
        if (getNoise() != Noise::pink)
            voice->peak *= juce::Decibels::decibelsToGain (burstBoostDbPerOctave() * (float) std::log2 (std::max (20.0, cutoff) / 20.0));

        // Quietest first: from the floor up to full level, in even steps
        if (depthRuns > 1)
            voice->peak *= juce::Decibels::decibelsToGain (floorDb.load() * (float) (depthRuns - 1 - repeat % depthRuns) / (float) (depthRuns - 1));

        voice->age = 0;
        voice->active = true;
    }

    /// How far the noise wanted now is tilted beyond pink, in dB an octave
    float wantedTilt() const { return getSlope (getNoise()) - 3.0f; }

    /// Makes the noise for a new tilt, if it's changed, and leaves it for the audio thread to pick up
    void updateTiltedNoise()
    {
        const juce::ScopedLock build (buildLock);
        const float tilt = wantedTilt();
        if (getNoise() == Noise::pink || pinkSpectrum.empty() || std::abs (tilt - builtTilt) < 1.0e-4f)
            return;

        auto noise = makeTiltedNoise (tilt);
        builtTilt = tilt;
        const juce::SpinLock::ScopedLockType lock (pendingLock);
        pendingNoise = std::move (noise);
        hasPending = true;
    }

    static constexpr int noiseOrder = 18; // 262144 samples: 5.5 s at 48 kHz

    /// The spectrum of a few seconds of pink noise, kept so that tilting it again is quick
    void makePinkSpectrum()
    {
        const int size = 1 << noiseOrder;
        juce::dsp::FFT fft (noiseOrder);
        pinkSpectrum.assign ((size_t) size * 2, 0.0f);
        for (int i = 0; i < size; ++i)
            pinkSpectrum[(size_t) i] = nextPink();
        fft.performRealOnlyForwardTransform (pinkSpectrum.data());

        octavesAbove20.resize ((size_t) size);
        for (int bin = 0; bin < size; ++bin)
            octavesAbove20[(size_t) bin] = (float) std::log2 (std::max (20.0, (double) std::min (bin, size - bin) * sampleRate / size) / 20.0);
    }

    /// That pink noise through a filter that tilts it down (level with pink at 20 Hz), to loop. The filter is
    /// applied to the whole stretch at once, in the frequency domain, which is the same as a long FIR filter
    /// that wraps round, so the loop has no seam, and playing it costs nothing.
    std::vector<float> makeTiltedNoise (float dbPerOctave) const
    {
        const int size = 1 << noiseOrder;
        juce::dsp::FFT fft (noiseOrder);
        auto data = pinkSpectrum;

        double expectedPower = 0.0;
        const float exponent = -dbPerOctave / 6.0206f;
        for (int bin = 0; bin < size; ++bin)
        {
            const float tilt = bin == 0 ? 0.0f : std::exp2 (exponent * octavesAbove20[(size_t) bin]);
            data[(size_t) bin * 2] *= tilt;
            data[(size_t) bin * 2 + 1] *= tilt;
            expectedPower += (double) data[(size_t) bin * 2] * data[(size_t) bin * 2] + (double) data[(size_t) bin * 2 + 1] * data[(size_t) bin * 2 + 1];
        }
        fft.performRealOnlyInverseTransform (data.data());

        // Parseval says how loud it should have come out, whichever way the FFT scales its inverse
        double power = 0.0;
        for (int i = 0; i < size; ++i)
            power += (double) data[(size_t) i] * data[(size_t) i];
        const float scale = power > 0.0 ? (float) std::sqrt (expectedPower / size / power) : 0.0f;

        data.resize ((size_t) size);
        for (auto& sample : data)
            sample *= scale;
        return data;
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

    double sampleRate = 48000.0;

    std::atomic<int> numRows { defaultRows }, numColumns { defaultColumns }, depth { 1 };
    std::atomic<juce::uint64> selectedLow { 0 }, selectedHigh { 0 };
    std::atomic<float> level { juce::Decibels::decibelsToGain (-20.0f) };
    std::atomic<float> rate { defaultRate };
    std::atomic<float> releaseMs { defaultReleaseMs }, attackMs { defaultAttackMs };
    std::atomic<float> floorDb { defaultFloorDb };
    std::atomic<int> noiseType { (int) Noise::pink };
    std::array<std::atomic<float>, 2> slopes { defaultSlopeDbPerOctave (Noise::minus4_5), defaultSlopeDbPerOctave (Noise::minus6) };

    // The tilted noise the audio thread plays, and the next one, made on the message thread
    std::vector<float> tiltedNoise;
    size_t tiltedIndex = 0;
    juce::CriticalSection buildLock; // makers only: never the audio thread
    std::vector<float> pinkSpectrum, octavesAbove20;
    float builtTilt = 0.0f;
    juce::SpinLock pendingLock;
    std::vector<float> pendingNoise;
    std::atomic<bool> hasPending { false };
    std::atomic<int> mode { (int) Mode::spots }, spotCount { 3 }, panSteps { 1 };
    std::array<std::atomic<float>, maxSpots> spotFrequency { 200.0f, 1000.0f, 5000.0f, 12000.0f };
    std::atomic<float> panLow { 0.0f }, panHigh { 0.0f };
    std::atomic<bool> playing { false };
    std::atomic<int> currentPosition { -1 };

    // Audio thread
    std::array<Voice, 10> voices {};
    int samplesUntilNext = 0, position = -1, repeat = 0;
    juce::Random random { 42 };
    float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;
};
