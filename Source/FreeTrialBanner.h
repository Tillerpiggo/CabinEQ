/*
  ==============================================================================

    FreeTrialBanner.h
    Created: 18 Jan 2025 1:21:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "Layout.h"
#include "Listeners.h"

// This represent the free trial button, which shows a timer associated with the free trial and gives a pop up to buy it
class FreeTrialBanner  : public BuildableComponent,
                         public juce::Timer
{
public:
    FreeTrialBanner();
    ~FreeTrialBanner() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (FreeTrialListener* listener);
    
    void timerCallback() override;
    
private:
    std::string convertSecondsToTimeFormat (int seconds);
    
    FreeTrialListener* listener;
    
    int secondsLeftUntilReset;
    int resetCycleInSeconds = 60 * 20; // 20 minutes
    
    juce::Label timeLabel;
    juce::Label freeTrialLabel;
    juce::TextButton activateLicenseButton { "Sign in to activate license" };
};
