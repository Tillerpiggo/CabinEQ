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
    juce::HyperlinkButton howToButton { "How to use", juce::URL("https://docs.google.com/document/d/162MVRm11t0VAeSKOezLR6heoVUkZqhfN9HOny6xhnEk/edit?usp=sharing") };
    juce::HyperlinkButton setupButton { "How to get sound to work", juce::URL("https://docs.google.com/document/d/1QiblNNfKgauabegU9ERn_jlJF5BGtEAp9przLzF1-o8/edit?tab=t.0") };
    juce::HyperlinkButton restartAudioButton { "Restart Audio", juce::URL() };
};
