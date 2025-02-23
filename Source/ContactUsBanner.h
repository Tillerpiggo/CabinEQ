/*
  ==============================================================================

    ContactUsBanner.h
    Created: 7 Feb 2025 4:09:31pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "Layout.h"
#include "Listeners.h"
#include "TutorialButton.h"

// This explains that the app is new and being rapidly developed and needs your help to improve
class ContactUsBanner  : public BuildableComponent
{
public:
    ContactUsBanner();
    ~ContactUsBanner() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (ContactUsBannerListener* listener);
    
private:
    ContactUsBannerListener* listener = nullptr;
    
    juce::Font setupButtonFont { juce::FontOptions (16) };
    juce::Label contactLabel;
    TutorialButton tutorialButton;
};
