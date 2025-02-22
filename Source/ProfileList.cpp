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
    std::cout << "ProfileList constructor start" << std::endl;
    addAndMakeVisible (listBox);
    std::cout << "listBox added" << std::endl;
    listBox.setModel (this);
    std::cout << "ProfileList constructor end" << std::endl;
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
   return dataSource->getProfileNames()[rowNumber];
}

void ProfileList::paintListBoxItem (int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected)
{
    g.fillAll (juce::Colours::black);
    g.setColour (juce::Colours::white);
    g.drawText (getNameForRow (rowNumber), 0, 0, width, height, juce::Justification::centred);
    std::cout <<  "Painting list box item " << rowNumber << std::endl;
}

void ProfileList::listBoxItemClicked (int row, const juce::MouseEvent& event)
{
    if (listener != nullptr)
        listener->selectedRow (row);
}

void ProfileList::selectedRowsChanged (int lastRowSelected)
{
    if (listener != nullptr)
        listener->selectedRow (lastRowSelected);
}

void ProfileList::setListener (ProfileListListener* listener)
{
    this->listener = listener;
}

void ProfileList::setDataSource (ProfileListDataSource* dataSource)
{
    this->dataSource = dataSource;
    listBox.updateContent();
}














