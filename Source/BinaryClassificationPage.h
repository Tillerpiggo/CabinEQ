/*
  ==============================================================================

    BinaryClassificationPage.h
    Created: 9 Jul 2024 4:25:24pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CurveComponent.h"
#include "SliderGroup.h"
#include "ClearSoundLookAndFeel.h"
#include "CircleComponent.h"
#include "CircularButton.h"


class BinaryClassificationPage : public juce::Component,
                                 public juce::Button::Listener,
                                 public juce::Slider::Listener,
                                 public juce::Timer
{
public:
    BinaryClassificationPage(StartupMVPAudioProcessor& p);
    ~BinaryClassificationPage() override;

    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;

private:
    void addComponents();
    void addListeners();
    void removeListeners();

    void timerCallback() override;

    StartupMVPAudioProcessor& audioProcessor;
    CurveComponent curveComponent;
    CircularButton lowerPreferredButton { "Lower Preferred" };
    CircularButton higherPreferredButton { "Higher Preferred" };
    juce::ToggleButton toggleCalibrationButton { "Toggle Calibration" };
    juce::TextButton bypassButton { "Bypass" };
    juce::TextButton applyCurveButton { "Apply Curve" };
    juce::Slider referenceSlider;
};
