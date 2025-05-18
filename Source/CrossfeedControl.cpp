/*
  ==============================================================================

    CrossfeedControl.cpp
    Created: 18 May 2025 2:56:11am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CrossfeedControl.h"

CrossfeedControl::CrossfeedControl()
{
    addAndMakeVisible (delayLabel);
    delayLabel.setText ("Delay (samples)", juce::dontSendNotification);
    delayLabel.setJustificationType (juce::Justification::centredLeft);

    addAndMakeVisible (delaySlider);
    delaySlider.setRange (0, 2000, 1); // 0 to 2000 samples, integer steps
    delaySlider.setValue (0);
    delaySlider.setSliderStyle (juce::Slider::LinearHorizontal);
    delaySlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 80, 20);
    delaySlider.onValueChange = [this]
    {
        if (calibrationListener != nullptr)
            calibrationListener->setCrossfeedDelaySamples (static_cast<int>(delaySlider.getValue()));
    };

    addAndMakeVisible (volumeLabel);
    volumeLabel.setText ("Volume (0-1)", juce::dontSendNotification);
    volumeLabel.setJustificationType (juce::Justification::centredLeft);

    addAndMakeVisible (volumeSlider);
    volumeSlider.setRange (0.0, 1.0, 0.01); // 0.0 to 1.0, 0.01 steps
    volumeSlider.setValue (0.0);
    volumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 80, 20);
    volumeSlider.onValueChange = [this]
    {
        if (calibrationListener != nullptr)
            calibrationListener->setCrossfeedVolume (static_cast<float>(volumeSlider.getValue()));
    };
    
    addAndMakeVisible (enableButton);
    updateEnableButtonText();
    enableButton.onClick = [this]
    {
        isCrossfeedEnabled = !isCrossfeedEnabled;
        if (calibrationListener != nullptr)
            calibrationListener->setCrossfeedEnabled (isCrossfeedEnabled);
        updateEnableButtonText();
    };
}

CrossfeedControl::~CrossfeedControl()
{
}

void CrossfeedControl::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId).darker(0.1f));
}

void CrossfeedControl::resized()
{
    auto bounds = getLocalBounds().reduced (10);
    Layout layout (bounds, 8.0f); // 8.0f padding between rows

    // Row for Delay Slider and Label
    layout.addRow ({ Space(&delayLabel, 100.0f), Space(&delaySlider) }, 30.0f);
    layout.addRow ( {Space()}, 10.0f); // Spacer row
    
    // Row for Volume Slider and Label
    layout.addRow ({ Space(&volumeLabel, 100.0f), Space(&volumeSlider) }, 30.0f);
    layout.addRow ( {Space()}, 10.0f); // Spacer row

    // Row for Enable Button
    layout.addRow ({ Space(), Space(&enableButton, 150.0f), Space() }, 30.0f);
    
    layout.updateComponentBounds();
}

void CrossfeedControl::setListener (CalibrationListener* listener)
{
    calibrationListener = listener;
}

void CrossfeedControl::updateEnableButtonText()
{
    enableButton.setButtonText (isCrossfeedEnabled ? "Crossfeed: ON" : "Crossfeed: OFF");
}
