/*
  ==============================================================================

    ProfileList.h
    Created: 21 Feb 2025 9:30:33pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Listeners.h"

class ProfileList : public juce::Component,
                   public juce::ListBoxModel
{
public:
    ProfileList();

    void paint (juce::Graphics& g) override;
    void resized() override;

    // ListBoxModel methods
    int getNumRows() override;
    juce::String getNameForRow (int rowNumber) override;
    void paintListBoxItem (int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& event) override;
    void selectedRowsChanged (int lastRowSelected) override;

    void setListener (ProfileListListener* listener);
    void setDataSource (ProfileListDataSource* dataSource);

private:
    ProfileListListener* listener = nullptr;
    ProfileListDataSource* dataSource = nullptr;

    juce::ListBox listBox;
};
    
    
