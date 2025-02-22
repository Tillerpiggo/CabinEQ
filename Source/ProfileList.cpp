/*
  ==============================================================================

    ProfileList.cpp
    Created: 21 Feb 2025 9:30:33pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ProfileList.h"

ProfileList::ProfileList()
{
    addAndMakeVisible (listBox);
    listBox.setModel (this);
    listBox.setRowHeight (40);
    listBox.addMouseListener (this, true);

    optionsMenu = std::make_unique<juce::PopupMenu>();
    optionsMenu->addItem ("Duplicate", [this] {
        if (listener != nullptr)
            listener->duplicateProfile (optionsMenuRow);
        optionsMenuRow = -1;
    });
    optionsMenu->addItem ("Rename", [this] {
        editingRowNumber = optionsMenuRow;
        updateContent();
    });
    optionsMenu->addSeparator();
    optionsMenu->addColouredItem (3, "Delete", juce::Colours::red); // handle on return
}

void ProfileList::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void ProfileList::resized()
{
    listBox.setBounds (getLocalBounds());
}

int ProfileList::getNumRows()
{
    if (dataSource == nullptr)
        return 0;
    int numRows = static_cast<int> (dataSource->getProfileNames().size());
    numRows += isAddingProfile ? 1 : 0;
    return numRows;
}

juce::String ProfileList::getNameForRow (int rowNumber)
{
    if (dataSource == nullptr)
        return juce::String();
    if (rowNumber < 0 || rowNumber >= dataSource->getProfileNames().size())
    {
        return "";
    }
    if (isAddingProfile && rowNumber == dataSource->getProfileNames().size())
    {
        return "Adding Profile...";
    }
    return dataSource->getProfileNames()[rowNumber];
}

void ProfileList::paintListBoxItem (int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected)
{
    g.fillAll (juce::Colours::lightblue);
    g.setColour (juce::Colours::white);
    g.drawText (getNameForRow (rowNumber), 0, 0, width, height, juce::Justification::centred);
}

void ProfileList::listBoxItemClicked (int row, const juce::MouseEvent& event)
{
    if (listener != nullptr)
        listener->selectedRow (row);
    listBox.selectRow (row);
    std::cout << "list box item clicked, row: " << row << std::endl;
}

void ProfileList::selectedRowsChanged (int lastRowSelected)
{
    if (listener != nullptr)
        listener->selectedRow (lastRowSelected);
    std::cout << "selected rows changed to: " << lastRowSelected << std::endl;
}

juce::Component* ProfileList::refreshComponentForRow (int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate)
{
    ProfileRow* row = nullptr;
    
    if (existingComponentToUpdate == nullptr)
    {
        row = new ProfileRow (rowNumber);
        row->setListener (this);
    }
    else
    {
        row = dynamic_cast<ProfileRow*>(existingComponentToUpdate);
        if (row == nullptr)
        {
            delete existingComponentToUpdate;
            row = new ProfileRow (rowNumber);
            row->setListener (this);
        }
    }
    
    row->setProfileName (getNameForRow (rowNumber));
    row->setRowNumber (rowNumber);
    row->setIsSelected (rowNumber == selectedRowNumber);
    row->setIsEditing (rowNumber == editingRowNumber);

    // Make the adding profile row editable
    if (isAddingProfile && rowNumber == dataSource->getProfileNames().size())
    {
        row->setIsEditing (true);
    }

    return row;
}

void ProfileList::backgroundClicked (const juce::MouseEvent& event)
{
    isAddingProfile = false;
    updateContent();
}

void ProfileList::profileRowClicked (int row)
{
    // Don't do anything if we click on the adding profile row
    if (row == dataSource->getProfileNames().size() && isAddingProfile)
    {
        return;
    }

    if (listener != nullptr)
        listener->selectedRow (row);
    selectedRowNumber = row;
    optionsMenuRow = -1;
    editingRowNumber = -1;
    isAddingProfile = false;
    updateContent();
}

void ProfileList::profileRowOptionsClicked (int row)
{
    // If we're adding a profile, don't respond to the options menu changing... we should probably just hide it for now anyways
    if (isAddingProfile && row == dataSource->getProfileNames().size())
    {
        return;
    }

    optionsMenuRow = row; // must be before - showAt del
    editingRowNumber = -1;
    int selectedItem = optionsMenu->showAt (listBox.getComponentForRowNumber (row));
    if (selectedItem == 3)
    {
        showAlertWindow();
    }

    isAddingProfile = false;
}

void ProfileList::setListener (ProfileListListener* listener)
{
    this->listener = listener;
}

void ProfileList::setDataSource (ProfileListDataSource* dataSource)
{
    this->dataSource = dataSource;
    updateContent();
}

void ProfileList::setIsAddingProfile (bool isAddingProfile)
{
    this->isAddingProfile = isAddingProfile;
    updateContent();
    scrollToBottom();
}

void ProfileList::profileRowRenamed (int row, juce::String newProfileName)
{
    if (listener != nullptr)
    {
        // If we're adding a profile, then rename should add the profile
        if (isAddingProfile && row == dataSource->getProfileNames().size())
        {
            listener->addProfile (newProfileName);
            isAddingProfile = false;
        }

        // Otherwise, just rename the profile
        else
        {
            listener->renameProfile (row, newProfileName);
        }
    }
    editingRowNumber = -1;
    isAddingProfile = false;
    updateContent();
}

void ProfileList::profileRowRenameCancelled (int row)
{
    editingRowNumber = -1;
    isAddingProfile = false;
    updateContent();
}

void ProfileList::updateContent()
{
    listBox.updateContent();
}

void ProfileList::scrollToBottom()
{
    listBox.scrollToEnsureRowIsOnscreen (getNumRows() - 1);
}

void ProfileList::showAlertWindow()
{
    alertWindow = std::make_unique<juce::AlertWindow> ("Delete Profile", "Are you sure you want to delete this profile?", juce::MessageBoxIconType::NoIcon);
    alertWindow->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    alertWindow->addButton ("Delete", 1, juce::KeyPress (juce::KeyPress::returnKey));
    alertWindow->setEscapeKeyCancels (true);

    alertWindow->getButton (0)->onClick = [this] {
        dismissAlertWindow();
    };
    alertWindow->getButton (1)->onClick = [this] {
        if (listener != nullptr)
        {
            listener->deleteProfile (optionsMenuRow);
            dismissAlertWindow();
            updateContent();
        }
    };

    alertWindow->enterModalState();
}

void ProfileList::dismissAlertWindow()
{
    alertWindow.reset();
}



