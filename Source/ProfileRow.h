/*
  ==============================================================================

    ProfileRow.h
    Created: 21 Feb 2025 11:28:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "Layout.h"

// Displays a single profile row in the profile list, with ellipsis to show other options
class ProfileRow :  public BuildableComponent
{
public:
    ProfileRow();
    
    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;

    void setProfileName (const juce::String& profileName);
    void setIsSelected (bool isSelected);

private:
    juce::Label profileNameLabel;
    juce::ImageButton ellipsisButton;

    bool isHovering = false;
    bool isSelected = false;
};
