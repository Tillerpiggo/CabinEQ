/*
  ==============================================================================

    ProfileRow.cpp
    Created: 21 Feb 2025 11:28:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ProfileRow.h"

ProfileRow::ProfileRow()
{
    addAndMakeVisible (profileNameLabel);
    addAndMakeVisible (ellipsisButton);

    profileNameLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    profileNameLabel.setJustificationType (juce::Justification::left);
    profileNameLabel.setColour (juce::Label::textColourId, juce::Colours::white);
}

void ProfileRow::paint (juce::Graphics& g)
{
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
    profileNameLabel.setColour (juce::Label::textColourId, juce::Colours::lightblue);
}

void ProfileRow::mouseExit (const juce::MouseEvent& event)
{
    profileNameLabel.setColour (juce::Label::textColourId, juce::Colours::white);
}

void ProfileRow::setProfileName (const juce::String& profileName)
{
    profileNameLabel.setText (profileName, juce::dontSendNotification);
}

