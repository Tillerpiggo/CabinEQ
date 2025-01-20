/*
  ==============================================================================

    FreeTrialLockScreen.cpp
    Created: 19 Jan 2025 11:27:12am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "FreeTrialLockScreen.h"


FreeTrialLockScreen::FreeTrialLockScreen()
{
    addAndMakeVisible (titleLabel);
    addAndMakeVisible (explanationLabel);
    addButton (&buyButton);
    addButton (&activateLicenseButton);
    
    titleLabel.setText ("Profile Locked", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions(36, juce::Font::bold)));
    titleLabel.setJustificationType (juce::Justification::centred);
    explanationLabel.setText ("You're using the free trial of CabinEQ. Buy a license to unlock your profiles.", juce::dontSendNotification);
    explanationLabel.setJustificationType (juce::Justification::centred);
    
    addButtonAction (&buyButton, [this](juce::Button*) {
        // Buy the license..
    });
}

FreeTrialLockScreen::~FreeTrialLockScreen()
{
    
}

void FreeTrialLockScreen::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.9f));
}

void FreeTrialLockScreen::resized()
{
    Layout layout (getBounds().withX (0).withY (0), 16.0f);
    layout.addRow ({ Space() });
    layout.addRow ({ Space (&titleLabel) });
    layout.addRow ({ Space (&explanationLabel) });
    layout.addRow ({ Space (&buyButton) });
    layout.addRow ({ Space() });
    layout.updateComponentBounds();
}
