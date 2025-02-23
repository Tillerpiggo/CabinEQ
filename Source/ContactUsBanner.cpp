/*
  ==============================================================================

    ContactUsBanner.cpp
    Created: 7 Feb 2025 4:09:31pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ContactUsBanner.h"

ContactUsBanner::ContactUsBanner()
{
    contactLabel.setJustificationType (juce::Justification::left);
    contactLabel.setText ("Contact Us | julian@cabinaudio.com | tyler@cabinaudio.com", juce::NotificationType::dontSendNotification);
    
    addAndMakeVisible (contactLabel);
    addAndMakeVisible (tutorialButton);

    tutorialButton.onClick = [this] {
        // TODO: Show tutorial
    };
}

ContactUsBanner::~ContactUsBanner()
{
}

void ContactUsBanner::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::teal);
}

void ContactUsBanner::resized()
{
    float tutorialWidth = 100.0f;
    Layout layout (getBounds().withX (0).withY (0), 8.0f);
    layout.addRow ({ Space (&contactLabel), Space (20.0f), Space (&tutorialButton, tutorialWidth) });
    layout.updateComponentBounds();
}

void ContactUsBanner::setListener (ContactUsBannerListener* listener)
{
    this->listener = listener;
}
