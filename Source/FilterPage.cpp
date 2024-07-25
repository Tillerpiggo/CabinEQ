/*
  ==============================================================================

    FilterPage.cpp
    Created: 11 Jul 2024 11:53:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "FilterPage.h"

FilterPage::FilterPage(StartupMVPAudioProcessor& p)
    : processor(p), curveComponent(p.getSetPointManager().getCurve()), isBypassed (false)
{
    balanceSlider.setRange(-12.0, 12.0);
    balanceSlider.setTextValueSuffix(" dB");
    balanceSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    balanceSlider.setTextBoxStyle(juce::Slider::TextBoxRight, true, 100, 20);

    applyFilterButton.addListener(this);
    bypassButton.addListener(this);
    balanceSlider.addListener(this);

    addAndMakeVisible(curveComponent);
    addAndMakeVisible(applyFilterButton);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(balanceSlider);
}

FilterPage::~FilterPage()
{
    applyFilterButton.removeListener(this);
    bypassButton.removeListener(this);
    balanceSlider.removeListener(this);
}

void FilterPage::resized()
{
    auto area = getLocalBounds();

    auto halfHeight = area.getHeight() / 2;
    auto buttonHeight = 100;
    auto sliderHeight = 50;
    auto buttonWidth = area.getWidth() / 2;

    curveComponent.setBounds(area.removeFromTop(halfHeight));

    auto buttonArea = area.removeFromTop(buttonHeight).reduced(10);
    applyFilterButton.setBounds(buttonArea.removeFromLeft(buttonWidth - 10));
    bypassButton.setBounds(buttonArea);

    balanceSlider.setBounds(area.removeFromTop(sliderHeight).reduced(10));
}

void FilterPage::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(30, 30, 30));

    auto sliderThumbColor = juce::Colour::fromRGB(0, 255, 128); // Curve green
    auto sliderTrackColor = juce::Colour::fromRGB(70, 70, 70); // Dark grey track
    auto sliderBackgroundColor = juce::Colour::fromRGB(40, 40, 40); // Matching dark background
    auto textBoxBackgroundColor = juce::Colour::fromRGB(50, 50, 50); // Slightly lighter background for text box
    auto textBoxTextColor = juce::Colour::fromRGB(255, 255, 255); // White text in the text box

    balanceSlider.setColour(juce::Slider::thumbColourId, sliderThumbColor);
    balanceSlider.setColour(juce::Slider::trackColourId, sliderTrackColor);
    balanceSlider.setColour(juce::Slider::backgroundColourId, sliderBackgroundColor);
    balanceSlider.setColour(juce::Slider::textBoxBackgroundColourId, textBoxBackgroundColor);
    balanceSlider.setColour(juce::Slider::textBoxTextColourId, textBoxTextColor);
}

void FilterPage::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &balanceSlider)
    {
        processor.setBypassBalance(slider->getValue());
    }
}

void FilterPage::buttonClicked(juce::Button* button)
{
    if (button == &applyFilterButton)
    {
        processor.applyCurve();
    }
    else if (button == &bypassButton)
    {
        isBypassed = ! isBypassed;
        processor.setIsBypassed (isBypassed);
    }
}
