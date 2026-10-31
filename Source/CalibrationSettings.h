/*
  ==============================================================================

    CalibrationSettings.h

    The calibration spots (for playing on the EQ graph rather than the grid), kept
    as properties of the plugin's state so the graph and the calibration panel can
    both change them. Not undoable: they're how you listen, not part of the EQ.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CalibrationPlayer.h"

namespace CalibrationSettings
{
    inline const juce::Identifier idMode { "calibrationMode" };
    inline const juce::Identifier idSpotCount { "calibrationSpots" };
    inline const juce::Identifier idPanLow { "calibrationPanLow" };
    inline const juce::Identifier idPanHigh { "calibrationPanHigh" };
    inline const juce::Identifier idPanSteps { "calibrationPanSteps" };
    inline const juce::Identifier idHeightSpacing { "calibrationHeightSpacing" };
    inline const juce::Identifier idRangeLow { "calibrationRangeLow" }, idRangeHigh { "calibrationRangeHigh" }, idRangeMiddle { "calibrationRangeMiddle" };
    inline const juce::Identifier idPanSpacing { "calibrationPanSpacing" }, idPanLaw { "calibrationPanLaw" };
    inline const juce::Identifier idHeightsPinned { "calibrationHeightsPinned" }, idHeightLow { "calibrationHeightLow" }, idHeightHigh { "calibrationHeightHigh" };
    inline const juce::Identifier idSquareSize { "calibrationSquareSize" }, idSquareX { "calibrationSquareX" }, idSquareY { "calibrationSquareY" },
                                  idSquareWidth { "calibrationSquareWidth" }, idSquareHeight { "calibrationSquareHeight" };
    inline const juce::Identifier idLineX1 { "calibrationLineX1" }, idLineY1 { "calibrationLineY1" }, idLineX2 { "calibrationLineX2" }, idLineY2 { "calibrationLineY2" },
                                  idLineSweep { "calibrationLineSweep" };

    constexpr float minFrequency = 20.0f, maxFrequency = 16000.0f;

    inline juce::Identifier frequencyId (int index) { return "calibrationSpot" + juce::String (index) + "Frequency"; }
    inline juce::String spotName (int index) { return juce::String::charToString ((juce::juce_wchar) ('A' + index)); }

    inline juce::Colour spotColour (int index)
    {
        static const juce::Colour colours[] { juce::Colour (0xff4fd1c5), juce::Colour (0xfff28bd6), juce::Colour (0xffffb454), juce::Colour (0xff5ab0ff) };
        return colours[(size_t) juce::jlimit (0, CalibrationPlayer::maxSpots - 1, index)];
    }

    /// "On graph" unless you've picked the grid
    inline CalibrationPlayer::Mode getMode (const juce::ValueTree& state)
    {
        return (CalibrationPlayer::Mode) juce::jlimit (0, 5, (int) state.getProperty (idMode, 0));
    }

    inline int getSpotCount (const juce::ValueTree& state)
    {
        return juce::jlimit (2, CalibrationPlayer::maxSpots, (int) state.getProperty (idSpotCount, 3));
    }

    inline float getSpotFrequency (const juce::ValueTree& state, int index)
    {
        static const float defaults[] { 200.0f, 1000.0f, 5000.0f, 12000.0f };
        return juce::jlimit (minFrequency, maxFrequency,
                             (float) state.getProperty (frequencyId (index), defaults[(size_t) juce::jlimit (0, 3, index)]));
    }

    /// The pan range, -1 (left) to 1 (right). Older versions kept a pan for each spot; start from the first one's.
    inline juce::Range<float> getPanRange (const juce::ValueTree& state)
    {
        const float old = state.getProperty ("calibrationSpot0Pan", 0.0f);
        const float low = state.getProperty (idPanLow, old), high = state.getProperty (idPanHigh, old);
        return { juce::jlimit (-1.0f, 1.0f, std::min (low, high)), juce::jlimit (-1.0f, 1.0f, std::max (low, high)) };
    }

    inline int getPanSteps (const juce::ValueTree& state)
    {
        return juce::jlimit (1, CalibrationPlayer::maxPanSteps, (int) state.getProperty (idPanSteps, 1));
    }

    /// How the low cuts are spaced up the grid, Drag mode's field and the guided arrangements: octaves unless you've picked another
    inline CalibrationPlayer::HeightSpacing getHeightSpacing (const juce::ValueTree& state)
    {
        return (CalibrationPlayer::HeightSpacing) juce::jlimit (0, 2, (int) state.getProperty (idHeightSpacing, 0));
    }

    /// The square you drag about, in square mode
    inline CalibrationPlayer::Square getSquare (const juce::ValueTree& state)
    {
        CalibrationPlayer::Square square;
        square.size = state.getProperty (idSquareSize, square.size);
        square.x = state.getProperty (idSquareX, square.x);
        square.y = state.getProperty (idSquareY, square.y);
        square.width = state.getProperty (idSquareWidth, square.width);
        square.height = state.getProperty (idSquareHeight, square.height);
        return square;
    }

    /// Line mode's line
    inline CalibrationPlayer::Line getLine (const juce::ValueTree& state)
    {
        CalibrationPlayer::Line line;
        line.x1 = state.getProperty (idLineX1, line.x1);
        line.y1 = state.getProperty (idLineY1, line.y1);
        line.x2 = state.getProperty (idLineX2, line.x2);
        line.y2 = state.getProperty (idLineY2, line.y2);
        return line;
    }

    /// Saves the line as the player has it, kept on the field
    inline void setLine (juce::ValueTree state, CalibrationPlayer& player, CalibrationPlayer::Line line)
    {
        player.setLine (line);
        line = player.getLine();
        state.setProperty (idLineX1, line.x1, nullptr);
        state.setProperty (idLineY1, line.y1, nullptr);
        state.setProperty (idLineX2, line.x2, nullptr);
        state.setProperty (idLineY2, line.y2, nullptr);
    }

    inline void setLineSweep (juce::ValueTree state, CalibrationPlayer& player, float seconds)
    {
        player.setLineSweep (seconds);
        state.setProperty (idLineSweep, player.getLineSweep(), nullptr);
    }

    /// Hands everything in the state to the player
    inline void apply (const juce::ValueTree& state, CalibrationPlayer& player)
    {
        player.setMode (getMode (state));
        player.setSpotCount (getSpotCount (state));
        for (int i = 0; i < CalibrationPlayer::maxSpots; ++i)
            player.setSpot (i, getSpotFrequency (state, i));
        const auto pan = getPanRange (state);
        player.setPanRange (pan.getStart(), pan.getEnd());
        player.setPanSteps (getPanSteps (state));
        player.setSquare (getSquare (state));
        player.setLine (getLine (state));
        player.setLineSweep (state.getProperty (idLineSweep, CalibrationPlayer::defaultLineSweep));
        player.setHeightSpacing (getHeightSpacing (state));
        player.setRange (state.getProperty (idRangeLow, CalibrationPlayer::defaultRangeLow), state.getProperty (idRangeHigh, CalibrationPlayer::defaultRangeHigh),
                         state.getProperty (idRangeMiddle, CalibrationPlayer::defaultRangeMiddle));
        player.setPanSpacing ((CalibrationPlayer::PanSpacing) juce::jlimit (0, 2, (int) state.getProperty (idPanSpacing, 0)));
        player.setPanLaw ((CalibrationPlayer::PanLaw) juce::jlimit (0, 2, (int) state.getProperty (idPanLaw, 0)));
        player.setHeightPins (state.getProperty (idHeightsPinned, false), state.getProperty (idHeightLow, CalibrationPlayer::defaultPinLow),
                              state.getProperty (idHeightHigh, CalibrationPlayer::defaultPinHigh));
    }

    /// Range mode's range, and the middle's share of it, saved as the player has them
    inline void setRange (juce::ValueTree state, CalibrationPlayer& player, float low, float high, float middle)
    {
        player.setRange (low, high, middle);
        state.setProperty (idRangeLow, player.getRangeLow(), nullptr);
        state.setProperty (idRangeHigh, player.getRangeHigh(), nullptr);
        state.setProperty (idRangeMiddle, player.getRangeMiddle(), nullptr);
    }

    inline void setPanSpacing (juce::ValueTree state, CalibrationPlayer& player, CalibrationPlayer::PanSpacing spacing)
    {
        state.setProperty (idPanSpacing, (int) spacing, nullptr);
        player.setPanSpacing (spacing);
    }

    inline void setPanLaw (juce::ValueTree state, CalibrationPlayer& player, CalibrationPlayer::PanLaw law)
    {
        state.setProperty (idPanLaw, (int) law, nullptr);
        player.setPanLaw (law);
    }

    /// Pins the bottom and top heights to these frequencies, or lets them go, and saves it as the player has it
    inline void setHeightPins (juce::ValueTree state, CalibrationPlayer& player, bool shouldPin, float low, float high)
    {
        player.setHeightPins (shouldPin, low, high);
        const auto heights = player.getHeights();
        state.setProperty (idHeightsPinned, heights.pinned, nullptr);
        state.setProperty (idHeightLow, heights.low, nullptr);
        state.setProperty (idHeightHigh, heights.high, nullptr);
    }

    inline void setHeightSpacing (juce::ValueTree state, CalibrationPlayer& player, CalibrationPlayer::HeightSpacing spacing)
    {
        state.setProperty (idHeightSpacing, (int) spacing, nullptr);
        player.setHeightSpacing (spacing);
    }

    /// Saves the square as the player has it, once it's kept it on the field
    inline void setSquare (juce::ValueTree state, CalibrationPlayer& player, CalibrationPlayer::Square square)
    {
        player.setSquare (square);
        square = player.getSquare();
        state.setProperty (idSquareSize, square.size, nullptr);
        state.setProperty (idSquareX, square.x, nullptr);
        state.setProperty (idSquareY, square.y, nullptr);
        state.setProperty (idSquareWidth, square.width, nullptr);
        state.setProperty (idSquareHeight, square.height, nullptr);
    }

    inline void setSpotFrequency (juce::ValueTree state, CalibrationPlayer& player, int index, float frequency)
    {
        frequency = juce::jlimit (minFrequency, maxFrequency, frequency);
        state.setProperty (frequencyId (index), frequency, nullptr);
        player.setSpot (index, frequency);
    }

    inline void setMode (juce::ValueTree state, CalibrationPlayer& player, CalibrationPlayer::Mode mode)
    {
        state.setProperty (idMode, (int) mode, nullptr);
        apply (state, player);
    }

    /// Changing how many spots spreads the new set evenly (in octaves) between the lowest and highest
    inline void setSpotCount (juce::ValueTree state, CalibrationPlayer& player, int count)
    {
        count = juce::jlimit (2, CalibrationPlayer::maxSpots, count);
        const int oldCount = getSpotCount (state);
        float low = maxFrequency, high = minFrequency;
        for (int i = 0; i < oldCount; ++i)
        {
            low = std::min (low, getSpotFrequency (state, i));
            high = std::max (high, getSpotFrequency (state, i));
        }
        if (high <= low * 1.01f)
            high = std::min (maxFrequency, low * 25.0f);

        state.setProperty (idSpotCount, count, nullptr);
        for (int i = 0; i < count; ++i)
            setSpotFrequency (state, player, i, low * std::pow (high / low, (float) i / (float) (count - 1)));
        apply (state, player);
    }

    inline void setPanRange (juce::ValueTree state, CalibrationPlayer& player, float low, float high)
    {
        state.setProperty (idPanLow, juce::jlimit (-1.0f, 1.0f, low), nullptr);
        state.setProperty (idPanHigh, juce::jlimit (-1.0f, 1.0f, high), nullptr);
        player.setPanRange (low, high);
    }

    inline void setPanSteps (juce::ValueTree state, CalibrationPlayer& player, int steps)
    {
        state.setProperty (idPanSteps, steps, nullptr);
        player.setPanSteps (steps);
    }

    inline juce::String describePan (float pan)
    {
        const int amount = juce::roundToInt (std::abs (pan) * 100.0f);
        if (amount == 0)
            return "Centre";
        return (pan < 0 ? "L " : "R ") + juce::String (amount);
    }

    inline juce::String describePanRange (juce::Range<float> range)
    {
        if (range.getLength() < 0.005f)
            return describePan (range.getStart());
        return describePan (range.getStart()) + " to " + describePan (range.getEnd());
    }
}
