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
#include "ProfileRow.h"

class ProfileList : public juce::Component,
                    public juce::ListBoxModel,
                    public ProfileRow::ProfileRowListener
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
    juce::Component* refreshComponentForRow (int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

    // ProfileRowListener methods
    void profileRowClicked (int row) override;
    void profileRowOptionsClicked (int row) override;
    void profileRowRenamed (int row, juce::String newProfileName) override;
    
    void updateContent(); // triggers an update of the list box content
    void scrollToBottom();

    // ProfileListListener methods
    void setListener (ProfileListListener* listener);
    void setDataSource (ProfileListDataSource* dataSource);

private:
    ProfileListListener* listener = nullptr;
    ProfileListDataSource* dataSource = nullptr;

    std::unique_ptr<juce::PopupMenu> optionsMenu;

    juce::ListBox listBox;

    int selectedRowNumber = -1;
};
    
    
