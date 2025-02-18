/*
  ==============================================================================

    MusicList2.h
    Created: 17 Feb 2025 6:29:36pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class MusicList  : public juce::Component,
                   public juce::ListBoxModel
{
public:
    MusicList();

    void paint (juce::Graphics& g) override;
    void resized() override;
    
    // ListBoxModel methods
    int getNumRows() override;
    juce::String getNameForRow (int rowNumber) override;
    void paintListBoxItem (int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    
private:
    juce::ListBox listBox;
    
    juce::Colour SELECTED_COLOUR = juce::Colours::teal;
    juce::Colour UNSELECTED_COLOUR = juce::Colours::darkgrey;
    juce::Colour TEXT_COLOUR = juce::Colours::white;
};
