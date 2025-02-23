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

    profileNameLabel.setFont (PROFILE_ROW_FONT);
    profileNameLabel.setJustificationType (juce::Justification::centredLeft);
    profileNameLabel.setColour (juce::Label::textColourId, PRIMARY_TEXT_COLOUR);
    profileNameLabel.setInterceptsMouseClicks (false, false);

    ellipsisButton.setImages (false, true, true, ellipsisImage, 1.0f, juce::Colours::white.withAlpha (0.0f), ellipsisImage, 1.0f, juce::Colours::black.withAlpha (0.1f), ellipsisImage, 1.0f, juce::Colours::black.withAlpha (0.2f));
    ellipsisButton.onClick = [this] {
        if (listener != nullptr)
        {
            listener->profileRowOptionsClicked (this->rowNumber);
//            listener->profileRowClicked (rowNumber);
        }
    };

    addAndMakeVisible (renameEditor);
    renameEditor.setFont (PROFILE_ROW_FONT);
    renameEditor.setJustification (juce::Justification::left);
    renameEditor.setColour (juce::Label::textColourId, PRIMARY_TEXT_COLOUR);
    renameEditor.setColour (juce::TextEditor::backgroundColourId, BACKGROUND_COLOUR.withAlpha(0.0f));
    renameEditor.setColour (juce::TextEditor::outlineColourId, BACKGROUND_COLOUR.withAlpha(0.0f));
    renameEditor.setColour (juce::TextEditor::focusedOutlineColourId, PRIMARY_TEXT_COLOUR.withAlpha(0.0f));
    renameEditor.setColour (juce::Label::outlineWhenEditingColourId, PRIMARY_TEXT_COLOUR.withAlpha(0.0f));
    renameEditor.setColour (juce::TextEditor::shadowColourId, juce::Colours::white.withAlpha (0.0f));
    renameEditor.setTextToShowWhenEmpty ("Profile name...", SECONDARY_TEXT_COLOUR);
    renameEditor.setSelectAllWhenFocused (true);

    renameEditor.setInterceptsMouseClicks (false, false);
    renameEditor.setMultiLine (false);
    renameEditor.setVisible (false);
    renameEditor.addListener (this);
}

void ProfileRow::paint (juce::Graphics& g)
{
    if (isSelected)
        g.fillAll (HOVER_COLOUR);
    else if (isHovering)
        g.fillAll (HOVER_COLOUR);
    else
        g.fillAll (BACKGROUND_COLOUR);

    if (isEditing)
        renameEditor.grabKeyboardFocus();
}

void ProfileRow::resized()
{
    Layout layout (getBounds().withX (0).withY (0), 8.0f);
    layout.addRow ({ Space(&profileNameLabel, &renameEditor), Space (&ellipsisButton, 20.0f) });
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

void ProfileRow::mouseDoubleClick (const juce::MouseEvent& event)
{
    if (listener != nullptr)
        listener->tryToSetIsEditing (rowNumber);
}

void ProfileRow::setProfileName (const juce::String& profileName)
{
    this->profileName = profileName;
    profileNameLabel.setText (profileName, juce::dontSendNotification);
    renameEditor.setText (profileName);
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
    profileNameLabel.setVisible (! isEditing);
    renameEditor.setWantsKeyboardFocus (isEditing);
    if (isEditing)
    {
        renameEditor.grabKeyboardFocus();
    }
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
    if (listener != nullptr)
    {
        listener->profileRowRenameCancelled (rowNumber);
    }
    setIsEditing (false);
}

void ProfileRow::setListener (ProfileRowListener* listener)
{
    this->listener = listener;
}

void ProfileRow::focusLost (FocusChangeType cause)
{
    std::cout << "focusLost" << std::endl;
    if (cause == FocusChangeType::focusChangedByMouseClick)
    {
        std::cout << "focus changed by mouse click for ProfileRow (" << rowNumber << ")" << std::endl;
    }
    else if (cause == FocusChangeType::focusChangedByTabKey)
    {
        std::cout << "focus changed by tab key for ProfileRow (" << rowNumber << ")" << std::endl;
    }
    else if (cause == FocusChangeType::focusChangedDirectly)
    {
        std::cout << "focus changed directly for ProfileRow (" << rowNumber << ")" << std::endl;
    }
}

void ProfileRow::grabRenameEditorKeyboardFocus()
{
    std::cout << "grabRenameEditorKeyboardFocus" << std::endl;
    renameEditor.grabKeyboardFocus();
}
