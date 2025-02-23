/*
  ==============================================================================

    CabinEqPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqPage.h"

CabinEqPage::CabinEqPage (CabinEqAudioProcessor& p)
    : processor (p), profileId ("NO_PROFILE"), unlockForm (p.getMarketplaceStatus())
{
    p.getMarketplaceStatus().load();
    isUnlocked = p.getHasLicense();

    profileView.setListener (this);
    profileView.setDataSource (&processor);

    amplGraph = std::make_unique<CabinPeqGraph>();
    amplGraph->setListener (&processor);
    amplGraph->addDataSource (&processor);
    amplGraph->toBack();

    calibrationView.setListener (&processor);
    calibrationView.setCalibrationListener (&processor);
    calibrationView.setDataSource (&processor);
    calibrationView.setElevationCalibrationListener (&processor);
    calibrationView.setElevationCalibrationDataSource (&processor);

    freeTrialBanner.setListener (this);
    contactUsBanner.setListener (&processor);

    gearButton.setButtonText ("Advanced...");
    gearButton.onClick = [this] {
        processor.showAudioSettingsDialog();
    };
    
    // Buttons
    addButton (&bypassButton);
    
    // Slider Actions
    addSliderAction (&masterVolumeSlider, [this](juce::Slider* slider) {
        processor.setVolume (slider->getValue());
    });
    // Button Actions
    addButtonAction (&bypassButton, [this](juce::Button* button) {
        toggleBypass();
        processor.setIsFilterOn (! isBypassed);
    });
    
    // profileDropdown.addListener (this);
    processor.addListener (this);
    
    // Extra stuff, will clean up later
    if (! isUnlocked)
    {
        addAndMakeVisible (freeTrialBanner);
        addAndMakeVisible (freeTrialLockScreen);
    }
    
    addAndMakeVisible (contactUsBanner);
    addAndMakeVisible (amplGraph.get());
    addAndMakeVisible (profileView);
    addAndMakeVisible (inputDropdown);
    addAndMakeVisible (outputDropdown);
    addAndMakeVisible (gearButton);
    addAndMakeVisible (calibrationView);
    addAndMakeVisible (unlockForm);

    unlockForm.setVisible (false);
    
    didLoadData();
    startTimer (100);
    
    bypassButton.setColour (juce::TextButton::buttonColourId, juce::Colours::blueviolet);
    
    setLookAndFeel (&cabinEqLookAndFeel);
}

CabinEqPage::~CabinEqPage()
{
    setLookAndFeel (nullptr);
    // profileDropdown.removeListener (this);
    bypassButton.removeListener (this);
    masterVolumeSlider.removeListener (this);

    amplGraph->removeListener();

    processor.removeListener();
    
    stopTimer();
}

void CabinEqPage::paint (juce::Graphics& g)
{
    g.setColour (BACKGROUND_COLOUR);
    g.fillRect (getLocalBounds());
}

void CabinEqPage::resized()
{
    float profileViewWidth = 240.0f;
    
    // ProfileView
    Layout profileViewLayout (getBounds().withTrimmedRight (getWidth() - profileViewWidth), 0.0f);
    profileViewLayout.addRow ({ Space(&profileView) });
    profileViewLayout.updateComponentBounds();
    
    // Everything else
    Layout layout (getBounds().withTrimmedLeft (profileViewWidth), 0.0f);
    if (! processor.getHasLicense())
    {
        layout.addRow ({ Space (&freeTrialBanner) }, 40);
    }
    
    int topRowHeight = 40;
    
    float gearButtonWidth = 120.0f;
    layout.addRow ({ Space (&bypassButton, 60), Space(), Space (&gearButton) }, (float) topRowHeight);
    layout.addRow ({ Space (amplGraph.get(), &freeTrialLockScreen) }, 0.5);
    layout.addRow ({ Space (&calibrationView) });
    layout.addRow ({ Space (&contactUsBanner) }, 40);
    layout.updateComponentBounds();
    
    unlockForm.centreWithSize (getWidth() * 0.8, getHeight() * 0.8);
    
//    Layout onButtonLayout (getBounds().withTrimmedLeft (getWidth() - sidebarWidth), 0.0f);
//    onButtonLayout.addRow ({ Space (&bypassButton) });
//    onButtonLayout.addRow ({ Space() }, 0.5);
//    onButtonLayout.updateComponentBounds();
}

// ====================================================
void CabinEqPage::textEditorTextChanged (juce::TextEditor& textEditor)
{
    // Check if the text is a duplicate. If it is, add a warning on the alert window
    auto text = textEditor.getText();
    bool isDuplicate = isDuplicateProfileName (text);
    if (renamingProfile && text == profileId) // To be less annoying, let people rename a profile to itself
        isDuplicate = false;
    std::string message = (renamingProfile || creatingDuplicate) ? "Enter new name" : "Enter profile name";
    alertWindow->setMessage (isDuplicate ? "This profile name is already taken!" :
                                           message);
    
    // Update alert window
    bool canAddProfile = ! isDuplicate && ! text.isEmpty();
    alertWindow->getButton (1)->setEnabled (canAddProfile);
}

void CabinEqPage::textEditorReturnKeyPressed (juce::TextEditor& textEditor)
{
//    submitAlertWindowText();
}

void CabinEqPage::textEditorEscapeKeyPressed (juce::TextEditor& textEditor)
{
    dismissAlertWindow();
}

void CabinEqPage::textEditorFocusLost (juce::TextEditor& textEditor)
{
    dismissAlertWindow();
}

void CabinEqPage::inputAttemptWhenModal()
{
    dismissAlertWindow();
}

void CabinEqPage::freeTrialDidReset()
{
    if (! processor.getHasLicense())
    {
        bypassButton.setButtonText ("OFF");
        processor.freeTrialDidReset();
        lockIfNecessary();
        freeTrialLockScreen.setVisible (processor.isProfileLocked());
    }
}

void CabinEqPage::showActivateLicenseForm()
{
    showForm();
}

// ProfileViewListener methods
void CabinEqPage::addProfile (juce::String profileName)
{
    processor.addProfile (profileName);
}

void CabinEqPage::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    processor.addDuplicateProfile (profileName, oldProfileName);
}

void CabinEqPage::renameProfile (juce::String profileName, juce::String newProfileName)
{
    processor.renameProfile (profileName, newProfileName);
}

void CabinEqPage::deleteProfile (juce::String profileName)
{
    processor.removeProfile (profileName);
}

void CabinEqPage::selectProfile (juce::String profileName)
{
    goToProfileWithId (profileName);
}

void CabinEqPage::didLoadData()
{
    addingFirstProfile = true;
    processor.setIsFilterOn (! isBypassed && ! processor.isProfileLocked());
    auto lastSelectedProfileName = processor.getLastSelectedProfileName();
    if (lastSelectedProfileName.has_value())
    {
        goToProfileWithId (lastSelectedProfileName.value());
        int lastSelectedId = 0;
        for (int i = 0; i < profileView.getProfileNames().size(); i++)
        {
            if (profileView.getProfileNames()[i] == lastSelectedProfileName.value())
            {
                lastSelectedId = i;
            }
        }
        profileView.setProfileSelected (lastSelectedId);
    }
    
    // If there are no profiles, add one
    if (processor.getProfileNames().size() == 0)
    {
        juce::String firstProfileId = "My Headphone Profile";
        processor.addProfile (firstProfileId);
        goToProfileWithId (firstProfileId);
    }
    
    lockIfNecessary(); // locks the filter/graph if you don't have a license and the profile is locked
    
    masterVolumeSlider.setValue (processor.getMasterVolume(), juce::sendNotification);
    
    resized();
    
//    multiBandStepBar.updateBandProfile (processor.getBandProfile());
}

//=========================================
void CabinEqPage::toggleBypass()
{
    isBypassed = ! isBypassed;
    if (processor.isProfileLocked())
        isBypassed = true;
    amplGraph->setGrayscale (isBypassed);
    processor.setIsFilterOn (! isBypassed);
    updateButtonText();
}

void CabinEqPage::dismissAlertWindow()
{
    alertWindow->getTextEditor (textEditorName)->removeListener (this);
    alertWindow.reset();
    creatingDuplicate = false;
    renamingProfile = false;
}
                                
void CabinEqPage::updateButtonText()
{
    if (isBypassed)
    {
        bypassButton.setButtonText ("OFF");
        bypassButton.setColour (juce::TextButton::buttonColourId, juce::Colours::grey);
    }
    else
    {
        bypassButton.setButtonText ("ON");
        bypassButton.setColour (juce::TextButton::buttonColourId, juce::Colours::blueviolet);
    }
}

void CabinEqPage::showForm()
{
    unlockForm.setVisible (true);
}

void CabinEqPage::unlockApp()
{
    processor.setHasLicense (true);
    processor.getMarketplaceStatus().save();

    amplGraph->setGrayscale (isBypassed);
    processor.setIsFilterOn (! isBypassed);
    updateButtonText();

    freeTrialBanner.setVisible (false);
    freeTrialLockScreen.setVisible (false);
    resized();
}

void CabinEqPage::lockApp()
{
    processor.setHasLicense (false);
    amplGraph->setGrayscale (true);
    processor.setIsFilterOn (false);
    updateButtonText();
    
    freeTrialBanner.setVisible (true);
    freeTrialLockScreen.setVisible (true);
    resized();
}

void CabinEqPage::timerCallback()
{
    if (processor.getHasLicense() && ! isUnlocked)
    {
        unlockApp();
        isUnlocked = true;
    }
    
    // * UNTESTED *
    if (! processor.getHasLicense() && isUnlocked)
    {
        lockApp();
        isUnlocked = false;
    }
}

void CabinEqPage::lockIfNecessary()
{
    if (! processor.isProfileLocked())
        return;
    
    bypassButton.setButtonText ("OFF");
    amplGraph->setGrayscale (true);
    processor.setIsFilterOn (false);
    // loadDropdownOptions();
}

void CabinEqPage::addProfile()
{
    // Create and present an alert for the user to enter the profile name into
    alertWindow = std::make_unique<juce::AlertWindow> ("Add Profile", "Enter profile name", juce::MessageBoxIconType::NoIcon);
    creatingDuplicate = false;
    
    alertWindow->addTextEditor (textEditorName, "");
    alertWindow->getTextEditor (textEditorName)->addListener (this);
    alertWindow->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    alertWindow->addButton ("Add", 1, juce::KeyPress (juce::KeyPress::returnKey));
    alertWindow->setEscapeKeyCancels (true);
    
    alertWindow->getButton (0)->onClick = [this] {
        dismissAlertWindow();
    };
    alertWindow->getButton (1)->onClick = [this] {
        submitAlertWindowText();
    };
    
    alertWindow->enterModalState();
    alertWindow->getTextEditor (textEditorName)->grabKeyboardFocus();
    
    // profileDropdown.setSelectedId (lastSelectedId);
}

void CabinEqPage::duplicateProfile()
{
//    auto copyName = profileId + " copy";
//    while (isDuplicateProfileName (copyName))
//    {
//        copyName += " copy";
//    }
//    
//    processor.addDuplicateProfile (copyName, profileId);
//    
//    // Select the new profile and go to it
//    profileDropdown.setSelectedId (profileDropdown.getItemId (profileDropdown.getNumItems() - 2));
//    goToProfileWithId (copyName);
//    loadDropdownOptions();
//    
    // Present option to add duplicate profile, and opportunity to name it
    // Create a present an alert for the user to enter the profile name into
    alertWindow = std::make_unique<juce::AlertWindow> ("Duplicate \"" + profileId + "\"", "Enter new name", juce::MessageBoxIconType::NoIcon);
    creatingDuplicate = true;
    
    // Create copy name
    auto copyName = profileId + " copy";
    
    while (isDuplicateProfileName (copyName))
    {
        copyName += " copy";
    }
    
    alertWindow->addTextEditor (textEditorName, copyName);
    alertWindow->getTextEditor (textEditorName)->addListener (this);
    alertWindow->setEscapeKeyCancels (true);
    
    alertWindow->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    alertWindow->addButton ("Duplicate", 1, juce::KeyPress (juce::KeyPress::returnKey));
    alertWindow->setEscapeKeyCancels (true);
    
    alertWindow->getButton (0)->onClick = [this] {
        dismissAlertWindow();
    };
    alertWindow->getButton (1)->onClick = [this] {
        submitAlertWindowText();
    };
    
    alertWindow->enterModalState();
    alertWindow->getTextEditor (textEditorName)->grabKeyboardFocus();
    
//    profileDropdown.setSelectedId (lastSelectedId);
}

void CabinEqPage::renameProfile()
{
    // Create and present alert to rename profile
    alertWindow = std::make_unique<juce::AlertWindow> ("Rename \"" + profileId + "\"", "Enter new name", juce::MessageBoxIconType::NoIcon);
    renamingProfile = true;
    
    alertWindow->addTextEditor (textEditorName, profileId);
    alertWindow->getTextEditor (textEditorName)->addListener (this);
    alertWindow->setEscapeKeyCancels (true);
    
    alertWindow->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    alertWindow->addButton ("Rename", 1, juce::KeyPress (juce::KeyPress::returnKey));
    alertWindow->setEscapeKeyCancels (true);
    
    alertWindow->getButton (0)->onClick = [this] {
        dismissAlertWindow();
    };
    alertWindow->getButton (1)->onClick = [this] {
        submitAlertWindowText();
    };
    
    alertWindow->enterModalState();
    alertWindow->getTextEditor (textEditorName)->grabKeyboardFocus();
    
//    profileDropdown.setSelectedId (lastSelectedId);
}

void CabinEqPage::goToProfileWithId (juce::String profileIdToGoTo)
{
    profileId = profileIdToGoTo;
    processor.setLastSelectedProfileName (profileId);
    
    BandProfile bandProfile = processor.getBandProfile();
    amplGraph->setBandProfile (bandProfile);
    // profileDropdown.setText (profileIdToGoTo);
    processor.updateFilter();
    if (! processor.isProfileLocked())
    {
        processor.setIsFilterOn (! isBypassed);
        amplGraph->setGrayscale (isBypassed);
        updateButtonText();
    }
    else
    {
        lockIfNecessary();
    }
        
    freeTrialLockScreen.setVisible (processor.isProfileLocked());
}

bool CabinEqPage::isDuplicateProfileName (juce::String profileName)
{
    auto profileNames = processor.getProfileNames();
    
    for (const auto& existingProfileName : profileNames)
        if (profileName == existingProfileName)
            return true;
    
    return false;
}

void CabinEqPage::submitAlertWindowText()
{
    std::cout << "submit alert window text" << std::endl;
    auto textEditor = alertWindow->getTextEditor (textEditorName);
    if (textEditor == nullptr)
        return;
    
    if (textEditor->getText().isEmpty())
    {
        dismissAlertWindow();
        return;
    }
    
    // This is handled by the button being enabled/disabled
//    // Check if the text is a duplicate. If it is, don't add or do anything
//    if (isDuplicateProfileName (textEditor->getText()))
//        return;
    
    // If we're renaming the profile to a new name, change the name of the profile
    if (renamingProfile)
    {
        juce::String text = textEditor->getText();
        processor.renameProfile (profileId, text);
        profileId = text;
        goToProfileWithId (text);
        renamingProfile = false;
    }
    
    // Add & retrieve profile from processor
    juce::String profileName = textEditor->getText();
    if (! creatingDuplicate)
    {
        processor.addProfile (profileName);
    }
    else
    {
        processor.addDuplicateProfile (profileName, profileId);
        creatingDuplicate = false;
    }
    
    // loadDropdownOptions();
    
    // Select the new profile and go to it
    // profileDropdown.setSelectedId (profileDropdown.getItemId (profileDropdown.getNumItems() - 2));
    goToProfileWithId (profileName);
    // loadDropdownOptions();
    
    // Finally, dismiss the window
    dismissAlertWindow();
}
