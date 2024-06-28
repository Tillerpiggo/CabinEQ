/*
  ==============================================================================

    Slider.h
    Created: 27 Jun 2024 5:22:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SliderPair : public juce::Component
{
public:
    SliderPair(const std::string& gainName, const std::string& balanceName,
               const std::string& gainParam, const std::string& balanceParam,
               juce::AudioProcessorValueTreeState& parameters, const int i)
    {
        addSlider (gainSlider, gainLabel, gainName, gainParam, parameters, i);
        addSlider (balanceSlider, balanceLabel, balanceName, balanceParam, parameters, i + SetPointManager::NUM_SET_POINTS);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        gainLabel.setBounds (area.removeFromTop (20));
        gainSlider.setBounds (area.removeFromTop (60));
        balanceLabel.setBounds (area.removeFromTop (20));
        balanceSlider.setBounds (area.removeFromTop (60));
    }
    
    void addSliderListener(juce::Slider::Listener* listener)
    {
        gainSlider.addListener(listener);
        balanceSlider.addListener(listener);
    }
    
    void removeSliderListener(juce::Slider::Listener* listener)
    {
        gainSlider.removeListener(listener);
        balanceSlider.removeListener(listener);
    }

private:
    void addSlider(juce::Slider& slider, juce::Label& label, const std::string& name, const std::string& paramName,
                   juce::AudioProcessorValueTreeState& parameters, const int i)
    {
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setRange (-24.f, 48.f, 0.1f);
        slider.getProperties().set ("index", i);
        addAndMakeVisible (slider);

        label.setText (name, juce::dontSendNotification);
        label.attachToComponent (&slider, true);
        addAndMakeVisible (label);

        attachments.emplace_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            parameters, paramName, slider));
    }


    juce::Slider gainSlider;
    juce::Slider balanceSlider;
    juce::Label gainLabel;
    juce::Label balanceLabel;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
};
