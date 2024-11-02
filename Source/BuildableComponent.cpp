/*
  ==============================================================================

    BuildableComponent.cpp
    Created: 31 Oct 2024 1:42:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "BuildableComponent.h"

BuildableComponent::BuildableComponent()
{
}

BuildableComponent::~BuildableComponent()
{
    for (const auto& button : buttons)
        button->removeListener (this);
    
    for (const auto& slider : sliders)
        slider->removeListener (this);
}

void BuildableComponent::sliderValueChanged (juce::Slider *slider)
{
    if (sliderActions.find (slider) != sliderActions.end()) // if sliderActions contains slider
    {
        sliderActions[slider](slider);
    }
}

void BuildableComponent::buttonClicked (juce::Button *button)
{
    if (buttonActions.find (button) != buttonActions.end()) // if buttonActions contains button
    {
        buttonActions[button](button);
    }
}

void BuildableComponent::addButton (juce::Button& button)
{
    addAndMakeVisible (button);
    button.addListener (this);
    
    buttons.push_back (&button);
}

void BuildableComponent::addSlider (juce::Slider& slider, float lowerBound, float upperBound, float startVal)
{
    addAndMakeVisible (slider);
    slider.setRange (lowerBound, upperBound);
    slider.setValue (startVal);
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.addListener (this);
    
    sliders.push_back (&slider);
}

void BuildableComponent::addSliderAndLabel (juce::Slider& slider, juce::Label& label, juce::String labelText, float lowerBound, float upperBound, float startVal)
{
    addAndMakeVisible (slider);
    addAndMakeVisible (label);
    slider.setRange (lowerBound, upperBound);
    slider.setValue (startVal);
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    label.setText (labelText, juce::dontSendNotification);
    label.attachToComponent (&slider, true);
    
    slider.addListener (this);
    
}

void BuildableComponent::addButtonAction (juce::Button* buttonPtr, std::function<void(juce::Button*)> buttonAction)
{
    buttonActions[buttonPtr] = buttonAction;
}

void BuildableComponent::addSliderAction (juce::Slider* sliderPtr, std::function<void(juce::Slider*)> sliderAction)
{
    sliderActions[sliderPtr] = sliderAction;
}

void BuildableComponent::removeButton (juce::Button* buttonToRemove)
{
    // Find the button in buttons array and remove it
    int buttonIdxToRemove = -1;
    for (int i = 0; i < buttons.size(); ++i)
    {
        if (buttonToRemove == buttons[i])
        {
            buttonIdxToRemove = i;
            break;
        }
    }
    buttons.erase (buttons.begin() + buttonIdxToRemove);
    
    // If we can find it in the button actions map, remove it too
    auto buttonActionIter = buttonActions.find (buttonToRemove);
    if (buttonActionIter != buttonActions.end())
    {
        buttonActions.erase (buttonActionIter);
    }
}

void BuildableComponent::removeSlider (juce::Slider* sliderToRemove)
{
    // Find the slider in buttons array and remove it
    int sliderIdxToRemove = -1;
    for (int i = 0; i < sliders.size(); ++i)
    {
        if (sliderToRemove == sliders[i])
        {
            sliderIdxToRemove = i;
            break;
        }
    }
    sliders.erase (sliders.begin() + sliderIdxToRemove);
    
    // If we can find it in the slider actions map, remove it too
    auto sliderActionIter = sliderActions.find (sliderToRemove);
    if (sliderActionIter != sliderActions.end())
    {
        sliderActions.erase (sliderActionIter);
    }
}
