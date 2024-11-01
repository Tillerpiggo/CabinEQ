/*
  ==============================================================================

    CabinEqPage.h
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "CabinEqAudioProcessor.h"
#include "CabinPeqGraph.h"
#include "StepView.h"

class CabinEqPage   : public BuildableComponent,
                      public juce::ComboBox::Listener,
                      public juce::TextEditor::Listener,
                      public CabinPeqGraph::Listener,
                      public CabinEqAudioProcessor::Listener,
                      public CabinPeqGraph::DataSource,
                      public StepView::Listener
{
public:
    CabinEqPage (CabinEqAudioProcessor& p);
    ~CabinEqPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // CabinPeqGraph::Listener methods
    int addBand (float freq, float ampl, float bandwidth, CabinPeqGraph* sender) override;
    void updateBand (int id, float freq, float ampl, float bandwidth, CabinPeqGraph* sender) override;
    void removeBand (int id, CabinPeqGraph* sender) override;
    void setVolume (float volume, CabinPeqGraph* sender) override;
    
    // CabinPeqGraph::DataSource methods
    BandProfile getBandProfile() override;
    float getCurrPlayingFreq() override;
    
    // StepView Listener methods
    void setDifficulty (float difficulty) override;
    void setIsPlaying (bool isPlaying) override;
    void goToNextStep() override;
    void goToPrevStep() override;
    
    // Text editor stuff
    void textEditorTextChanged (juce::TextEditor& textEditor) override;
    void textEditorReturnKeyPressed (juce::TextEditor& textEditor) override;
    void textEditorEscapeKeyPressed (juce::TextEditor& textEditor) override;
    void textEditorFocusLost (juce::TextEditor& textEditor) override;
    void comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged) override;
    void inputAttemptWhenModal() override;
    
    void didLoadData() override;
    
protected:
    void toggleBypass();
    void loadDropdownOptions();
    void dismissAlertWindow();
    void updateButtonText();
    
    void showForm();
    void unlockApp();
    
    void goToProfileWithId (juce::String profileIdToGoTo);
    bool isDuplicateProfileName (juce::String profileName);
    
    // JUCE Labels
    juce::Label cabinEQLabel;
    CabinEqAudioProcessor& processor;
    juce::String profileId;
    juce::TextButton bypassButton { "ON" };
    bool isBypassed = false;
    
    bool creatingDuplicate = false;
    bool renamingProfile = false;
    int fftSize = 16;
    
    std::unique_ptr<CabinPeqGraph> amplGraph;
    StepView stepView;
    juce::ComboBox profileDropdown;
    juce::ComboBox filterQualityDropdown;
    std::unique_ptr<juce::AlertWindow> alertWindow;
    
    const juce::Colour backgroundColor = juce::Colour::fromRGB (0.4, 0.4, 0.4);
    const juce::String textEditorName = "ProfileEditor";
    
    int lastSelectedId = 1;
    int lastSelectedNodeIdForCalibration = 0;
    
    juce::Slider masterVolumeSlider; // controls master volume for all sound, whether processing or not, including calibration volume
    juce::Label masterVolumeSliderLabel;
    
    bool playingNoisePattern = false;
    
    bool isUnlocked = false;
    bool addingFirstProfile = false;
};
