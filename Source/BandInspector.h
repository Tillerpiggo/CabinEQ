/*
  ==============================================================================

    BandInspector.h

    The strip under the graph for reading and typing in the focused band's exact
    values: shape, frequency, gain, Q, which ears, on or off.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqAudioProcessor.h"
#include "ValueField.h"

class BandInspector : public juce::Component
{
public:
    explicit BandInspector (CabinEqAudioProcessor& processor);

    /// Shows this band, or a hint if it's -1. numSelected is shown when it's more than one.
    void showBand (int bandId, int bandNumber, int numSelected);

    std::function<void()> onEdited;
    std::function<void()> onDeleteClicked;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    std::optional<Band> currentBand() const;
    void edit (const juce::String& name, std::function<void (Band&)> change);
    void updateControls();

    CabinEqAudioProcessor& processor;
    int bandId = -1, bandNumber = 0, numSelected = 0;

    juce::ComboBox shapeBox, channelBox;
    ValueField frequencyField { "Freq", Band::minFreq, Band::maxFreq, 1000.0, ValueField::Scale::logarithmic };
    ValueField gainField { "Gain", Band::minGain, Band::maxGain, 0.0 };
    ValueField qField { "Q", Band::minQ, Band::maxQ, Band::defaultQ, ValueField::Scale::logarithmic };
    juce::ToggleButton enabledToggle { "On" };
    juce::TextButton deleteButton { "Delete" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandInspector)
};
