/*
  ==============================================================================

    FreeTrialLockScreen.h
    Created: 19 Jan 2025 11:27:12am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "Layout.h"

class FreeTrialLockScreen  : public BuildableComponent
{
public:
    FreeTrialLockScreen();
    ~FreeTrialLockScreen() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
private:
    juce::Label titleLabel;
    juce::Label explanationLabel;
    juce::HyperlinkButton cabinaudioButton { "cabinaudio.com", juce::URL("https://cabinaudio.webflow.io") };
    juce::Label explanationLabel2;
//    juce::TextButton activateLicenseButton { "Activate License" };
//    juce::TextButton buyButton { "Go to cabinaudio.com ->" };
};
