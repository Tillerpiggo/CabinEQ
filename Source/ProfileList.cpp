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
//        if (listener != nullptr)
//            listener->renameProfile (optionsMenuRow, juce::String());
        std::cout << "options menu row: " << optionsMenuRow << std::endl;
        editingRowNumber = optionsMenuRow;
        updateContent();
    });
    optionsMenu->addSeparator();
    optionsMenu->addItem ("Delete", [this] {
        if (listener != nullptr)
            listener->deleteProfile (optionsMenuRow);
        optionsMenuRow = -1;
    });
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
    return static_cast<int> (dataSource->getProfileNames().size());
}

juce::String ProfileList::getNameForRow (int rowNumber)
{
    if (dataSource == nullptr)
        return juce::String();
    if (rowNumber < 0 || rowNumber >= dataSource->getProfileNames().size())
    {
        return "OUT OF BOUNDS";
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
    return row;
}

void ProfileList::profileRowClicked (int row)
{
    if (listener != nullptr)
        listener->selectedRow (row);
    selectedRowNumber = row;
    optionsMenuRow = -1;
    editingRowNumber = -1;
    listBox.updateContent();
}

void ProfileList::profileRowOptionsClicked (int row)
{
    optionsMenuRow = row; // must be before - showAt del
    editingRowNumber = -1;
    optionsMenu->showAt (listBox.getComponentForRowNumber (row));
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

void ProfileList::profileRowRenamed (int row, juce::String newProfileName)
{
    if (listener != nullptr)
        listener->renameProfile (row, newProfileName);
    editingRowNumber = -1;
}

void ProfileList::updateContent()
{
    listBox.updateContent();
}

void ProfileList::scrollToBottom()
{
    listBox.scrollToEnsureRowIsOnscreen (getNumRows() - 1);
}
