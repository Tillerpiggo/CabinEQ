/*
  ==============================================================================

    AddProfileButton.cpp
    Created: 22 Feb 2025 7:36:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "AddProfileButton.h"

AddProfileButton::AddProfileButton()
{
    label.setText ("New Profile", juce::dontSendNotification);
    label.setFont (juce::Font (16.0f));
    label.setColour (juce::Label::textColourId, textColour);
    addAndMakeVisible (&label);

    icon.setImage (iconImage);
    addAndMakeVisible (&icon);
}

AddProfileButton::~AddProfileButton()
{
}

void AddProfileButton::paint (juce::Graphics& g)
{
    g.setColour (currBackgroundColour);
    g.fillRect (getLocalBounds());
}

void AddProfileButton::resized()
{
    Layout layout (getLocalBounds().withX (0).withWidth (getWidth() / 2), 8);
    layout.addRow ({ Space (&icon, 10.0f), Space (&label) });
    layout.updateComponentBounds();
}

void AddProfileButton::mouseEnter (const juce::MouseEvent& event)
{
    currBackgroundColour = hoverColour;
}

void AddProfileButton::mouseExit (const juce::MouseEvent& event)
{
    currBackgroundColour = backgroundColour;
}

void AddProfileButton::mouseDown (const juce::MouseEvent& event)
{
    currBackgroundColour = pressedColour;
}

void AddProfileButton::mouseUp (const juce::MouseEvent& event)
{
    currBackgroundColour = hoverColour;
    if (onClick != nullptr)
        onClick();
}
