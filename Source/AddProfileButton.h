/*
  ==============================================================================

    AddProfileButton.h
    Created: 22 Feb 2025 7:36:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "Layout.h"

// A custom button (made from a Component) that is stylized to have "+ New Profile"
class AddProfileButton : public juce::Component
{
public:
    AddProfileButton();
    ~AddProfileButton();

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;

    std::function<void()> onClick; // called when the button is pressed

private:
    // UI Constants
    juce::Colour backgroundColour = juce::Colours::white;
    juce::Colour hoverColour = juce::Colours::lightgrey;
    juce::Colour pressedColour = juce::Colours::darkgrey;
    juce::Colour textColour = juce::Colours::black;

    juce::Label label;
    juce::ImageComponent icon;
    
    juce::Image iconImage = juce::ImageFileFormat::loadFrom (BinaryData::PlusIcon_png, BinaryData::PlusIcon_pngSize);
};
