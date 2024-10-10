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
                      public CabinEqAudioProcessor::Listener
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
    void stopPlaying() override;
    void setPatternSolo (bool solo) override;
    float getCurrPlayingFreq() override;
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
    
//    static const juce::Font getCabinFont()
//    {
//        static auto typeface = juce::Typeface::createSystemTypefaceFor (juce::BinaryData::CustomFont, juce::BinaryData::CustomFont_size);
//        return Font (typeface);
//    }
    
protected:
    void flagFilterChanged();
    void toggleBypass();
    void applyFilter();
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
    bool hasFilterChanged = true;
    
    bool creatingDuplicate = false;
    bool renamingProfile = false;
    int fftSize = 16;
    
//    juce::TabbedComponent graphs;
    std::unique_ptr<CabinEqGraph> amplGraph;
//    std::unique_ptr<CabinEqGraph> panGraph;
//    std::unique_ptr<CabinEqGraph> phaseGraph;
    juce::ComboBox profileDropdown;
    juce::ComboBox filterQualityDropdown;
    std::unique_ptr<juce::AlertWindow> alertWindow;
    
    const juce::Colour backgroundColor = juce::Colour::fromRGB (0.4, 0.4, 0.4);
    const juce::String textEditorName = "ProfileEditor";
    
    int lastSelectedId = 1;
    
    juce::Slider volumeSlider;
 
    bool isUnlocked = false;
    bool addingFirstProfile = false;
};
