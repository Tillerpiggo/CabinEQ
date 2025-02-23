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
    label.setFont (juce::Font (16.0f));
    label.setColour (juce::Label::textColourId, textColour);

    label.setInterceptsMouseClicks (false, false);
    icon.setInterceptsMouseClicks (false, false);

    addAndMakeVisible (label);
    addAndMakeVisible (icon);
}

void TutorialButton::paint (juce::Graphics& g)
{
    g.setColour (currBackgroundColour);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 5);
}

void TutorialButton::resized()
{
    Layout layout (getBounds().withX (0).withY (0));
    layout.addRow ({ Space (&icon, 10), Space (&label) });
    layout.updateComponentBounds();
}

void TutorialButton::mouseEnter (const juce::MouseEvent& event)
{
    currBackgroundColour = hoverColour;
    repaint();
}

void TutorialButton::mouseExit (const juce::MouseEvent& event)
{
    currBackgroundColour = backgroundColour;
    repaint();
}

void TutorialButton::mouseDown (const juce::MouseEvent& event)
{
    std::cout << "mouse down" << std::endl;
    currBackgroundColour = pressedColour;
    repaint();
}

void TutorialButton::mouseUp (const juce::MouseEvent& event)
{
    currBackgroundColour = hoverColour;
    if (onClick != nullptr)
        onClick();
    repaint();
}






