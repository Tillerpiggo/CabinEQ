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
#include "CabinEqLookAndFeel.h"
#include "CalibrationView.h"
#include "FreeTrialBanner.h"
#include "FreeTrialLockScreen.h"
#include "ContactUsBanner.h"
#include "Listeners.h"
#include "CabinEqMarketplaceStatus.h"
#include "CabinEqUnlockForm.h"
#include "ProfileView.h"

class CabinEqPage   : public BuildableComponent,
                      public juce::ComboBox::Listener,
                      public juce::TextEditor::Listener,
                      public CabinEqAudioProcessor::Listener,
                      public FreeTrialListener,
                      public juce::Timer
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
    void unlockApp(); // unlocks the app, hiding the free trial banner and free trial lock screen
    void lockApp(); // locks the app, showing the free trial banner and free trial lock screen
    
    void lockIfNecessary();
    
    void addProfile();
    void duplicateProfile();
    void renameProfile();
    
    void goToProfileWithId (juce::String profileIdToGoTo);
    bool isDuplicateProfileName (juce::String profileName);
    
    void submitAlertWindowText(); // tries to add, rename, or duplicate the profile based on the text in the textEditor
    
    
    CabinEqLookAndFeel cabinEqLookAndFeel;
    
    // JUCE Labels
    FreeTrialBanner freeTrialBanner;
    ContactUsBanner contactUsBanner;
    FreeTrialLockScreen freeTrialLockScreen;
    CalibrationView calibrationView;
    juce::Label cabinEQLabel;
    CabinEqAudioProcessor& processor;
    juce::String profileId;
    juce::TextButton bypassButton { "ON" };
    
    bool isBypassed = false;
    
    // Free Trial Unlock
//    CabinEqMarketplaceStatus marketplaceStatus;
    CabinEqUnlockForm unlockForm;
    
    bool creatingDuplicate = false;
    bool renamingProfile = false;
    int fftSize = 16;
    
    std::unique_ptr<CabinPeqGraph> amplGraph;
    juce::ComboBox profileDropdown;
    juce::TextButton addProfileButton { "+ New" };
    juce::TextButton duplicateProfileButton { "Duplicate" };
    juce::TextButton renameProfileButton { "Rename" };
    // ProfileView profileView;
    
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
