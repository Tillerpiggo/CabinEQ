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
        return (int) state.getProperty (idMode, 0) == 1 ? CalibrationPlayer::Mode::spots : CalibrationPlayer::Mode::grid;
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
