/*
  ==============================================================================

    MusicList.cpp
    Created: 17 Feb 2025 4:26:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "MusicList.h"

MusicList::MusicList()
{
    addAndMakeVisible (listBox);
    listBox.setModel (this);
}

MusicList::~MusicList()
{
    
}

void MusicList::paint (juce::Graphics& g)
{
    
}

void MusicList::resized()
{
    listBox.setBounds (getBounds());
}

// ListBoxComponent stuff...
int MusicList::getNumRows()
{
    if (dataSource == nullptr)
        return 1;
    return dataSource->getNumRows();
}

juce::String MusicList::getNameForRow (int rowNumber)
{
    if (dataSource == nullptr)
        return "Loading...";
    return dataSource->getNameAtRow (rowNumber);
}

void MusicList::paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    g.setColour (rowIsSelected ? SELECTED_COLOUR : UNSELECTED_COLOUR);
    
    // Draw a rectangle at the list depending
    juce::Rectangle<float> area = { 0, 0, static_cast<float>(width), static_cast<float>(height) };
    g.fillRect (area);
    
    // Fill in the text
    g.setColour (juce::Colours::white);
    g.drawText (getNameForRow (rowNumber), area, juce::Justification::centredLeft);
}

void MusicList::listBoxItemClicked (int row, const juce::MouseEvent& event)
{
    if (listener != nullptr)
    {
        listener->selectedRow (row);
    }
}

void MusicList::setListener (MusicListListener* listener)
{
    this->listener = listener;
}

void MusicList::setDataSource (CheckerboardViewDataSource* dataSource)
{
    this->dataSource = dataSource;
    listBox.updateContent();
}
