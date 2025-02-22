/*
  ==============================================================================

    ProfileRow.h
    Created: 21 Feb 2025 11:28:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "Layout.h"

// Displays a single profile row in the profile list, with ellipsis to show other options
class ProfileRow :  public BuildableComponent,
                    public juce::TextEditor::Listener
{
public:
    class ProfileRowListener
    {
    public:
        virtual ~ProfileRowListener() = default;
        virtual void profileRowClicked (int row) = 0;
        virtual void profileRowOptionsClicked (int row) = 0;
        virtual void profileRowRenamed (int row, juce::String newProfileName) = 0;
    };

    ProfileRow (int rowNumber);
    
    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;
    void mouseDown (const juce::MouseEvent& event) override;

    void setProfileName (const juce::String& profileName);
    void setIsSelected (bool isSelected);
    void setIsHovering (bool isHovering);
    void setIsEditing (bool isEditing);
    void setRowNumber (int rowNumber);

    void textEditorReturnKeyPressed (juce::TextEditor& editor) override;
    void textEditorEscapeKeyPressed (juce::TextEditor& editor) override;

    void setListener (ProfileRowListener* listener);

    void focusLost (FocusChangeType cause) override;

    void grabRenameEditorKeyboardFocus();

private:
    ProfileRowListener* listener = nullptr;
    
    juce::String profileName;
    juce::Label profileNameLabel;
    juce::ImageButton ellipsisButton;

    juce::Image ellipsisImage = juce::ImageFileFormat::loadFrom (BinaryData::EllipsisIcon_png, BinaryData::EllipsisIcon_pngSize);

    juce::TextEditor renameEditor;

    int rowNumber;
    bool isSelected = false;
    bool isHovering = false;
    bool isEditing = false;
};
