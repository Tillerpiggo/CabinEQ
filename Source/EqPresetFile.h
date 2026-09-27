/*
  ==============================================================================

    EqPresetFile.h

    Reads and writes parametric EQ settings as text in the Equalizer APO format,
    which is what AutoEQ's ParametricEQ.txt files and most headphone EQ databases use:

        Preamp: -6.2 dB
        Filter 1: ON LSC Fc 105 Hz Gain 5.8 dB Q 0.70
        Filter 2: ON PK Fc 2310 Hz Gain -2.3 dB Q 1.93

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandProfile.h"

namespace EqPresetFile
{
    /// Returns nothing if the text has no filters CabinEQ understands.
    std::optional<BandProfile> parse (const juce::String& text);
    juce::String write (const BandProfile& bandProfile);

    inline const juce::String fileExtensions { "*.txt;*.cfg" };
}
