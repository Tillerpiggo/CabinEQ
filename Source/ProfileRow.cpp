/*
  ==============================================================================

    ProfileRow.cpp
    Created: 21 Feb 2025 11:28:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ProfileRow.h"

ProfileRow::ProfileRow (int rowNumber)
    : rowNumber (rowNumber)
{
    addAndMakeVisible (profileNameLabel);
    addAndMakeVisible (ellipsisButton);

    profileNameLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    profileNameLabel.setJustificationType (juce::Justification::left);
    profileNameLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    profileNameLabel.setInterceptsMouseClicks (false, false);
}

void ProfileRow::paint (juce::Graphics& g)
{
    if (isSelected)
        g.fillAll (juce::Colours::lightblue);
    else if (isHovering)
        g.fillAll (juce::Colours::darkgrey);
    else
        g.fillAll (juce::Colours::black);
}

void ProfileRow::resized()
{
    Layout layout (getBounds().withX (0).withY (0), 8.0f);
    layout.addRow ({ Space(&profileNameLabel), Space (&ellipsisButton, 20.0f) }, 20.0f);
    layout.updateComponentBounds();
}

void ProfileRow::mouseEnter (const juce::MouseEvent& event)
{
    isHovering = true;
    repaint();
}

void ProfileRow::mouseExit (const juce::MouseEvent& event)
{
    isHovering = false;
    repaint();
}

void ProfileRow::mouseDown (const juce::MouseEvent& event)
{
    if (listener != nullptr)
        listener->profileRowClicked (rowNumber);
    std::cout << "profile row clicked, row: " << rowNumber << std::endl;
}

void ProfileRow::setProfileName (const juce::String& profileName)
{
    profileNameLabel.setText (profileName, juce::dontSendNotification);
}

void ProfileRow::setIsSelected (bool isSelected)
{
    this->isSelected = isSelected;
    repaint();
}

void ProfileRow::setIsHovering (bool isHovering)
{
    this->isHovering = isHovering;
    repaint();
}

void ProfileRow::setListener (ProfileRowListener* listener)
{
    this->listener = listener;
}

