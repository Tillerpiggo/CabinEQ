/*
  ==============================================================================

    ProfileList.h
    Created: 21 Feb 2025 9:30:33pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqAudioProcessor.h"
#include "Theme.h"

/// The sidebar of profiles. Click one to use it, double-click to rename it, and
/// right-click (or use its "..." button) to duplicate, export or delete it.
class ProfileList : public juce::Component
{
public:
    explicit ProfileList (CabinEqAudioProcessor& processor);
    ~ProfileList() override;

    /// Rebuilds the rows. Call whenever the profiles change.
    void refresh();

    // The page does the file and clipboard work
    std::function<void()> onImportFile, onPaste;
    std::function<void (const juce::String& profileName)> onExportFile, onCopy;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class Row;
    class AddButton;

    void showRowMenu (const juce::String& profileName, juce::Component& target);
    void showAddMenu();
    void addProfile();
    void confirmDelete (const juce::String& profileName);
    void startRenaming (const juce::String& profileName);

    CabinEqAudioProcessor& processor;
    juce::Viewport viewport;
    juce::Component rowHolder;
    juce::OwnedArray<Row> rows;
    std::unique_ptr<AddButton> addButton;
    juce::String pendingRename;

    static constexpr int rowHeight = 36;
    static constexpr int headerHeight = 52;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProfileList)
};
