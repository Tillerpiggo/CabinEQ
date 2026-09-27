/*
  ==============================================================================

    CrossfeedControl.cpp
    Created: 18 May 2025 2:56:11am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CrossfeedControl.h"
#include "Theme.h"

CrossfeedControl::CrossfeedControl (juce::AudioProcessorValueTreeState& parameters)
    : enableAttachment (parameters, ParamIDs::crossfeed, enableButton),
      levelAttachment (parameters, ParamIDs::crossfeedLevel, levelSlider),
      delayAttachment (parameters, ParamIDs::crossfeedDelay, delaySlider)
{
    addAndMakeVisible (enableButton);
    enableButton.onStateChange = [this] { updateEnablement(); };

    for (auto [slider, label, text] : { std::tuple { &levelSlider, &levelLabel, "Amount" }, std::tuple { &delaySlider, &delayLabel, "Delay" } })
    {
        slider->setSliderStyle (juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 20);
        addAndMakeVisible (*slider);

        label->setText (text, juce::dontSendNotification);
        label->setFont (Theme::font (12.0f));
        label->setColour (juce::Label::textColourId, Theme::textDim);
        addAndMakeVisible (*label);
    }

    levelSlider.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " dB"; };
    delaySlider.textFromValueFunction = [] (double v) { return juce::String (v, 2) + " ms"; };
    levelSlider.updateText();
    delaySlider.updateText();

    updateEnablement();
    setSize (320, 170);
}

void CrossfeedControl::updateEnablement()
{
    const bool on = enableButton.getToggleState();
    levelSlider.setEnabled (on);
    delaySlider.setEnabled (on);
}

void CrossfeedControl::paint (juce::Graphics& g)
{
    auto text = getLocalBounds().reduced (4, 0).withTrimmedTop (34).removeFromTop (34);
    g.setColour (Theme::textFaint);
    g.setFont (Theme::font (12.0f));
    g.drawFittedText ("Blends a little of each side into the other, like listening to speakers. Makes hard-panned mixes easier on headphones.",
                      text, juce::Justification::topLeft, 2);
}

void CrossfeedControl::resized()
{
    auto area = getLocalBounds().reduced (4, 0);
    enableButton.setBounds (area.removeFromTop (28));
    area.removeFromTop (44);

    for (auto [slider, label] : { std::pair { &levelSlider, &levelLabel }, std::pair { &delaySlider, &delayLabel } })
    {
        auto row = area.removeFromTop (30);
        label->setBounds (row.removeFromLeft (64));
        slider->setBounds (row);
        area.removeFromTop (4);
    }
}
