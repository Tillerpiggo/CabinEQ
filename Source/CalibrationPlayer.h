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

    With a depth above 1, each row plays that many times before the next row does, a level louder
    each time, in even steps from the floor up to full level: depth 3 with a floor of -20 dB plays
    a row at -20, then at -10, then at 0 dB, and then the next row starts from -20. With repeats
    above 1, the row plays that many times at each of those levels before going up one. The depth
    can work on each dot, or on the whole cycle of them, instead of on each row.

    The noise falls 4.5 dB an octave (pink falls 3), and so do the bursts' bottom edges from one
    burst to the next: every burst is the same noise, cut off lower or higher.

    If some positions are selected, only those play.

    In spots mode it plays 2 to 4 spots instead of the grid, which you place on the EQ
    graph. Each spot plays from its own frequency (a sharp low cut) up to 20 kHz. They
    share a pan range: with more than one pan step, each spot plays at that many
    positions across it, left to right, before the next spot plays.

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
    static constexpr int maxRows = 12;
    static constexpr int maxColumns = 18;
    static constexpr int defaultRows = 3;
    static constexpr int defaultColumns = 5;
    static constexpr int maxDepth = 5;
    static constexpr float minRate = 0.5f, maxRate = 64.0f, defaultRate = 2.5f; // bursts per second
    static constexpr float maxReleaseMs = 3000.0f, defaultReleaseMs = 1500.0f; // how long a burst takes to fade out (by 60 dB)
    static constexpr float maxAttackMs = 200.0f, defaultAttackMs = 4.0f; // how long a burst takes to reach full level
    static constexpr int maxSpots = 4;
    static constexpr int maxPanSteps = 5;

    enum class Mode { grid = 0, spots = 1, square = 2, sequence = 3, range = 4 };

    /// Range mode: a range you choose, split into a bottom, a middle and a top, each a sharp band (48 dB an
    /// octave at both edges), played top to bottom, round and round, so everything it plays is inside the range.
    /// The middle takes this share of the range, in octaves, in the middle of it; the top and the bottom split
    /// what's left. It shares the spots' pan range and pan steps: each band plays at that many positions across
    /// it, left to right, before the next.
    static constexpr float defaultRangeLow = 100.0f, defaultRangeHigh = 10000.0f, defaultRangeMiddle = 0.5f;
    static constexpr float minRangeMiddle = 0.05f, maxRangeMiddle = 0.95f;
    void setRange (float low, float high, float middle)
    {
        low = juce::jlimit (20.0f, 20000.0f, low);
        high = juce::jlimit (20.0f, 20000.0f, high);
        rangeLow = std::min (low, high);
        rangeHigh = std::max (std::max (low, high), std::min (low, high) * 1.01f);
        rangeMiddle = juce::jlimit (minRangeMiddle, maxRangeMiddle, middle);
    }
    float getRangeLow() const    { return rangeLow.load(); }
    float getRangeHigh() const   { return rangeHigh.load(); }
    float getRangeMiddle() const { return rangeMiddle.load(); }

    /// The three bands' edges, from the bottom up: the range's bottom, the middle's bottom, its top, the range's top
    static std::array<double, 4> rangeEdges (double low, double high, double middle)
    {
        const double outside = 0.5 * (1.0 - middle) * std::log2 (high / low); // each of the top and bottom, in octaves
        return { low, low * std::pow (2.0, outside), high * std::pow (2.0, -outside), high };
    }

    /// Sequence mode: a list of bursts to play in order, round and round, each wherever it's put. The guided
    /// calibration plays its arrangements this way.
    struct SequenceBurst
    {
        float pan = 0.5f;      // 0 left, 1 right
        double cutoff = 0.0;   // low cut in Hz: 20 or under is none
        float levelDb = 0.0f;  // under the volume
    };
    static constexpr int maxSequenceLength = 128;

    /// Message thread. It starts from the first burst, and the count of bursts played starts again.
    void setSequence (const std::vector<SequenceBurst>& bursts)
    {
        const int length = std::min ((int) bursts.size(), maxSequenceLength);
        sequenceLength = 0; // nothing plays from a half-written list
        for (int i = 0; i < length; ++i)
        {
            sequencePan[(size_t) i] = bursts[(size_t) i].pan;
            sequenceCutoff[(size_t) i] = bursts[(size_t) i].cutoff;
            sequenceLevelDb[(size_t) i] = bursts[(size_t) i].levelDb;
        }
        ++sequenceGeneration;
        sequenceLength = length;
    }

    /// How many bursts of the sequence set last have started
    int getSequenceBurstsPlayed() const
    {
        const auto progress = sequenceProgress.load();
        return (int) (progress >> 32) == sequenceGeneration.load() ? (int) (progress & 0xffffffff) : 0;
    }

    float getFloorDb() const { return floorDb.load(); }

    /// How positions across are spaced between your left ear and your right, in the grid, Drag mode and the
    /// guided arrangements (on the graph, the pan range is your own). Even is the usual; centre bunches them
    /// towards the middle, where hearing's sharpest, and sides bunches them out towards the ears.
    enum class PanSpacing { even = 0, centre = 1, sides = 2 };

    /// How a position becomes a level in each ear. Power keeps the two ears' power the same all the way across
    /// (the middle is 3 dB down in each). Linear is a straight crossfade, so the middle is 6 dB down and sounds
    /// quieter. Decibels makes every step across the same number of dB between the ears, up to maxPanDb at the
    /// ends, so even hard left is still faintly in the right ear.
    enum class PanLaw { power = 0, linear = 1, decibels = 2 };
    static constexpr float maxPanDb = 30.0f;

    void setPanSpacing (PanSpacing spacing) { panSpacing = juce::jlimit (0, 2, (int) spacing); }
    PanSpacing getPanSpacing() const        { return (PanSpacing) panSpacing.load(); }
    void setPanLaw (PanLaw law)             { panLaw = juce::jlimit (0, 2, (int) law); }
    PanLaw getPanLaw() const                { return (PanLaw) panLaw.load(); }

    /// Where an evenly spaced position (0 left, 1 right) goes with this spacing. The ends and the middle stay put.
    static float spacedPan (float pan, PanSpacing spacing)
    {
        const float across = juce::jlimit (-1.0f, 1.0f, 2.0f * pan - 1.0f);
        const float power = spacing == PanSpacing::centre ? 2.0f : spacing == PanSpacing::sides ? 0.5f : 1.0f;
        return 0.5f * (1.0f + std::copysign (std::pow (std::abs (across), power), across));
    }

    /// The left and right gains for a position (0 left, 1 right)
    static std::pair<float, float> panGains (float pan, PanLaw law)
    {
        pan = juce::jlimit (0.0f, 1.0f, pan);
        if (law == PanLaw::linear)
            return { 1.0f - pan, pan };
        if (law == PanLaw::decibels)
        {
            const float ratio = juce::Decibels::decibelsToGain ((2.0f * pan - 1.0f) * maxPanDb); // right over left
            const float left = 1.0f / std::sqrt (1.0f + ratio * ratio);
            return { left, ratio * left };
        }
        return { std::cos (pan * juce::MathConstants<float>::halfPi), std::sin (pan * juce::MathConstants<float>::halfPi) };
    }

    /// How the low cuts are spaced between 20 Hz and 20 kHz, wherever they're worked out from a height: the
    /// grid's rows, Drag mode's field and the guided arrangements. (On the graph, you put them where you like.)
    /// Octaves is the usual; hertz is each step the same number of Hz, so nearly everything's high; mels, roughly
    /// how pitch is heard, is between the two.
    enum class HeightSpacing { octaves = 0, mels = 1, hertz = 2 };

    static constexpr float defaultPinLow = 100.0f, defaultPinHigh = 10000.0f;

    /// A band pass on top of whatever's playing, in every mode: a sharp low cut and a sharp low-pass (8th-order
    /// Butterworth each, like a burst's own cuts) on all the calibration sounds together. At minHighPass the low
    /// cut is off, and at maxLowPass the low-pass is.
    static constexpr float minHighPass = 20.0f, minLowPass = 20.0f, maxLowPass = 20000.0f;
    void setLowPass (float frequency)  { lowPass = juce::jlimit (minLowPass, maxLowPass, frequency); }
    float getLowPass() const           { return lowPass.load(); }
    bool isLowPassing() const          { return lowPass.load() < maxLowPass - 0.5f; }
    void setHighPass (float frequency) { highPass = juce::jlimit (minHighPass, maxLowPass, frequency); }
    float getHighPass() const          { return highPass.load(); }
    bool isHighPassing() const         { return highPass.load() > minHighPass + 0.05f; }

    /// Where each burst's top is. Fixed: the low pass above, the same for every burst (off, at 20 kHz), with the
    /// burst's height its low cut. Octaves and mels: each burst is a band that wide, centred on its height, so the
    /// whole band moves with it: the lowest ones' low cuts can be under 20 Hz (none at all), and their tops come
    /// down with them; the highest ones' tops can be over 20 kHz. A band so many mels wide is narrow in octaves
    /// up high and wide down low, as pitch is heard.
    enum class TopEdge { fixed = 0, octaves = 1, mels = 2 };
    /// Up to the whole range: 10 octaves, or about 3800 mels, from 20 Hz is 20 kHz
    static constexpr float minBandOctaves = 0.25f, maxBandOctaves = 10.0f, defaultBandOctaves = 2.0f;
    static constexpr float minBandMels = 50.0f, maxBandMels = 3800.0f, defaultBandMels = 500.0f;
    void setTopEdge (TopEdge edge)          { topEdge = juce::jlimit (0, 2, (int) edge); }
    TopEdge getTopEdge() const              { return (TopEdge) topEdge.load(); }
    void setBandOctaves (float octaves)     { bandOctaves = juce::jlimit (minBandOctaves, maxBandOctaves, octaves); }
    float getBandOctaves() const            { return bandOctaves.load(); }
    void setBandMels (float mels)           { bandMels = juce::jlimit (minBandMels, maxBandMels, mels); }
    float getBandMels() const               { return bandMels.load(); }

    /// A burst's low cut and top for its height's frequency: 0 for either means it hasn't one (it's under 20 Hz,
    /// or over 20 kHz). Fixed is the height's frequency as the low cut, and no top of its own.
    ///
    /// As a band, the heights reach further than the bottom and top (`bottom` and `top`, the heights' own range)
    /// by half the band's width each way: the bottom of the range is where the band's top is the bottom
    /// frequency, so the lowest bursts are only their lowest frequencies, and the top of the range is where the
    /// band's low cut is the top frequency. A height between is the same way along, in octaves or mels.
    struct BandEdges { double low = 0.0, top = 0.0; };
    static BandEdges bandEdges (double frequency, TopEdge edge, float octaves, float mels, double bottom = 20.0, double top = 20000.0)
    {
        if (edge == TopEdge::fixed)
            return { frequency > 20.0 ? frequency : 0.0, 0.0 };

        // In octaves or mels, whichever the band's measured in
        const bool inOctaves = edge == TopEdge::octaves;
        const double width = inOctaves ? (double) octaves : (double) mels;
        auto toUnit = [inOctaves] (double f) { return inOctaves ? std::log2 (std::max (1.0e-6, f)) : toMels (std::max (0.0, f)); };
        auto fromUnit = [inOctaves] (double u) { return inOctaves ? std::pow (2.0, u) : fromMels (u); };

        const double low = toUnit (bottom), high = toUnit (std::max (top, bottom * 1.001));
        const double along = juce::jlimit (0.0, 1.0, (toUnit (juce::jlimit (bottom, top, frequency)) - low) / (high - low));
        const double centre = (low - 0.5 * width) + along * (high - low + width);

        BandEdges edges;
        edges.low = fromUnit (centre - 0.5 * width);
        edges.top = fromUnit (centre + 0.5 * width);
        if (edges.low <= 20.0)
            edges.low = 0.0;
        if (edges.top >= 20000.0)
            edges.top = 0.0;
        return edges;
    }

    /// A band of the width set, centred on a frequency (for spots, whose frequencies are where you put them)
    BandEdges bandEdgesAround (double frequency) const
    {
        const auto edge = getTopEdge();
        if (edge == TopEdge::fixed)
            return bandEdges (frequency, edge, 0.0f, 0.0f);
        const double centre = std::max (20.0, frequency);
        BandEdges edges;
        if (edge == TopEdge::octaves)
        {
            edges.low = centre * std::pow (2.0, -0.5 * (double) getBandOctaves());
            edges.top = centre * std::pow (2.0, 0.5 * (double) getBandOctaves());
        }
        else
        {
            edges.low = fromMels (toMels (centre) - 0.5 * (double) getBandMels());
            edges.top = fromMels (toMels (centre) + 0.5 * (double) getBandMels());
        }
        if (edges.low <= 20.0) edges.low = 0.0;
        if (edges.top >= 20000.0) edges.top = 0.0;
        return edges;
    }

    /// Everything about the heights: how they're spaced, and between what. Unpinned, they're between 20 Hz and
    /// 20 kHz with room left at both ends (three rows are a quarter, a half and three quarters of the way up).
    /// Pinned, the bottom is exactly `low` and the top exactly `high`, with the rest spaced between them.
    struct Heights
    {
        HeightSpacing spacing;
        bool pinned;
        float low, high;

        Heights (HeightSpacing spacingToUse = HeightSpacing::octaves) : spacing (spacingToUse), pinned (false), low (defaultPinLow), high (defaultPinHigh) {}
        Heights (HeightSpacing spacingToUse, float lowToUse, float highToUse) : spacing (spacingToUse), pinned (true), low (lowToUse), high (highToUse) {}

        bool operator== (const Heights& other) const { return spacing == other.spacing && pinned == other.pinned && low == other.low && high == other.high; }
        bool operator!= (const Heights& other) const { return ! (*this == other); }

        double bottom() const { return pinned ? (double) low : 20.0; }
        double top() const    { return pinned ? (double) high : 20000.0; }
    };

    void setHeightSpacing (HeightSpacing spacing) { heightSpacing = juce::jlimit (0, 2, (int) spacing); }
    HeightSpacing getHeightSpacing() const        { return (HeightSpacing) heightSpacing.load(); }

    /// Pins the bottom and top heights to these frequencies (the lower of the two is the bottom), or lets them go
    void setHeightPins (bool shouldPin, float low, float high)
    {
        low = juce::jlimit (20.0f, 20000.0f, low);
        high = juce::jlimit (20.0f, 20000.0f, high);
        pinLow = std::min (low, high);
        pinHigh = std::max (low, high);
        heightsPinned = shouldPin;
    }

    Heights getHeights() const
    {
        Heights heights (getHeightSpacing());
        heights.pinned = heightsPinned.load();
        heights.low = pinLow.load();
        heights.high = pinHigh.load();
        return heights;
    }

    static double toMels (double frequency) { return 2595.0 * std::log10 (1.0 + frequency / 700.0); }
    static double fromMels (double mels)    { return 700.0 * (std::pow (10.0, mels / 2595.0) - 1.0); }

    /// The frequency a fraction of the way up from the bottom to the top (20 Hz to 20 kHz unless they're pinned)
    static double frequencyAtHeight (double fraction, const Heights& heights = Heights())
    {
        fraction = juce::jlimit (0.0, 1.0, fraction);
        const double low = heights.bottom(), high = heights.top();
        if (heights.spacing == HeightSpacing::hertz)
            return low + (high - low) * fraction;
        if (heights.spacing == HeightSpacing::mels)
            return fromMels (toMels (low) + (toMels (high) - toMels (low)) * fraction);
        return low * std::pow (high / low, fraction);
    }

    /// And back: how far up from the bottom to the top a frequency is
    static double heightOfFrequency (double frequency, const Heights& heights = Heights())
    {
        const double low = heights.bottom(), high = heights.top();
        if (high <= low)
            return 0.0;
        frequency = juce::jlimit (low, high, frequency);
        if (heights.spacing == HeightSpacing::hertz)
            return (frequency - low) / (high - low);
        if (heights.spacing == HeightSpacing::mels)
            return (toMels (frequency) - toMels (low)) / (toMels (high) - toMels (low));
        return std::log (frequency / low) / std::log (high / low);
    }

    /// The low cut for a height from -1 (the grid's bottom row of three) to 1 (its top row), and anywhere between
    static double cutoffForHeight (float height, const Heights& heights = Heights())
    {
        return frequencyAtHeight (heights.pinned ? (1.0 + (double) height) / 2.0 : (2.0 + (double) height) / 4.0, heights);
    }

    /// Square mode: a square of 2 x 2 or 3 x 3 dots that you put anywhere on a field. Across the field is from
    /// your left ear to your right; up it is the low cut, from none at the bottom to 20 kHz at the top. All of
    /// these are fractions of the field: (0, 0) is its bottom left.
    struct Square
    {
        int size = 3;                    // dots along each side
        float x = 0.5f, y = 0.5f;        // its centre
        float width = 0.5f, height = 0.5f;

        /// Where a dot is on the field, in reading order from the top left
        juce::Point<float> dot (int position) const
        {
            const int row = position / size, column = position % size;
            const float across = size > 1 ? (float) column / (float) (size - 1) - 0.5f : 0.0f;
            const float down = size > 1 ? (float) row / (float) (size - 1) - 0.5f : 0.0f;
            return { x + across * width, y - down * height };
        }

        /// A dot's low cut, in Hz: 20 Hz at the bottom of the field, which is none at all, to 20 kHz at the top
        static double cutoffAt (float fieldY, const Heights& heights = Heights()) { return frequencyAtHeight ((double) fieldY, heights); }
    };
    static constexpr float minSquareSide = 0.08f;

    /// Keeps it a sensible size and wholly on the field
    void setSquare (Square square)
    {
        square.size = square.size <= 2 ? 2 : 3;
        square.width = juce::jlimit (minSquareSide, 1.0f, square.width);
        square.height = juce::jlimit (minSquareSide, 1.0f, square.height);
        square.x = juce::jlimit (square.width * 0.5f, 1.0f - square.width * 0.5f, square.x);
        square.y = juce::jlimit (square.height * 0.5f, 1.0f - square.height * 0.5f, square.y);
        squareSize = square.size;
        squareX = square.x;
        squareY = square.y;
        squareWidth = square.width;
        squareHeight = square.height;
    }
    Square getSquare() const { return { squareSize.load(), squareX.load(), squareY.load(), squareWidth.load(), squareHeight.load() }; }
    static constexpr float minFloorDb = -60.0f, defaultFloorDb = -20.0f; // the quietest time round, below the volume

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
        for (size_t i = 0; i < voices.size(); ++i)
        {
            voices[i].active = false;
            voices[i].pink = PinkNoise { juce::Random ((juce::int64) (1000 + i)) }; // each its own, and the same each time
        }
        samplesUntilNext = 0;
        seqRow = seqColumn = -1;
        depthLevel = depthRepeat = 0;
    }

    void setDepth (int newDepth)        { depth = juce::jlimit (1, maxDepth, newDepth); }
    /// What the depth works on: what plays at each level in turn before the next thing does. A dot, a row
    /// (for spots, a spot across its pan steps), or the whole cycle of every position.
    enum class DepthUnit { dot = 0, row = 1, cycle = 2 };
    void setDepthUnit (DepthUnit unit)  { depthUnit = juce::jlimit (0, 2, (int) unit); }
    DepthUnit getDepthUnit() const      { return (DepthUnit) depthUnit.load(); }

    /// How many times that plays at each level before it goes up one
    static constexpr int maxDepthRepeats = 8;
    void setDepthRepeats (int repeats)  { depthRepeats = juce::jlimit (1, maxDepthRepeats, repeats); }
    /// How far below the volume the first time round is. Each time round after it is a step louder, up to the volume.
    void setFloorDb (float newFloorDb)  { floorDb = juce::jlimit (minFloorDb, 0.0f, newFloorDb); }

    /// What the bursts are made of, as two slopes in dB an octave (positive numbers: 3 is pink), set independently:
    ///
    ///   slope     how fast each burst falls away above its lowest frequency. Pink falls 3. Anything steeper is
    ///             pink through a filter that tilts it further (level with pink at 20 Hz).
    ///   top line  how fast the bursts' bottom edges fall from one burst to the next. Each burst is turned up
    ///             or down, by how many octaves its lowest frequency is above 20 Hz, by whatever puts it on that
    ///             line: the slope less the top line, an octave.
    ///
    /// Both at 3 is plain pink noise, every burst as strong at its bottom edge as the last.
    static constexpr float pinkDbPerOctave = 3.0f, maxSlopeDbPerOctave = 12.0f;
    static constexpr float defaultDbPerOctave = 4.5f; // what both are, unless something sets them

    /// Message thread. A new slope makes new noise here, and the audio picks it up at its next block.
    void setSlope (float dbPerOctave)
    {
        slope = juce::jlimit (pinkDbPerOctave, maxSlopeDbPerOctave, dbPerOctave);
        updateTiltedNoise();
    }
    float getSlope() const { return slope.load(); }

    void setTopLine (float dbPerOctave) { topLine = juce::jlimit (pinkDbPerOctave, maxSlopeDbPerOctave, dbPerOctave); }
    float getTopLine() const            { return topLine.load(); }

    /// How much each burst is turned up for every octave its lowest frequency is above 20 Hz
    float burstBoostDbPerOctave() const { return getSlope() - getTopLine(); }
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
        std::array<juce::uint64, selectionWords> bits {};
        for (int position : positions)
            if (position >= 0 && position < selectionWords * 64)
                bits[(size_t) position / 64] |= (juce::uint64) 1 << (position % 64);
        for (size_t word = 0; word < bits.size(); ++word)
            selected[word] = bits[word];
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
        if (position < 0 || position >= selectionWords * 64)
            return false;
        return ((selected[(size_t) position / 64].load() >> (position % 64)) & 1) != 0;
    }

    /// The high-pass cutoff for a row (0 is the top): the rows' lines cut 20 Hz to 20 kHz into rows + 1 sections.
    /// With the heights pinned, the bottom row is at the bottom and the top row at the top (and one row is halfway).
    static double cutoffForRow (int row, int rows, const Heights& heights = Heights())
    {
        if (heights.pinned)
            return frequencyAtHeight (rows > 1 ? (double) (rows - 1 - row) / (double) (rows - 1) : 0.5, heights);
        const int line = rows - row; // 1 for the bottom row, up to rows for the top
        return frequencyAtHeight ((double) line / (double) (rows + 1), heights);
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
            seqRow = seqColumn = -1;
            depthLevel = depthRepeat = 0;
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
            }
        }
        const std::vector<float>* tilted = isTilted() && ! tiltedNoise.empty() ? &tiltedNoise : nullptr;

        // The band pass, designed again when it's moved; its state carries on, so moving it doesn't click
        const bool lowPassing = isLowPassing(), highPassing = isHighPassing();
        if (lowPassing && lowPass.load() != designedLowPass)
        {
            designedLowPass = lowPass.load();
            const double frequency = std::min ((double) designedLowPass, FilterDesign::maxFrequency (sampleRate) * 0.9);
            for (size_t i = 0; i < lowPassStages[0].size(); ++i)
                for (auto& ear : lowPassStages)
                    ear[i].c = FilterDesign::design (Band::Shape::highCut, frequency, 0.0, butterworthQs[i], sampleRate);
        }
        if (highPassing && highPass.load() != designedHighPass)
        {
            designedHighPass = highPass.load();
            const double frequency = std::min ((double) designedHighPass, FilterDesign::maxFrequency (sampleRate) * 0.9);
            for (size_t i = 0; i < highPassStages[0].size(); ++i)
                for (auto& ear : highPassStages)
                    ear[i].c = FilterDesign::design (Band::Shape::lowCut, frequency, 0.0, butterworthQs[i], sampleRate);
        }

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

                // Each burst has noise of its own, so two that overlap don't add up as one sound (or cancel)
                float noise;
                if (tilted != nullptr)
                {
                    if (voice.noiseIndex >= tilted->size())
                        voice.noiseIndex = 0;
                    noise = (*tilted)[voice.noiseIndex++];
                }
                else
                {
                    noise = voice.pink.next();
                }
                const float sample = voice.bandLimited || voice.topLimited ? voice.filter (noise) : noise;
                l += sample * envelope * voice.leftGain;
                r += sample * envelope * voice.rightGain;
            }

            if (highPassing)
            {
                for (auto& stage : highPassStages[0])
                    l = stage.process (l);
                for (auto& stage : highPassStages[1])
                    r = stage.process (r);
            }
            if (lowPassing)
            {
                for (auto& stage : lowPassStages[0])
                    l = stage.process (l);
                for (auto& stage : lowPassStages[1])
                    r = stage.process (r);
            }

            left[i] += l * gain;
            if (numChannels > 1)
                right[i] += r * gain;
            else
                left[i] += r * gain;
        }
    }

