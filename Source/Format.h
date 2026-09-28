/*
  ==============================================================================

    Format.h

    How frequencies, gains and Q are shown and typed in.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Format
{
    inline juce::String frequency (double hz)
    {
        if (hz >= 10000.0) return juce::String (hz / 1000.0, 1) + " kHz";
        if (hz >= 1000.0)  return juce::String (hz / 1000.0, 2) + " kHz";
        if (hz >= 100.0)   return juce::String (juce::roundToInt (hz)) + " Hz";
        return juce::String (hz, 1) + " Hz";
    }

    inline juce::String frequencyShort (double hz)
    {
        if (hz >= 1000.0)
            return juce::String (hz / 1000.0, hz >= 10000.0 || std::fmod (hz, 1000.0) == 0.0 ? 0 : 1) + "k";
        return juce::String (juce::roundToInt (hz));
    }

    inline juce::String gain (double db)
    {
        if (std::abs (db) < 0.05)
            return "0.0 dB";
        return (db > 0 ? "+" : "") + juce::String (db, 1) + " dB";
    }

    inline juce::String q (double value)
    {
        return juce::String (value, value < 10.0 ? 2 : 1);
    }

    /// Reads "1.2k", "1200", "1.2 kHz" and so on.
    inline std::optional<double> parseFrequency (const juce::String& text)
    {
        auto trimmed = text.trim().toLowerCase();
        if (! trimmed.containsAnyOf ("0123456789"))
            return std::nullopt;
        double value = trimmed.getDoubleValue();
        if (trimmed.containsChar ('k'))
            value *= 1000.0;
        return value;
    }
}
