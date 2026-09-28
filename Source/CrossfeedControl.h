/*
  ==============================================================================

    CrossfeedControl.h
    Created: 18 May 2025 2:56:11am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqAudioProcessor.h"

/// Crossfeed's settings, shown in a pop-over from the top bar.
class CrossfeedControl  : public juce::Component
{
public:
    explicit CrossfeedControl (juce::AudioProcessorValueTreeState& parameters);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void updateEnablement();

    juce::ToggleButton enableButton { "Crossfeed" };
    juce::Slider levelSlider, delaySlider;
    juce::Label levelLabel, delayLabel;

    juce::AudioProcessorValueTreeState::ButtonAttachment enableAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment levelAttachment, delayAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrossfeedControl)
};
