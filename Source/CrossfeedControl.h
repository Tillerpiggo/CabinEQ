/*
  ==============================================================================

    CrossfeedControl.h
    Created: 18 May 2025 2:56:11am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Layout.h"
#include "Listeners.h"

class CrossfeedControl  : public juce::Component
{
public:
    CrossfeedControl();
    ~CrossfeedControl() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setListener (CalibrationListener* listener);

private:
    void updateEnableButtonText();

    CalibrationListener* calibrationListener = nullptr;

    juce::Slider delaySlider;
    juce::Label  delayLabel;

    juce::Slider volumeSlider;
    juce::Label  volumeLabel;

    juce::TextButton enableButton;
    bool isCrossfeedEnabled = false;
};
