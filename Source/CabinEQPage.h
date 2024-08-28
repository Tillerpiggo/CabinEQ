/*
  ==============================================================================

    CabinEQPage.h
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EQNode.h"
#include "CabinEQGraph.h"

class CabinEQPage   : public juce::Component,
                      public juce::Slider::Listener,
                      public juce::ComboBox::Listener,
                      public juce::TextEditor::Listener,
                      public juce::Button::Listener,
                      public CabinEQGraph::Listener,
                      public StartupMVPAudioProcessor::Listener,
                      public juce::Timer
{
public:
    CabinEQPage (StartupMVPAudioProcessor& p);
    ~CabinEQPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // CabinEQGraphListener methods
    int addCurvePt (float freq, float ampl, CabinEQGraph* sender) override;
    void updateCurvePt (int id, float freq, float ampl, CabinEQGraph* sender) override;
    void removeCurvePt (int id, CabinEQGraph* sender) override;
    void startPlayingValueAt (float freq, float ampl) override;
    void playValueAt (float freq, float ampl) override;
    void testValueAt (float freq) override;
    void stopPlaying() override;
    void stopTesting() override;
    float getCurrPlayingFreq() override;
    float getCurrTestingFreq() override;
    void userStoppedDoingShit() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
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
    
    StartupMVPAudioProcessor& processor;
    juce::String profileId;
    juce::TextButton bypassButton { "ON" };
    juce::TextButton applyButton { "APPLY" };
    juce::TextButton blindButton { "BLIND" };
    bool isBypassed = false;
    bool isBlind = false;
    bool hasFilterChanged = true;
    bool creatingDuplicate = false;
    int fftSize = 16;
    
    CabinEQGraph cabinEQGraph;
    juce::ComboBox profileDropdown;
    juce::ComboBox filterQualityDropdown;
    juce::Slider referenceSlider;
    std::unique_ptr<juce::AlertWindow> alertWindow;
    
    const juce::Colour backgroundColor = juce::Colour::fromRGB (0.4, 0.4, 0.4);
    const juce::String textEditorName = "ProfileEditor";
    
    int lastSelectedId = 1;
    
    juce::Slider dryVolumeSlider;
    juce::Slider wetVolumeSlider;
 
    bool isUnlocked = false;
};
