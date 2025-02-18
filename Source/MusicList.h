/*
  ==============================================================================

    MusicList.h
    Created: 17 Feb 2025 4:26:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Layout.h"
#include "Listeners.h"

// Displays a list of music to be selected, potentially representing checkerboards or uploaded songs
class MusicList  : public juce::Component,
                   public juce::ListBoxModel
{
public:
    MusicList();
    ~MusicList();
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    // ListBoxModel overrides
    int getNumRows() override;
    juce::String getNameForRow (int rowNumber) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& event);
    
    void setListener (MusicListListener* listener);
    void setDataSource (CheckerboardViewDataSource* dataSource);
    
private:
    MusicListListener* listener;
    CheckerboardViewDataSource* dataSource;
    
    juce::ListBox listBox;
    
    juce::Colour SELECTED_COLOUR = juce::Colours::teal;
    juce::Colour UNSELECTED_COLOUR = juce::Colours::darkgrey;
    
};
