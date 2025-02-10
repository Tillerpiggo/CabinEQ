/*
  ==============================================================================

    CabinEqUnlockForm.h
    Created: 20 Jan 2025 7:21:24pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "CabinEqMarketplaceStatus.h"

#include <JuceHeader.h>

class CabinEqUnlockForm    : public juce::OnlineUnlockForm
{
public:
    CabinEqUnlockForm (CabinEqMarketplaceStatus& status)
        : OnlineUnlockForm (status, "Please provide your email and password.")
    {
        addAndMakeVisible (setupButton);
        setupButton.setFont (juce::Font (juce::FontOptions (16)), juce::NotificationType::dontSendNotification);
    }
    
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);
    }
    
    void resized() override
    {
        setupButton.setBounds (0, getHeight() - 80.0f, getWidth(), 60.0f);
    }

    void dismiss() override
    {
        setVisible (false);
    }
    
    juce::HyperlinkButton setupButton { "To get account info, buy license at cabinaudio.com", juce::URL("https://cabinaudio.com") };
};
