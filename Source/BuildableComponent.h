/*
  ==============================================================================

    BuildableComponent.h
    Created: 31 Oct 2024 1:33:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This is a component that provides helper methods that automate some of the boilerplate when adding sliders and buttons to a juce::Component
class BuildableComponent  : juce::Component,
                            juce::Button::Listener,
                            juce::Slider::Listener
{
    BuildableComponent();
    ~BuildableComponent() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void buttonClicked (juce::Button *button) override;
    
private:
    void addButton (juce::Button& button);
    void addSlider (juce::Slider& slider, float lowerBound = 0.0f, float upperBound = 1.0f, float startVal = 0.5f);
    void addSliderAndLabel (juce::Slider& slider, juce::Label& label, juce::String labelText, float lowerBound = 0.0f, float upperBound = 1.0f, float startVal = 0.5f);
    
    void addButtonAction (juce::Button* buttonPtr, std::function<void(juce::Button*)> buttonAction);
    void addSliderAction (juce::Slider* sliderPtr, std::function<void(juce::Slider*)> sliderAction);
    
    std::vector<juce::Button*> buttons;
    std::vector<juce::Slider*> sliders;
    std::unordered_map<juce::Button*, std::function<void(juce::Button*)>> buttonActions;
    std::unordered_map<juce::Slider*, std::function<void(juce::Slider*)>> sliderActions;
};
