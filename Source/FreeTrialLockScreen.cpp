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
    addAndMakeVisible (explanationLabel2);
    addAndMakeVisible (cabinaudioButton);
//    addButton (&buyButton);
//    addButton (&activateLicenseButton);
    
    titleLabel.setText ("Profile Locked", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions(36, juce::Font::bold)));
    titleLabel.setJustificationType (juce::Justification::centred);
    explanationLabel.setText ("You're using the free trial of CabinEQ. Buy a license at ", juce::dontSendNotification);
    explanationLabel.setJustificationType (juce::Justification::centred);
    explanationLabel2.setText (" to unlock your profiles.", juce::dontSendNotification);
    explanationLabel2.setJustificationType (juce::Justification::centred);
    
    cabinaudioButton.setFont (explanationLabel.getFont(), false);
    cabinaudioButton.changeWidthToFitText();
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
    // Get widths of text
    auto font = explanationLabel.getFont();
    float textWidth = font.getStringWidth (explanationLabel.getText());
    float textWidth2 = font.getStringWidth (explanationLabel2.getText());
    float cabinaudioWidth = font.getStringWidth (cabinaudioButton.getButtonText()) + 20;
    
    Layout layout (getBounds().withX (0).withY (0), -8.0f);
    layout.addRow ({ Space() });
    layout.addRow ({ Space (&titleLabel) });
    layout.addRow ({ Space(), Space (&explanationLabel, textWidth), Space (&cabinaudioButton, cabinaudioWidth), Space (&explanationLabel2, textWidth2), Space() });
//    layout.addRow ({ Space (&buyButton) });
    layout.addRow ({ Space() });
    layout.updateComponentBounds();
}
