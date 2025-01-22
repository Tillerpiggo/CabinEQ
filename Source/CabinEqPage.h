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
#include "GlyphView.h"
#include "KnobView.h"
#include "CabinEqLookAndFeel.h"
#include "NoiseGridView.h"
#include "CalibrationView.h"
#include "MultiBandStepBar.h"
#include "FreeTrialBanner.h"
#include "FreeTrialLockScreen.h"
#include "Listeners.h"
#include "CabinEqMarketplaceStatus.h"
#include "CabinEqUnlockForm.h"
//#include "MagicKnob.h"

class CabinEqPage   : public BuildableComponent,
                      public juce::ComboBox::Listener,
                      public juce::TextEditor::Listener,
                      public CabinEqAudioProcessor::Listener,
                      public FreeTrialListener,
                      public juce::Timer
//                      public MagicKnob::Listener
{
public:
    CabinEqPage (CabinEqAudioProcessor& p);
    ~CabinEqPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Text editor stuff
    void textEditorTextChanged (juce::TextEditor& textEditor) override;
    void textEditorReturnKeyPressed (juce::TextEditor& textEditor) override;
    void textEditorEscapeKeyPressed (juce::TextEditor& textEditor) override;
    void textEditorFocusLost (juce::TextEditor& textEditor) override;
    void comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged) override;
    void inputAttemptWhenModal() override;
    
    void freeTrialDidReset() override;
    void showActivateLicenseForm() override;
    
//    void setBands (std::vector<Band> bands) override;
    void timerCallback() override;
    
    void didLoadData() override;
    
protected:
    void toggleBypass();
    void loadDropdownOptions();
    void dismissAlertWindow();
    void updateButtonText();
    
    void showForm();
    void unlockApp();
    
    void lockIfNecessary();
    
    void goToProfileWithId (juce::String profileIdToGoTo);
    bool isDuplicateProfileName (juce::String profileName);
    
    CabinEqLookAndFeel cabinEqLookAndFeel;
    
    // JUCE Labels
    FreeTrialBanner freeTrialBanner;
    FreeTrialLockScreen freeTrialLockScreen;
    NoiseGridView noiseGridView;
    CalibrationView calibrationView;
    juce::Label cabinEQLabel;
    CabinEqAudioProcessor& processor;
    juce::String profileId;
    juce::TextButton bypassButton { "ON" };
    bool isBypassed = false;
    
    // Free Trial Unlock
    CabinEqMarketplaceStatus marketplaceStatus;
    CabinEqUnlockForm unlockForm;
    
    bool creatingDuplicate = false;
    bool renamingProfile = false;
    int fftSize = 16;
    
    std::unique_ptr<CabinPeqGraph> amplGraph;
    juce::ComboBox profileDropdown;
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
