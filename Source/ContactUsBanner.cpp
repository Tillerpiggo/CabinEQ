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
    
    howToButton.setJustificationType (juce::Justification::right);
    howToButton.setColour (juce::HyperlinkButton::ColourIds::textColourId, juce::Colours::orange);
    howToButton.setFont (setupButtonFont, false);
    
    setupButton.setJustificationType (juce::Justification::right);
    setupButton.setColour (juce::HyperlinkButton::ColourIds::textColourId, juce::Colours::orange);
    setupButton.setFont (setupButtonFont, false);
    
    restartAudioButton.setJustificationType (juce::Justification::right);
//    restartAudioButton.setColour (juce::TextButton::ColourIds::textColourId, juce::Colours::red);
    restartAudioButton.setFont (setupButtonFont, false);
    restartAudioButton.onClick = [this] {
        if (listener != nullptr)
            listener->restartAudio();
    };
    
    addAndMakeVisible (contactLabel);
    addAndMakeVisible (setupButton);
    addAndMakeVisible (howToButton);
    addAndMakeVisible (restartAudioButton);
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
    float howToWidth = setupButtonFont.getStringWidth (howToButton.getButtonText()) + 20.0f;
    float setupWidth = setupButtonFont.getStringWidth (setupButton.getButtonText()) + 20.0f;
    float restartAudioWidth = setupButtonFont.getStringWidth (restartAudioButton.getButtonText()) + 20.0f;
    
    Layout layout (getBounds().withX (0).withY (0), 8.0f);
    layout.addRow ({ Space (&contactLabel), Space (20.0f), Space (&howToButton, howToWidth), Space (&setupButton, setupWidth), Space (&restartAudioButton, restartAudioWidth) });
    layout.updateComponentBounds();
}

void ContactUsBanner::setListener (ContactUsBannerListener* listener)
{
    this->listener = listener;
}