private:
    static constexpr double butterworthQs[] { 0.5098, 0.6013, 0.9000, 2.5629 }; // its four sections, for 8th order

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

    /// Paul Kellet's pink noise, scaled to 0 dB RMS (its raw RMS is about 1.74), so the level is the burst's RMS
    struct PinkNoise
    {
        juce::Random random;
        float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;

        float next() noexcept
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
    };

    /// A burst. Each has its own noise: its own pink generator, or its own place in the looped tilted noise,
    /// somewhere random each time it plays.
    struct Voice
    {
        bool active = false, bandLimited = false;
        int age = 0;
        PinkNoise pink;
        size_t noiseIndex = 0;
        bool topLimited = false;
        std::array<Biquad, 4> topStages; // a sharp low-pass, when it's a band
        float peak = 0, leftGain = 0, rightGain = 0;
        std::array<Biquad, 4> stages; // an 8th-order Butterworth high-pass, for a sharp cut

        float filter (float x) noexcept
        {
            if (bandLimited)
                for (auto& stage : stages)
                    x = stage.process (x);
            if (topLimited)
                for (auto& stage : topStages)
                    x = stage.process (x);
            return x;
        }
    };

    void trigger() noexcept
    {
        const auto modeNow = (Mode) mode.load();
        std::optional<BandEdges> rangeBand; // range mode's bands are their own
        float pan = 0.5f;      // where it is between the ears: 0 left, 1 right
        double cutoff = 0.0;   // its low cut, in Hz: 0 for none
        float levelDb = 0.0f;  // how far under the volume it plays

        if (modeNow == Mode::sequence)
        {
            // The bursts it's been given, in order, round and round
            const int length = sequenceLength.load();
            if (length <= 0)
                return;
            if (const int generation = sequenceGeneration.load(); generation != playingGeneration)
            {
                playingGeneration = generation;
                sequenceIndex = 0;
                sequencePlayed = 0;
            }
            const auto index = (size_t) (sequenceIndex % length);
            pan = juce::jlimit (0.0f, 1.0f, sequencePan[index].load());
            cutoff = sequenceCutoff[index].load();
            if (cutoff <= 20.5)
                cutoff = 0.0;
            levelDb = sequenceLevelDb[index].load();
            currentPosition = (int) index;
            sequenceIndex = ((int) index + 1) % length;
            sequenceProgress = ((juce::int64) playingGeneration << 32) | (juce::int64) (++sequencePlayed);
        }
        else
        {
            const bool spots = modeNow == Mode::spots, range = modeNow == Mode::range;
            const bool panned = spots || range; // across the pan range, in its steps
            const auto square = getSquare();
            const int rows = numRows.load(), columns = numColumns.load();
            const int panPositions = panned ? panSteps.load() : 1;

            // Everything plays a row at a time: a grid's or a square's own rows, and for spots and the range's
            // bands, a row each, with a place for each pan step
            const int rowCount = spots ? spotCount.load() : range ? 3 : modeNow == Mode::square ? square.size : rows;
            const int rowLength = panned ? panPositions : modeNow == Mode::square ? square.size : columns;

            // Only the grid has positions you pick out
            bool anySelected = false;
            for (int p = 0; modeNow == Mode::grid && p < rowCount * rowLength && ! anySelected; ++p)
                anySelected = isSelected (p);

            auto nextInRow = [&] (int row, int after)
            {
                for (int column = after + 1; column < rowLength; ++column)
                    if (! anySelected || isSelected (row * rowLength + column))
                        return column;
                return -1;
            };
            auto nextRowWithSomething = [&] (int after)
            {
                for (int step = 1; step <= rowCount; ++step)
                    if (const int row = (std::max (after, -1) + step) % rowCount; nextInRow (row, -1) >= 0)
                        return row;
                return 0;
            };

            // Starting out, or the row it was on has gone (or has nothing selected in it now)
            const int depthRuns = depth.load(), repeats = depthRepeats.load();
            if (seqRow < 0 || seqRow >= rowCount || nextInRow (seqRow, -1) < 0)
            {
                seqRow = nextRowWithSomething (seqRow < rowCount ? seqRow : -1);
                seqColumn = -1;
                depthLevel = 0;
                depthRepeat = 0;
            }

            if (seqColumn < 0)
            {
                seqColumn = std::max (0, nextInRow (seqRow, -1)); // the very first one
            }
            else
            {
                // What would play next, in reading order
                int nextRow = seqRow, nextColumn = nextInRow (seqRow, seqColumn);
                const bool rowEnded = nextColumn < 0;
                if (rowEnded)
                {
                    nextRow = nextRowWithSomething (seqRow);
                    nextColumn = std::max (0, nextInRow (nextRow, -1));
                }
                const bool cycleEnded = rowEnded && nextRow <= seqRow;

                // The unit the depth works on (a dot, a row, or the whole cycle) has just played. It plays again at
                // this level until it's had its repeats, then a level louder, and after the loudest, it moves on.
                const auto unit = (DepthUnit) depthUnit.load();
                if (unit == DepthUnit::dot || (unit == DepthUnit::row && rowEnded) || (unit == DepthUnit::cycle && cycleEnded))
                {
                    bool movesOn = false;
                    if (++depthRepeat >= repeats)
                    {
                        depthRepeat = 0;
                        if (++depthLevel >= depthRuns)
                        {
                            depthLevel = 0;
                            movesOn = true;
                        }
                    }

                    if (! movesOn)
                    {
                        // Back to the start of the unit
                        if (unit == DepthUnit::dot && (! anySelected || isSelected (seqRow * rowLength + seqColumn)))
                        {
                            nextRow = seqRow;
                            nextColumn = seqColumn;
                        }
                        else if (unit == DepthUnit::row)
                        {
                            nextRow = seqRow;
                            nextColumn = std::max (0, nextInRow (seqRow, -1));
                        }
                        else if (unit == DepthUnit::cycle)
                        {
                            nextRow = nextRowWithSomething (-1);
                            nextColumn = std::max (0, nextInRow (nextRow, -1));
                        }
                    }
                }
                seqRow = nextRow;
                seqColumn = nextColumn;
            }
            depthLevel = std::min (depthLevel, depthRuns - 1); // if the depth's come down

            const int position = panned ? seqRow : seqRow * rowLength + seqColumn;
            const int repeat = seqColumn; // for a spot or a band, which pan position it's at
            currentPosition = position;

            // Where it is between the ears (0 = left, 1 = right), and where its low cut is
            if (panned)
            {
                // Left to right across the range
                const float low = panLow.load(), high = panHigh.load();
                const int panIndex = repeat;
                const float spread = panPositions > 1 ? low + (high - low) * (float) panIndex / (float) (panPositions - 1)
                                                      : 0.5f * (low + high);
                pan = 0.5f * (spread + 1.0f);
                if (range)
                {
                    // Top, middle, bottom: row 0 is the top band
                    const auto edges = rangeEdges (rangeLow.load(), rangeHigh.load(), rangeMiddle.load());
                    const size_t below = (size_t) (2 - juce::jlimit (0, 2, position));
                    rangeBand = BandEdges { edges[below] > 20.0 ? edges[below] : 0.0, edges[below + 1] < 20000.0 ? edges[below + 1] : 0.0 };
                }
                else
                {
                    cutoff = spotFrequency[(size_t) position].load();
                    if (cutoff <= 20.0)
                        cutoff = 0.0;
                }
            }
            else if (modeNow == Mode::square)
            {
                const auto dot = square.dot (position);
                pan = juce::jlimit (0.0f, 1.0f, dot.x);
                cutoff = Square::cutoffAt (dot.y, getHeights());
                if (cutoff <= 20.5)
                    cutoff = 0.0;
            }
            else
            {
                const int row = position / columns, column = position % columns;
                pan = columns > 1 ? (float) column / (float) (columns - 1) : 0.5f;
                cutoff = cutoffForRow (row, rows, getHeights());
            }

            // Quietest first: each level is a step up, from the floor to full level
            if (depthRuns > 1)
                levelDb = floorDb.load() * (float) (depthRuns - 1 - depthLevel) / (float) (depthRuns - 1);
        }

        // Take a free voice, or the quietest one
        Voice* voice = &voices[0];
        for (auto& candidate : voices)
        {
            if (! candidate.active) { voice = &candidate; break; }
            if (candidate.peak < voice->peak) voice = &candidate;
        }

        // Across: spaced as asked (except on the graph, where the range is your own), then into each ear by the pan law
        if (modeNow != Mode::spots && modeNow != Mode::range)
            pan = spacedPan (pan, (PanSpacing) panSpacing.load());
        std::tie (voice->leftGain, voice->rightGain) = panGains (pan, (PanLaw) panLaw.load());

        // Pink noise has the same power in every octave, so cutting some octaves off makes it
        // quieter; make up for that so every row (or spot) is as loud.
        // As a band, its height is the band's middle: its low cut and its top are either side
        const auto heightsNow = getHeights();
        const auto edges = rangeBand.has_value()    ? *rangeBand
                         : modeNow == Mode::spots ? bandEdgesAround (cutoff) // a spot's frequency is your own: the band's just around it
                                                  : bandEdges (cutoff, getTopEdge(), getBandOctaves(), getBandMels(), heightsNow.bottom(), heightsNow.top());
        cutoff = std::min (edges.low, FilterDesign::maxFrequency (sampleRate) * 0.9);
        voice->bandLimited = cutoff > 0.0;
        voice->peak = 1.0f;

        // Its top, if it's a band: a sharp low-pass
        double top = edges.top;
        if (top > FilterDesign::maxFrequency (sampleRate) * 0.9)
            top = 0.0;
        voice->topLimited = top > 0.0;
        if (voice->topLimited)
            for (size_t i = 0; i < voice->topStages.size(); ++i)
                voice->topStages[i] = Biquad { FilterDesign::design (Band::Shape::highCut, top, 0.0, butterworthQs[i], sampleRate) };

        if (voice->bandLimited || voice->topLimited)
        {
            const double octavesLeft = std::log2 ((voice->topLimited ? top : 20000.0) / std::max (20.0, cutoff)), octavesAll = std::log2 (20000.0 / 20.0);
            voice->peak = (float) std::sqrt (octavesAll / std::max (0.5, octavesLeft));

            // Butterworth: four biquads with these Qs make a flat 8th-order high-pass
            const auto& qs = butterworthQs;
            if (voice->bandLimited)
                for (size_t i = 0; i < voice->stages.size(); ++i)
                    voice->stages[i] = Biquad { FilterDesign::design (Band::Shape::lowCut, cutoff, 0.0, qs[i], sampleRate) };
        }

        // Up (or down) for every octave this burst's lowest frequency is above 20 Hz, to put it on the top line
        voice->peak *= juce::Decibels::decibelsToGain (burstBoostDbPerOctave() * (float) std::log2 (std::max (20.0, cutoff) / 20.0));

        voice->peak *= juce::Decibels::decibelsToGain (levelDb);

        voice->noiseIndex = (size_t) noiseStarts.nextInt (1 << noiseOrder);
        voice->age = 0;
        voice->active = true;
    }

    /// How far the noise wanted now is tilted beyond pink, in dB an octave
    float wantedTilt() const { return getSlope() - pinkDbPerOctave; }
    bool isTilted() const    { return wantedTilt() > 0.001f; } // otherwise it's plain pink, made as it goes

    /// Makes the noise for a new tilt, if it's changed, and leaves it for the audio thread to pick up
    void updateTiltedNoise()
    {
        const juce::ScopedLock build (buildLock);
        const float tilt = wantedTilt();
        if (! isTilted() || pinkSpectrum.empty() || std::abs (tilt - builtTilt) < 1.0e-4f)
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

    /// Pink noise for making the tilted noise from (the bursts have their own)
    float nextPink() noexcept { return pinkSource.next(); }

    double sampleRate = 48000.0;

    std::atomic<int> numRows { defaultRows }, numColumns { defaultColumns }, depth { 1 }, depthRepeats { 1 }, depthUnit { (int) DepthUnit::row };
    // Which positions are picked out: a bit each, in words the audio thread can read without waiting
    static constexpr int selectionWords = (maxRows * maxColumns + 63) / 64;
    std::array<std::atomic<juce::uint64>, (size_t) selectionWords> selected {};
    std::atomic<float> level { juce::Decibels::decibelsToGain (-20.0f) };
    std::atomic<float> rate { defaultRate };
    std::atomic<float> releaseMs { defaultReleaseMs }, attackMs { defaultAttackMs };
    std::atomic<float> floorDb { defaultFloorDb };
    std::atomic<float> slope { defaultDbPerOctave }, topLine { defaultDbPerOctave };

    // The tilted noise the audio thread plays, and the next one, made on the message thread
    std::vector<float> tiltedNoise;
    juce::CriticalSection buildLock; // makers only: never the audio thread
    std::vector<float> pinkSpectrum, octavesAbove20;
    float builtTilt = 0.0f;
    juce::SpinLock pendingLock;
    std::vector<float> pendingNoise;
    std::atomic<bool> hasPending { false };
    std::atomic<int> mode { (int) Mode::spots }, spotCount { 3 }, panSteps { 1 }, heightSpacing { (int) HeightSpacing::octaves };
    std::atomic<int> panSpacing { (int) PanSpacing::even }, panLaw { (int) PanLaw::power };
    std::atomic<bool> heightsPinned { false };
    std::atomic<float> pinLow { defaultPinLow }, pinHigh { defaultPinHigh };
    std::array<std::atomic<float>, maxSpots> spotFrequency { 200.0f, 1000.0f, 5000.0f, 12000.0f };
    std::atomic<float> panLow { 0.0f }, panHigh { 0.0f };
    std::array<std::atomic<float>, maxSequenceLength> sequencePan {}, sequenceLevelDb {};
    std::array<std::atomic<double>, maxSequenceLength> sequenceCutoff {};
    std::atomic<int> sequenceLength { 0 }, sequenceGeneration { 0 };
    std::atomic<juce::int64> sequenceProgress { 0 }; // which sequence (its generation), and how many of its bursts have started
    int playingGeneration = 0, sequenceIndex = 0, sequencePlayed = 0; // the audio thread's
    std::atomic<int> squareSize { 3 };
    std::atomic<float> squareX { 0.5f }, squareY { 0.5f }, squareWidth { 0.5f }, squareHeight { 0.5f };
    std::atomic<bool> playing { false };
    std::atomic<int> currentPosition { -1 };

    // Audio thread
    std::array<Voice, 64> voices {}; // enough for the fastest bursts to ring for a second each before the quietest is reused
    int samplesUntilNext = 0;
    int seqRow = -1, seqColumn = -1; // where it's playing: the row, and the place along it
    int depthLevel = 0;  // which level the row's being played at: 0 is the quietest
    int depthRepeat = 0; // how many times it's been played at that level
    PinkNoise pinkSource { juce::Random (42) };
    std::atomic<float> rangeLow { defaultRangeLow }, rangeHigh { defaultRangeHigh }, rangeMiddle { defaultRangeMiddle };
    std::atomic<float> highPass { minHighPass };
    float designedHighPass = 0.0f;              // audio thread
    std::array<std::array<Biquad, 4>, 2> highPassStages {};
    std::atomic<float> lowPass { maxLowPass }, bandOctaves { defaultBandOctaves }, bandMels { defaultBandMels };
    std::atomic<int> topEdge { (int) TopEdge::fixed };
    float designedLowPass = 0.0f;                // audio thread
    std::array<std::array<Biquad, 4>, 2> lowPassStages {}; // left and right
    juce::Random noiseStarts { 7 }; // where in the tilted noise each burst starts: audio thread
};
