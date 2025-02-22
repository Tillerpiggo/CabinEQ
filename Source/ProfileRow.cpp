/*
  ==============================================================================

    ProfileRow.cpp
    Created: 21 Feb 2025 11:28:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ProfileRow.h"

ProfileRow::ProfileRow (int rowNumber)
    : rowNumber (rowNumber)
{
    addAndMakeVisible (profileNameLabel);
    addAndMakeVisible (ellipsisButton);

    profileNameLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    profileNameLabel.setJustificationType (juce::Justification::left);
    profileNameLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    profileNameLabel.setInterceptsMouseClicks (false, false);

    ellipsisButton.setImages (false, true, true, ellipsisImage, 1.0f, juce::Colours::white.withAlpha (0.0f), ellipsisImage, 1.0f, juce::Colours::black.withAlpha (0.1f), ellipsisImage, 1.0f, juce::Colours::black.withAlpha (0.2f));
    ellipsisButton.onClick = [this, rowNumber] {
        if (listener != nullptr)
        {
            listener->profileRowOptionsClicked (rowNumber);
//            listener->profileRowClicked (rowNumber);
        }
    };

    addAndMakeVisible (renameEditor);
    renameEditor.setFont (juce::Font (juce::FontOptions (16.0f, juce::Font::bold)));
    renameEditor.setJustification (juce::Justification::left);
    renameEditor.setColour (juce::Label::textColourId, juce::Colours::white);
    renameEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::black.withAlpha (0.0f));
    renameEditor.setColour (juce::TextEditor::outlineColourId, juce::Colours::black.withAlpha (0.0f));
    renameEditor.setColour (juce::TextEditor::outlineColourId, juce::Colours::white);
    renameEditor.setInterceptsMouseClicks (false, false);
    renameEditor.setMultiLine (false);
    renameEditor.setVisible (false);
    renameEditor.addListener (this);
}

void ProfileRow::paint (juce::Graphics& g)
{
    if (isSelected)
        g.fillAll (juce::Colours::lightblue);
    else if (isHovering)
        g.fillAll (juce::Colours::darkgrey);
    else
        g.fillAll (juce::Colours::black);
}

void ProfileRow::resized()
{
    Layout layout (getBounds().withX (0).withY (0), 8.0f);
    layout.addRow ({ Space(&profileNameLabel, &renameEditor), Space (&ellipsisButton, 20.0f) }, 20.0f);
    layout.updateComponentBounds();
}

void ProfileRow::mouseEnter (const juce::MouseEvent& event)
{
    isHovering = true;
    repaint();
}

void ProfileRow::mouseExit (const juce::MouseEvent& event)
{
    isHovering = false;
    repaint();
}

void ProfileRow::mouseDown (const juce::MouseEvent& event)
{
    if (listener != nullptr)
        listener->profileRowClicked (rowNumber);
}

void ProfileRow::setProfileName (const juce::String& profileName)
{
    this->profileName = profileName;
    profileNameLabel.setText (profileName, juce::dontSendNotification);
//    std::cout << "profile name set: " << profileName << ", row: " << rowNumber << std::endl;
}

void ProfileRow::setIsSelected (bool isSelected)
{
    this->isSelected = isSelected;
    repaint();
}

void ProfileRow::setIsHovering (bool isHovering)
{
    this->isHovering = isHovering;
    repaint();
}

void ProfileRow::setIsEditing (bool isEditing)
{
    this->isEditing = isEditing;
    renameEditor.setVisible (isEditing);
    renameEditor.setText ("rename...");
    profileNameLabel.setVisible (! isEditing);
    repaint();
    renameEditor.grabKeyboardFocus();
    std::cout << "set is editing: " << isEditing << std::endl;
}

void ProfileRow::setRowNumber (int rowNumber)
{
    this->rowNumber = rowNumber;
}

void ProfileRow::textEditorReturnKeyPressed (juce::TextEditor& editor)
{
    std::cout << "textEditorReturnKeyPressed" << std::endl;
    if (listener != nullptr && editor.getText().isNotEmpty())
    {
        listener->profileRowRenamed (rowNumber, editor.getText());
        setIsEditing (false);
    }
}

void ProfileRow::textEditorEscapeKeyPressed (juce::TextEditor& editor)
{
    setIsEditing (false);
}

void ProfileRow::setListener (ProfileRowListener* listener)
{
    this->listener = listener;
}

