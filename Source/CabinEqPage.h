/*
  ==============================================================================

    CabinEqPage.h
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqAudioProcessor.h"
#include "CabinPeqGraph.h"
#include "BandInspector.h"
#include "ProfileList.h"
#include "ValueField.h"
#include "CalibrationPanel.h"

/// The whole window: profiles down the left, the top bar, the graph, and the band inspector.
class CabinEqPage   : public juce::Component,
                      public juce::FileDragAndDropTarget,
                      private juce::ChangeListener,
                      private juce::Timer
{
public:
    explicit CabinEqPage (CabinEqAudioProcessor& p);
    ~CabinEqPage() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void fileDragEnter (const juce::StringArray&, int, int) override;
    void fileDragExit (const juce::StringArray&) override;
    void filesDropped (const juce::StringArray& files, int, int) override;

private:
    class IconButton;

    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    void timerCallback() override;
    void refreshAll();
    void updateInspector();
    void updateTopBar();

    void importFile();
    void importFiles (const juce::Array<juce::File>& files);
    void importText (const juce::String& text, const juce::String& profileName);
    void exportProfile (const juce::String& profileName);
    void copyProfile (const juce::String& profileName);
    void pasteProfile();
    void showCrossfeed();
    void showMessage (const juce::String& title, const juce::String& message);
    void setCalibrationShown (bool shouldShow);

    CabinEqAudioProcessor& processor;

    ProfileList profileList;
    CabinPeqGraph graph;
    BandInspector inspector;
    CalibrationPanel calibrationPanel;

    std::unique_ptr<IconButton> undoButton, redoButton, calibrationButton, crossfeedButton, settingsButton, powerButton;
    ValueField preampField { "Preamp", -30.0, 30.0, 0.0 };
    juce::ToggleButton autoGainToggle { "Auto gain" };
    juce::AudioProcessorValueTreeState::ButtonAttachment autoGainAttachment;

    juce::TooltipWindow tooltipWindow { this, 600 };
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::Component::SafePointer<juce::CallOutBox> crossfeedBox;
    bool isDraggingFiles = false;
    bool grewForCalibration = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabinEqPage)
};
