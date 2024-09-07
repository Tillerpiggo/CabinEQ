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
#include "CabinEqGraph.h"

class CabinEqPage   : public juce::Component,
                      public juce::Slider::Listener,
                      public juce::ComboBox::Listener,
                      public juce::TextEditor::Listener,
                      public juce::Button::Listener,
                      public CabinEqGraph::Listener,
                      public CabinEqAudioProcessor::Listener,
                      public juce::Timer
{
public:
    CabinEqPage (CabinEqAudioProcessor& p);
    ~CabinEqPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // CabinEQGraphListener methods
    int addCurvePt (float freq, float ampl, CabinEqGraph* sender) override;
    void updateCurvePt (int id, float freq, float ampl, CabinEqGraph* sender) override;
    void removeCurvePt (int id, CabinEqGraph* sender) override;
    void startPlayingValueAt (float freq, CabinEqGraph* sender) override;
    void updatePlayingValueAt (float freq, CabinEqGraph* sender) override;
    void testValueAt (float freq) override;
    void stopPlaying() override;
    void stopTesting() override;
    float getCurrPlayingFreq() override;
    float getCurrTestingFreq() override;
    void userStoppedDoingShit() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void textEditorTextChanged (juce::TextEditor& textEditor) override;
    void textEditorReturnKeyPressed (juce::TextEditor& textEditor) override;
    void textEditorEscapeKeyPressed (juce::TextEditor& textEditor) override;
    void textEditorFocusLost (juce::TextEditor& textEditor) override;
    void comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged) override;
    void inputAttemptWhenModal() override;
    
    void buttonClicked (juce::Button *button) override;
    
    void didLoadData() override;
    
    void timerCallback() override;
    
protected:
    void flagFilterChanged();
    void toggleBypass();
    void toggleBlind();
    void applyFilter();
    void loadDropdownOptions();
    void dismissAlertWindow();
    void updateButtonText();
    
    void showForm();
    void unlockApp();
    
    void goToProfileWithId (juce::String profileIdToGoTo);
    
    bool isDuplicateProfileName (juce::String profileName);
    
    CabinEqAudioProcessor& processor;
    juce::String profileId;
    juce::TextButton bypassButton { "ON" };
    juce::TextButton applyButton { "APPLY" };
    juce::TextButton blindButton { "BLIND" };
    bool isBypassed = false;
    bool isBlind = false;
    bool hasFilterChanged = true;
    bool creatingDuplicate = false;
    int fftSize = 16;
    
    juce::TabbedComponent graphs;
    std::unique_ptr<CabinEqGraph> amplGraph;
    std::unique_ptr<CabinEqGraph> panGraph;
    std::unique_ptr<CabinEqGraph> phaseGraph;
    juce::ComboBox profileDropdown;
    juce::ComboBox filterQualityDropdown;
    juce::Slider referenceSlider;
    std::unique_ptr<juce::AlertWindow> alertWindow;
    
    const juce::Colour backgroundColor = juce::Colour::fromRGB (0.4, 0.4, 0.4);
    const juce::String textEditorName = "ProfileEditor";
    
    int lastSelectedId = 1;
    
    juce::Slider dryVolumeSlider;
    juce::Slider wetVolumeSlider;
    juce::Label dryVolumeLabel;
    juce::Label wetVolumeLabel;
 
    bool isUnlocked = false;
};
