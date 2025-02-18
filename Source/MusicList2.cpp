/*
  ==============================================================================

    MusicList2.cpp
    Created: 17 Feb 2025 6:29:36pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "MusicList2.h"

MusicList::MusicList()
{
    addAndMakeVisible (listBox);
    listBox.setModel (this);
}

void MusicList::paint (juce::Graphics& g)
{
    
}

void MusicList::resized()
{
    listBox.setBounds (getBounds());
}

// ListBoxModel methods
int MusicList::getNumRows()
{
    return 1;
}

juce::String MusicList::getNameForRow (int rowNumber)
{
    return "Loading...";
}

void MusicList::paintListBoxItem (int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected)
{
    g.setColour (rowIsSelected ? SELECTED_COLOUR : UNSELECTED_COLOUR);
    
    // Get the rect
    juce::Rectangle<float> area { 0, 0, static_cast<float> (width), static_cast<float> (height) };
    g.fillRect (area);
    
    // Draw text
    g.setColour (TEXT_COLOUR);
    g.drawText (getNameForRow (rowNumber), area, juce::Justification::centredLeft);
}
