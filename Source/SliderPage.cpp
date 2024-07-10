/*
  ==============================================================================

    SliderPage.cpp
    Created: 9 Jul 2024 10:58:26pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderPage.h"

SliderPage::SliderPage(StartupMVPAudioProcessor& p)
    : processor(p)
{
    for (int i = 0; i < sliders.size(); ++i)
    {
        sliders[i] = std::make_unique<juce::Slider> (juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
        auto& slider = *sliders[i];
        addAndMakeVisible (sliderContainer);
        sliderContainer.addAndMakeVisible (slider);

        auto paramID = "gain_" + std::to_string(i);
        sliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, paramID, slider);
    }

    viewport.setViewedComponent (&sliderContainer, true);
    addAndMakeVisible (viewport);
}

SliderPage::~SliderPage() = default;

void SliderPage::resized()
{
    auto area = getLocalBounds();
    viewport.setBounds(area);

    int sliderWidth = 50;
    int sliderHeight = area.getHeight();
    int totalWidth = sliders.size() * sliderWidth;

    sliderContainer.setSize(totalWidth, sliderHeight);

    for (int i = 0; i < sliders.size(); ++i)
    {
        auto& slider = *sliders[i];
        slider.setBounds(i * sliderWidth, 0, sliderWidth, sliderHeight);
    }
}
