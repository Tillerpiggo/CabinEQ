/*
  ==============================================================================

    TutorialButton.cpp
    Created: 22 Feb 2025 8:23:32pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "TutorialButton.h"

TutorialButton::TutorialButton()
{
    label.setText ("Tutorial", juce::dontSendNotification);
    label.setFont (juce::Font (24.0f));
    label.setColour (juce::Label::textColourId, textColour);
}

void TutorialButton::paint (juce::Graphics& g)
{
    g.setColour (currBackgroundColour);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 5);
}

void TutorialButton::resized()
{
    Layout layout (getBounds().withX (0).withY (0).withWidth (100).withHeight (30));
    layout.addRow ({ Space (&icon, 10), Space (&label) });
    layout.updateComponentBounds();
}

void TutorialButton::mouseEnter (const juce::MouseEvent& event)
{
    currBackgroundColour = hoverColour;
}

void TutorialButton::mouseExit (const juce::MouseEvent& event)
{
    currBackgroundColour = backgroundColour;
}

void TutorialButton::mouseDown (const juce::MouseEvent& event)
{
    currBackgroundColour = pressedColour;
}

void TutorialButton::mouseUp (const juce::MouseEvent& event)
{
    if (onClick != nullptr)
        onClick();
}






