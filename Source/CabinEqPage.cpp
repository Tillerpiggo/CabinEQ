/*
  ==============================================================================

    CabinEqPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqPage.h"

CabinEqPage::CabinEqPage (CabinEqAudioProcessor& p)
    : processor (p), profileId ("NO_PROFILE")
     //, graphs (juce::TabbedButtonBar::Orientation::TabsAtTop)
{
    amplGraph = std::make_unique<CabinPeqGraph>();
    
    masterVolumeSlider.setRange (-20.0f, 20.0f);
    masterVolumeSlider.setValue (0.0f);
    masterVolumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    masterVolumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    
    calibrationVolumeSlider.setRange (-20.0f, 20.0f);
    calibrationVolumeSlider.setValue (0.0f);
    calibrationVolumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    calibrationVolumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    
    sineVolumeSlider.setRange (-20.0f, 20.0f);
    sineVolumeSlider.setValue (0.0f);
    sineVolumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    sineVolumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    
//    spacingSlider.setRange (0.0f, 5.0f);
//    spacingSlider.setSliderStyle (juce::Slider::LinearHorizontal);
//    spacingSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 0, spacingSlider.getTextBoxHeight());
//    
//    pitchSlider.setRange (10.0f, 20000.0f);
//    pitchSlider.setSkewFactorFromMidPoint (1000.0f);
//    pitchSlider.setSliderStyle (juce::Slider::LinearHorizontal);
//    pitchSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 0, pitchSlider.getTextBoxHeight());
    
    speedSlider.setRange (0.2f, 5.0f);
    speedSlider.setValue (1.0f);
    speedSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    speedSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 0, pitchSlider.getTextBoxHeight());
    
    masterVolumeSliderLabel.setText ("Volume", juce::dontSendNotification);
    masterVolumeSliderLabel.attachToComponent (&masterVolumeSlider, true);
    
    calibrationVolumeSliderLabel.setText ("Calibration", juce::dontSendNotification);
    calibrationVolumeSliderLabel.attachToComponent (&calibrationVolumeSlider, true);
    
    sineVolumeSliderLabel.setText ("Sines", juce::dontSendNotification);
    sineVolumeSliderLabel.attachToComponent (&sineVolumeSlider, true);
    
//    spacingSliderLabel.setText ("Spacing", juce::dontSendNotification);
//    spacingSliderLabel.attachToComponent (&spacingSlider, true);
//    
//    pitchSliderLabel.setText ("Pitch", juce::dontSendNotification);
//    pitchSliderLabel.attachToComponent (&pitchSlider, true);
    
    speedSliderLabel.setText ("Speed", juce::dontSendNotification);
    speedSliderLabel.attachToComponent (&speedSlider, true);
    
    amplGraph->addListener (this);
    amplGraph->addDataSource (this);
    profileDropdown.addListener (this);
    filterQualityDropdown.addListener (this);
    bypassButton.addListener (this);
    processor.addListener (this);
    masterVolumeSlider.addListener (this);
    calibrationVolumeSlider.addListener (this);
    startStopButton.addListener (this);
    sineVolumeSlider.addListener (this);
//    spacingSlider.addListener (this);
//    pitchSlider.addListener (this);
    speedSlider.addListener (this);
    
    addAndMakeVisible (amplGraph.get());
    addAndMakeVisible (profileDropdown);
    addAndMakeVisible (bypassButton);
    addAndMakeVisible (masterVolumeSlider);
    addAndMakeVisible (masterVolumeSlider);
    addAndMakeVisible (startStopButton);
    addAndMakeVisible (calibrationVolumeSlider);
    addAndMakeVisible (sineVolumeSlider);
//    addAndMakeVisible (spacingSlider);
//    addAndMakeVisible (pitchSlider);
    addAndMakeVisible (speedSlider);
    addAndMakeVisible (masterVolumeSliderLabel);
    addAndMakeVisible (calibrationVolumeSliderLabel);
    addAndMakeVisible (sineVolumeSliderLabel);
//    addAndMakeVisible (spacingSliderLabel);
//    addAndMakeVisible (pitchSliderLabel);
    addAndMakeVisible (speedSliderLabel);
    
    didLoadData();
}

CabinEqPage::~CabinEqPage()
{
    profileDropdown.removeListener (this);
    bypassButton.removeListener (this);
    masterVolumeSlider.removeListener (this);
    
    amplGraph->removeListener();
    
    processor.removeListener();
}

void CabinEqPage::paint (juce::Graphics& g)
{
//    g.fillAll (juce::Colours::lightgrey);
}

void CabinEqPage::resized()
{
    int padding = 12; // padding on the top and bottom
    int componentPadding = 8; // padding between graph, slider, and dropdown
    int dropdownHeight = 30;
    int sliderHeight = 30;
    int labelHeight = 15;
    int buttonWidth = 100;
    int applyButtonWidth = 100;
    int duplicateButtonWidth = 100;
    int toggleButtonHeight = 30;
    int totalButtonWidth = buttonWidth + duplicateButtonWidth + applyButtonWidth;

    // Get heights for each component
    int availableHeight = getHeight();
    int graphHeight = availableHeight - dropdownHeight * 4 - sliderHeight - toggleButtonHeight - 4 * componentPadding;
    
    // Get widths for each component
    int dropdownWidth = getWidth() - (2 * padding) - totalButtonWidth;
    int profileDropdownWidth = static_cast<int>(dropdownWidth * 0.75); // 75% width
    int filterQualityDropdownWidth = dropdownWidth - profileDropdownWidth; // Remaining 25% width

    int buttonsY = padding + graphHeight + componentPadding;

    // Set bounds for ampl graph to fill most of the space
    amplGraph->setBounds (0, padding, getWidth(), graphHeight);
    
    // Row of buttons beneath the graph
    profileDropdown.setBounds (padding, buttonsY, profileDropdownWidth, dropdownHeight);
    filterQualityDropdown.setBounds (padding + profileDropdownWidth, buttonsY, filterQualityDropdownWidth, dropdownHeight);
    bypassButton.setBounds (getWidth() - buttonWidth - padding, buttonsY, buttonWidth, dropdownHeight);

    // Set bounds for sliders
    int labelWidth = 60;
    int sliderY = buttonsY + dropdownHeight + componentPadding;
    int sliderY2 = sliderY + dropdownHeight + componentPadding;
    int sliderY3 = sliderY2 + dropdownHeight + componentPadding;
    int sliderY4 = sliderY3 + dropdownHeight + componentPadding;
    int sliderWidth = (getWidth() - (3 * padding)); // Two sliders with padding in between
    masterVolumeSlider.setBounds (padding + labelWidth, sliderY, sliderWidth - labelWidth, sliderHeight);
    calibrationVolumeSlider.setBounds (padding + labelWidth, sliderY2, sliderWidth - labelWidth - buttonWidth, sliderHeight);
    startStopButton.setBounds (getWidth() - buttonWidth - padding, sliderY2, buttonWidth, sliderHeight);
    sineVolumeSlider.setBounds (padding + labelWidth, sliderY3, sliderWidth / 2.0f - labelWidth, sliderHeight);
//    spacingSlider.setBounds (padding + labelWidth + sliderWidth / 2.0f, sliderY3, sliderWidth / 2.0f - labelWidth, sliderHeight);
//    pitchSlider.setBounds (padding + labelWidth, sliderY4, sliderWidth / 2.0f - labelWidth, sliderHeight);
    speedSlider.setBounds (padding + labelWidth + sliderWidth / 2.0f, sliderY3, sliderWidth / 2.0f - labelWidth, sliderHeight);
}

// ====================================================
int CabinEqPage::addBand (float freq, float ampl, float bandwidth, CabinPeqGraph* sender)
{
    std::cout << "Listener got adding band" << std::endl;
    if (sender == amplGraph.get())
    {
        int addedBandId = processor.addBand (freq, ampl, bandwidth, profileId);
        processor.updateFilter (profileId);
        return addedBandId;
    }
    
    return -1;
}

void CabinEqPage::updateBand (int id, float freq, float ampl, float bandwidth, CabinPeqGraph* sender)
{
    if (sender == amplGraph.get())
    {
        processor.updateBand (id, freq, ampl, bandwidth, profileId);
        processor.updateFilter (profileId);
    }
}

void CabinEqPage::removeBand (int id, CabinPeqGraph* sender)
{
    if (sender == amplGraph.get())
    {
        processor.removeBand (id, profileId);
        processor.updateFilter (profileId);
    }
}

void CabinEqPage::startMelodicPatternAt (int id, CabinPeqGraph *sender)
{
    if (sender == amplGraph.get())
    {
        processor.startMelodicPatternAt (id, profileId);
    }
}

void CabinEqPage::updateNoisePatternAt (int id, CabinPeqGraph* sender)
{
//    if (sender == amplGraph.get())
//    {
//        processor.updateNoisePatternAt  (id, profileId);
//    }
}

void CabinEqPage::stopNoisePattern()
{
    processor.stopNoisePattern();
}

void CabinEqPage::setNoisePatternSolo (bool solo)
{
    processor.setNoisePatternSolo (solo);
}

void CabinEqPage::setVolume (float volume, CabinPeqGraph* sender)
{
    if (sender == amplGraph.get())
    {
        processor.setProfileVolume (profileId, volume);
        processor.updateFilter (profileId);
    }
}

BandProfile CabinEqPage::getBandProfile()
{
    return processor.getBandProfile (profileId);
}

float CabinEqPage::getCurrPlayingFreq()
{
    return processor.getCurrPlayingFreq();
}

// ====================================================
void CabinEqPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &masterVolumeSlider)
    {
        processor.setVolume (slider->getValue());
    }
    else if (slider == &calibrationVolumeSlider)
    {
        processor.setCalibrationVolume (slider->getValue());
    }
    else if (slider == &sineVolumeSlider)
    {
        processor.setSineVolume (slider->getValue());
    }
//    else if (slider == &spacingSlider)
//    {
//        processor.setSpacing (slider->getValue());
//    }
//    else if (slider == &pitchSlider)
//    {
//        processor.setPitch (slider->getValue());
//    }
    else if (slider == &speedSlider)
    {
        processor.setSpeed (slider->getValue());
    }
}

void CabinEqPage::sliderDragStarted (juce::Slider *slider)
{
//    if (slider == &melodyVolumeSlider || slider == &noiseVolumeSlider)
//    {
//        processor.startNoisePatternAt (lastSelectedNodeIdForCalibration, profileId);
//    }
}

void CabinEqPage::sliderDragEnded (juce::Slider *slider)
{
//    if (slider == &melodyVolumeSlider || slider == &noiseVolumeSlider)
//    {
//        processor.stopNoisePattern();
//    }
}

void CabinEqPage::textEditorTextChanged (juce::TextEditor& textEditor)
{
    // Check if the text is a duplicate. If it is, add a warning on the alert window
    auto text = textEditor.getText();
    bool isDuplicate = isDuplicateProfileName (text);
    if (renamingProfile && text == profileId) // To be less annoying, let people rename a profile to itself
        isDuplicate = false;
    alertWindow->setMessage (isDuplicate ? "This profile name is already taken!" :
                                           "Enter your profile name");
}

void CabinEqPage::textEditorReturnKeyPressed (juce::TextEditor& textEditor)
{
    if (textEditor.getText().isEmpty())
    {
        dismissAlertWindow();
        return;
    }
    
    // Check if the text is a duplicate. If it is, don't add or do anything
    if (isDuplicateProfileName (textEditor.getText()))
        return;
    
    // If we're renaming the profile to a new name, change the name of the profile
    if (renamingProfile)
    {
        juce::String text = textEditor.getText();
        processor.renameProfile (profileId, text);
        profileId = text;
        goToProfileWithId (text);
        renamingProfile = false;
    }
    
    // Add & retrieve profile from processor
    juce::String profileName = textEditor.getText();
    if (! creatingDuplicate)
    {
        processor.addProfile (profileName);
    }
    else
    {
        processor.addDuplicateProfile (profileName, profileId);
        creatingDuplicate = false;
    }
    
    loadDropdownOptions();
    
    // Select the new profile and go to it
    profileDropdown.setSelectedId (profileDropdown.getItemId (profileDropdown.getNumItems() - 2));
    goToProfileWithId (profileName);
    loadDropdownOptions();
    
    // Finally, dismiss the window
    dismissAlertWindow();
}

void CabinEqPage::textEditorEscapeKeyPressed (juce::TextEditor& textEditor)
{
    dismissAlertWindow();
}

void CabinEqPage::textEditorFocusLost (juce::TextEditor& textEditor)
{
    dismissAlertWindow();
}

void CabinEqPage::comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &profileDropdown)
    {
        // Lazy flag so weird stuff doesn't happen when adding the very first profile
        if (addingFirstProfile)
        {
            addingFirstProfile = false;
            return;
        }
        
        // Add a profile if you select "+ Add Profile"
        if (profileDropdown.getSelectedId() == profileDropdown.getNumItems() - 2)
        {
            // Create and present an alert for the user to enter the profile name into
            alertWindow = std::make_unique<juce::AlertWindow> ("Add Profile", "Enter your profile name", juce::MessageBoxIconType::NoIcon);
            creatingDuplicate = false;
            
            alertWindow->addTextEditor (textEditorName, "");
            alertWindow->getTextEditor (textEditorName)->addListener (this);
            alertWindow->setEscapeKeyCancels (true);
            
            alertWindow->enterModalState();
            
            profileDropdown.setSelectedId (lastSelectedId);
        }
        
        // Duplicate a profile if you select "Duplicate [profilename]"
        else if (profileDropdown.getSelectedId() == profileDropdown.getNumItems() - 1)
        {
            // Present option to add duplicate profile, and opportunity to name it
            // Create a present an alert for the user to enter the profile name into
            alertWindow = std::make_unique<juce::AlertWindow> ("Create Duplicate Profile", "Enter your profile name", juce::MessageBoxIconType::NoIcon);
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
            
            alertWindow->enterModalState();
            
            profileDropdown.setSelectedId (lastSelectedId);
        }
        
        // Rename a profile if you select "Rename [profileName]"
        else if (profileDropdown.getSelectedId() == profileDropdown.getNumItems())
        {
            // Create and present alert to rename profile
            alertWindow = std::make_unique<juce::AlertWindow> ("Rename Profile", "Enter your profile name", juce::MessageBoxIconType::NoIcon);
            renamingProfile = true;
            
            alertWindow->addTextEditor (textEditorName, profileId);
            alertWindow->getTextEditor (textEditorName)->addListener (this);
            alertWindow->setEscapeKeyCancels (true);
            
            alertWindow->enterModalState();
            
            profileDropdown.setSelectedId (lastSelectedId);
        }
        
        // Go to a profile if you select the profile
        else if (profileDropdown.getSelectedId() != lastSelectedId && profileDropdown.getSelectedId() > 0)
        {
            int selectedIndex = profileDropdown.indexOfItemId (profileDropdown.getSelectedId());
            juce::String profileIdSelected = profileDropdown.getItemText (selectedIndex);
            goToProfileWithId (profileIdSelected);
            loadDropdownOptions();
        }
        
        lastSelectedId = profileDropdown.getSelectedId();
    }
    else if (comboBoxThatHasChanged == &filterQualityDropdown)
    {
        auto fftSizeBefore = fftSize;
        switch (filterQualityDropdown.getSelectedItemIndex())
        {
            case 0:
                fftSize = 21;
                break;
            case 1:
                fftSize = 18;
                break;
            case 2:
                fftSize = 16;
                break;
            case 3:
                fftSize = 12;
                break;
            case 4:
                fftSize = 10;
        }
    }
}

void CabinEqPage::inputAttemptWhenModal()
{
    dismissAlertWindow();
}

void CabinEqPage::buttonClicked (juce::Button *button)
{
    if (button == &bypassButton)
    {
        toggleBypass();
        processor.setIsProcessing (! isBypassed);
    }
//    else if (button == &startStopButton)
//    {
//        playingNoisePattern = ! playingNoisePattern;
//        if (playingNoisePattern)
//            processor.startMelodicPatternAt (dragging, <#juce::String profileName#>);
//        else
//            processor.stopNoisePattern();
//    }
}

void CabinEqPage::didLoadData()
{
    addingFirstProfile = true;
    processor.setIsProcessing (! isBypassed);
    auto lastSelectedProfileName = processor.getLastSelectedProfileName();
    if (lastSelectedProfileName.has_value())
    {
        goToProfileWithId (lastSelectedProfileName.value());
        loadDropdownOptions();
    }
    
    // If there are no profiles, add one
    if (processor.getProfileNames().size() == 0)
    {
        juce::String firstProfileId = "My Headphone Profile";
        processor.addProfile (firstProfileId);
        goToProfileWithId (firstProfileId);
    }
}

//=========================================
void CabinEqPage::toggleBypass()
{
    isBypassed = ! isBypassed;
    amplGraph->setGrayscale (isBypassed);
//    panGraph->setGrayscale (isBypassed);
//    phaseGraph->setGrayscale (isBypassed);
    updateButtonText();
}

void CabinEqPage::loadDropdownOptions()
{
    profileDropdown.clear();
    
    // Add existing profiles to dropdown menu
    int i = 1;
    for (const auto& name : processor.getProfileNames())
    {
        profileDropdown.addItem (name, i);
        i++;
    }
    
    if (profileDropdown.getNumItems() > 0)
    {
        profileDropdown.addSeparator();
        profileDropdown.setText (profileId);
    }
    
    profileDropdown.addItem ("+ Add Profile", i);
    profileDropdown.addItem ("[] Duplicate \"" + profileId + "\"", i + 1);
    profileDropdown.addItem ("* Rename \"" + profileId + "\"", i + 2);
}

void CabinEqPage::dismissAlertWindow()
{
    alertWindow->getTextEditor (textEditorName)->removeListener (this);
    alertWindow.reset();
}
                                
void CabinEqPage::updateButtonText()
{
    bypassButton.setButtonText (isBypassed ? "OFF" : "ON");
}

void CabinEqPage::showForm()
{
//    unlockForm.setVisible (true);
    bypassButton.setEnabled (true);
}

void CabinEqPage::unlockApp()
{
    bypassButton.setEnabled (true);
//    unlockLabel.setText ("Status: Unlocked", juce::dontSendNotification);
//    unlockLabel.setColour (juce::Label::textColourId, juce::Colours::green);
}

void CabinEqPage::goToProfileWithId (juce::String profileIdToGoTo)
{
    profileId = profileIdToGoTo;
    
    BandProfile bandProfile = processor.getBandProfile (profileId);
    amplGraph->setBandProfile (bandProfile);
    
    processor.setLastSelectedProfileName (profileId);
    profileDropdown.setText (profileIdToGoTo);
    processor.updateFilter (profileId);
}

bool CabinEqPage::isDuplicateProfileName (juce::String profileName)
{
    auto profileNames = processor.getProfileNames();
    
    for (const auto& existingProfileName : profileNames)
        if (profileName == existingProfileName)
            return true;
    
    return false;
}
