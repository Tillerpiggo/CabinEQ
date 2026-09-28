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

    struct Spot
    {
        float frequency = 1000.0f; // its low cut
        float pan = 0.0f;          // -1 left, 1 right
    };

    inline juce::Identifier frequencyId (int index) { return "calibrationSpot" + juce::String (index) + "Frequency"; }
    inline juce::Identifier panId (int index)       { return "calibrationSpot" + juce::String (index) + "Pan"; }

    inline juce::String spotName (int index) { return juce::String::charToString ((juce::juce_wchar) ('A' + index)); }

    inline juce::Colour spotColour (int index)
    {
        static const juce::Colour colours[] { juce::Colour (0xff4fd1c5), juce::Colour (0xfff28bd6), juce::Colour (0xffffb454) };
        return colours[(size_t) juce::jlimit (0, CalibrationPlayer::maxSpots - 1, index)];
    }

    inline CalibrationPlayer::Mode getMode (const juce::ValueTree& state)
    {
        return (int) state.getProperty (idMode, 0) == 1 ? CalibrationPlayer::Mode::spots : CalibrationPlayer::Mode::grid;
    }

    inline int getSpotCount (const juce::ValueTree& state)
    {
        return juce::jlimit (2, CalibrationPlayer::maxSpots, (int) state.getProperty (idSpotCount, 3));
    }

    inline Spot getSpot (const juce::ValueTree& state, int index)
    {
        static const float defaults[] { 200.0f, 1000.0f, 5000.0f };
        return { juce::jlimit (20.0f, 16000.0f, (float) state.getProperty (frequencyId (index), defaults[(size_t) juce::jlimit (0, 2, index)])),
                 juce::jlimit (-1.0f, 1.0f, (float) state.getProperty (panId (index), 0.0f)) };
    }

    /// Hands the mode and spots in the state to the player
    inline void apply (const juce::ValueTree& state, CalibrationPlayer& player)
    {
        player.setMode (getMode (state));
        player.setSpotCount (getSpotCount (state));
        for (int i = 0; i < CalibrationPlayer::maxSpots; ++i)
        {
            const auto spot = getSpot (state, i);
            player.setSpot (i, spot.frequency, spot.pan);
        }
    }

    inline void setSpot (juce::ValueTree state, CalibrationPlayer& player, int index, Spot spot)
    {
        spot.frequency = juce::jlimit (20.0f, 16000.0f, spot.frequency);
        spot.pan = juce::jlimit (-1.0f, 1.0f, spot.pan);
        state.setProperty (frequencyId (index), spot.frequency, nullptr);
        state.setProperty (panId (index), spot.pan, nullptr);
        player.setSpot (index, spot.frequency, spot.pan);
    }

    inline void setMode (juce::ValueTree state, CalibrationPlayer& player, CalibrationPlayer::Mode mode)
    {
        state.setProperty (idMode, (int) mode, nullptr);
        apply (state, player);
    }

    inline void setSpotCount (juce::ValueTree state, CalibrationPlayer& player, int count)
    {
        state.setProperty (idSpotCount, juce::jlimit (2, CalibrationPlayer::maxSpots, count), nullptr);
        apply (state, player);
    }

    inline juce::String describePan (float pan)
    {
        const int amount = juce::roundToInt (std::abs (pan) * 100.0f);
        if (amount == 0)
            return "Centre";
        return (pan < 0 ? "L " : "R ") + juce::String (amount);
    }
}
