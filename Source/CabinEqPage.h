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

class CabinEqPage   : public juce::Component,
                      public juce::Slider::Listener,
                      public juce::ComboBox::Listener,
                      public juce::TextEditor::Listener,
                      public juce::Button::Listener,
                      public CabinPeqGraph::Listener,
                      public CabinEqAudioProcessor::Listener,
                      public CabinPeqGraph::DataSource
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
    void startNoisePatternAt (int id, CabinPeqGraph* sender) override;
    void updateNoisePatternAt (int id, CabinPeqGraph* sender) override;
    void stopNoisePattern() override;
    void setNoisePatternSolo (bool solo) override;
    void setVolume (float volume, CabinPeqGraph* sender) override;
    
    // CabinPeqGraph::DataSource methods
    BandProfile getBandProfile() override;
    float getCurrPlayingFreq() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void sliderDragStarted (juce::Slider *slider) override;
    void sliderDragEnded (juce::Slider *slider) override;
    void textEditorTextChanged (juce::TextEditor& textEditor) override;
    void textEditorReturnKeyPressed (juce::TextEditor& textEditor) override;
    void textEditorEscapeKeyPressed (juce::TextEditor& textEditor) override;
    void textEditorFocusLost (juce::TextEditor& textEditor) override;
    void comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged) override;
    void inputAttemptWhenModal() override;
    
    void buttonClicked (juce::Button *button) override;
    
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
    juce::ComboBox profileDropdown;
    juce::ComboBox filterQualityDropdown;
    std::unique_ptr<juce::AlertWindow> alertWindow;
    
    const juce::Colour backgroundColor = juce::Colour::fromRGB (0.4, 0.4, 0.4);
    const juce::String textEditorName = "ProfileEditor";
    
    int lastSelectedId = 1;
    int lastSelectedNodeIdForCalibration = 0;
    
    juce::Slider masterVolumeSlider; // controls master volume for all sound, whether processing or not, including calibration volume
    juce::Slider calibrationVolumeSlider; // controls calibration volume, relative to master volume
    juce::Slider spacingSlider; // controls spacing between the 3 noise patterns
    juce::Slider bandwidthSlider; // controls the bandwidths of the noise patterns
    
    juce::Label masterVolumeSliderLabel;
    juce::Label calibrationVolumeSliderLabel;
    juce::Label spacingSliderLabel;
    juce::Label bandwidthSliderLabel;
    
    bool isUnlocked = false;
    bool addingFirstProfile = false;
};
