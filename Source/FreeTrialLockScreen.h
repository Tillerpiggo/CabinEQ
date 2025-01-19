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
    juce::TextButton buyButton { "Buy ($120)" };
};
