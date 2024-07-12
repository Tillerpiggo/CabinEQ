/*
  ==============================================================================

    LeftRightPage.cpp
    Created: 12 Jul 2024 12:16:55pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "LeftRightPage.h"

LeftRightPage::LeftRightPage(StartupMVPAudioProcessor& p)
    : processor(p)
{
    for (int i = 0; i < sliders.size(); ++i)
    {
        sliders[i] = std::make_unique<juce::Slider> (juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow);
        auto& slider = *sliders[i];
        addAndMakeVisible (sliderContainer);
        sliderContainer.addAndMakeVisible (slider);
        slider.getProperties().set("idx", i);

        auto paramID = "pan_" + std::to_string(i);
        sliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, paramID, slider);
        
        slider.addListener (this);
    }

    viewport.setViewedComponent (&sliderContainer, true);
    addAndMakeVisible (viewport);
    
    startTimer (10);
}

LeftRightPage::~LeftRightPage()
{
    for (auto& slider : sliders)
    {
        slider->removeListener (this);
    }
}

void LeftRightPage::resized()
{
    auto area = getLocalBounds();
    int padding = 10;
    
    viewport.setBounds(area);

    int sliderHeight = 50;
    int sliderWidth = area.getWidth();
    int totalHeight = sliders.size() * sliderHeight;

    sliderContainer.setSize(sliderWidth, totalHeight + 2 * padding);

    for (int i = 0; i < sliders.size(); ++i)
    {
        auto& slider = *sliders[i];
        slider.setBounds(0, padding + i * sliderHeight, sliderWidth, sliderHeight);
    }
}

void LeftRightPage::sliderValueChanged (juce::Slider *slider)
{
    int i = slider->getProperties().getWithDefault("idx", -1);
    processor.getSliderCalibrationManager().setPanAtIdx (i, slider->getValue());
}

void LeftRightPage::sliderDragStarted (juce::Slider *slider)
{
    SliderCalibrationManager& sliderCalibrationManager = processor.getSliderCalibrationManager();
    sliderCalibrationManager.setIsSlidingSlider (true);
    sliderCalibrationManager.setCurrIdx (slider->getProperties().getWithDefault("idx", -1));
}

void LeftRightPage::sliderDragEnded (juce::Slider *slider)
{
    processor.getSliderCalibrationManager().setIsSlidingSlider (false);
}

void LeftRightPage::timerCallback()
{
    int playingIdx = processor.getSliderCalibrationManager().getCurrentlyPlayingIdx();
    
    for (int i = 0; i < sliders.size(); ++i)
    {
        auto& slider = *sliders[i];
        
        if (i == playingIdx)
        {
            slider.setColour (juce::Slider::thumbColourId, juce::Colour::fromRGB(255, 69, 0)); // Orange-Red for active sliders
        }
        else
        {
            slider.setColour (juce::Slider::thumbColourId, juce::Colour::fromRGB(30, 144, 255)); // DodgerBlue for inactive sliders
        }

        slider.setColour(juce::Slider::trackColourId, juce::Colour::fromRGB(70, 70, 70)); // Dark grey track
        slider.setColour(juce::Slider::backgroundColourId, juce::Colour::fromRGB(40, 40, 40)); // Matching dark background
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(50, 50, 50)); // Slightly lighter background for text box
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromRGB(255, 255, 255)); // White text in the text box
    }
}

void LeftRightPage::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB(30, 30, 30)); // Dark background color
}
