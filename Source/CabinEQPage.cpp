/*
  ==============================================================================

    CabinEQPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQPage.h"

CabinEQPage::CabinEQPage (CabinEQAudioProcessor& p)
    : processor (p), profileId ("NO_PROFILE"), cabinEQGraph()//, unlockForm (marketplaceStatus)
{
    filterQualityDropdown.addItem ("Utopian", 1);
    filterQualityDropdown.addItem ("Fantastic", 2);
    filterQualityDropdown.addItem ("Great", 3);
    filterQualityDropdown.addItem ("Good", 4);
    filterQualityDropdown.addItem ("Economy", 5);
    filterQualityDropdown.setSelectedId (3);
    profileDropdown.addItem ("+ Add Profile", 1);
    
    referenceSlider.setRange (-24.0f, 24.0f);
    referenceSlider.setValue (0.0f);
    
    wetVolumeSlider.setRange (-20.0f, 20.0f);
    dryVolumeSlider.setRange (-20.0f, 20.0f);
    wetVolumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    dryVolumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    wetVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    dryVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    
    cabinEQGraph.addListener (this);
    profileDropdown.addListener (this);
    filterQualityDropdown.addListener (this);
    referenceSlider.addListener (this);
    bypassButton.addListener (this);
    blindButton.addListener (this);
    applyButton.addListener (this);
    processor.addListener (this);
    dryVolumeSlider.addListener (this);
    wetVolumeSlider.addListener (this);
    
    addAndMakeVisible (cabinEQGraph);
    addAndMakeVisible (profileDropdown);
    addAndMakeVisible (filterQualityDropdown);
    addAndMakeVisible (referenceSlider);
    addAndMakeVisible (bypassButton);
    addAndMakeVisible (applyButton);
    addAndMakeVisible (blindButton);
    addAndMakeVisible (dryVolumeSlider);
    addAndMakeVisible (wetVolumeSlider);
    
    didLoadData();
}

CabinEQPage::~CabinEQPage()
{
    referenceSlider.removeListener (this);
    
    profileDropdown.removeListener (this);
    filterQualityDropdown.removeListener (this);
    referenceSlider.removeListener (this);
    bypassButton.removeListener (this);
    applyButton.removeListener (this);
    blindButton.removeListener (this);
    dryVolumeSlider.removeListener (this);
    wetVolumeSlider.removeListener (this);
    
    cabinEQGraph.removeListener();
    processor.removeListener();
}

void CabinEQPage::paint (juce::Graphics& g)
{
    g.fillAll (backgroundColor);
}

void CabinEQPage::resized()
{
    int padding = 5; // Reduced padding
    int dropdownHeight = 30;
    int buttonWidth = 100;
    int applyButtonWidth = 100;
    int duplicateButtonWidth = 100;
    int totalButtonWidth = buttonWidth + duplicateButtonWidth + applyButtonWidth;

    // Calculate available height for the graph and sliders
    int availableHeight = getHeight() - (3 * padding) - (2 * dropdownHeight);
    
    int sliderHeight = 20; // Small height for sliders
    int graphHeight = availableHeight - sliderHeight; // Remaining height for the graph

    int dropdownWidth = getWidth() - (2 * padding) - totalButtonWidth;

    // Adjust the widths for profileDropdown and filterQualityDropdown
    int profileDropdownWidth = static_cast<int>(dropdownWidth * 0.75); // 75% width
    int filterQualityDropdownWidth = dropdownWidth - profileDropdownWidth; // Remaining 25% width

    int buttonsY = padding + graphHeight + padding;

    cabinEQGraph.setBounds(0, padding, getWidth(), graphHeight);
    profileDropdown.setBounds(padding, buttonsY, profileDropdownWidth, dropdownHeight);

    // Position filterQualityDropdown to the right of profileDropdown
    filterQualityDropdown.setBounds(padding + profileDropdownWidth, buttonsY, filterQualityDropdownWidth, dropdownHeight);

    // Adjust the positions of the buttons
    int currentX = padding + dropdownWidth + buttonWidth;
    bypassButton.setBounds(padding + dropdownWidth, buttonsY, buttonWidth, dropdownHeight);

    applyButton.setBounds(currentX, buttonsY, applyButtonWidth, dropdownHeight);
    currentX += applyButtonWidth;

    blindButton.setBounds(currentX, buttonsY, duplicateButtonWidth, dropdownHeight);

    // Calculate positions for sliders
    int sliderY = padding + buttonsY + dropdownHeight + padding;
    int sliderWidth = (getWidth() - (3 * padding)) / 2; // Two sliders with padding in between

    wetVolumeSlider.setBounds(padding, sliderY, sliderWidth, sliderHeight);
    dryVolumeSlider.setBounds(padding + sliderWidth + padding, sliderY, sliderWidth, sliderHeight);
}

// ====================================================
int CabinEQPage::addCurvePt (float freq, float ampl, CabinEQGraph* sender)
{
    flagFilterChanged();
    std::cout << "curve pt added" << std::endl;
    return processor.addAmplPt (freq, ampl, profileId); // TODO: FIX, I'M JUST DOING AMPL FOR NOW
}

void CabinEQPage::updateCurvePt (int id, float freq, float ampl, CabinEQGraph* sender)
{
    flagFilterChanged();
    processor.updateAmplPt (id, freq, ampl, profileId);
}

void CabinEQPage::removeCurvePt (int id, CabinEQGraph* sender)
{
    flagFilterChanged();
    std::cout << "curve pt removed" << std::endl;
    processor.removeAmplPt (id, profileId);
}

void CabinEQPage::startPlayingValueAt (float freq, float ampl)
{
    processor.startPlayingFreq (freq, profileId); // hacky way to make this impact pan
//    processor.startSineSweep (freq, profileId);
}

void CabinEQPage::playValueAt (float freq, float ampl)
{
    processor.updatePlayingFreq (freq, profileId); // hacky way to make this impact pan
//    processor.updateSineSweep (freq, profileId);
}

void CabinEQPage::testValueAt (float freq)
{
    processor.startTestingAt (freq, profileId);
}

void CabinEQPage::stopPlaying()
{
    processor.endCalibratingEQNode();
    processor.endSineSweep();
}

void CabinEQPage::stopTesting()
{
    processor.endTesting();
}

float CabinEQPage::getCurrPlayingFreq()
{
    return processor.getCurrPlayingFreq();
}

float CabinEQPage::getCurrTestingFreq()
{
    return processor.getCurrTestingFreq();
}

void CabinEQPage::userStoppedDoingShit()
{
//    applyFilterIfProcessing(); // stop autosaving
}

// ====================================================
void CabinEQPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &referenceSlider)
    {
        processor.setReferenceVolume (slider->getValue());
    }
    else if (slider == &dryVolumeSlider)
    {
        processor.setDryVolume (slider->getValue());
    }
    else if (slider == &wetVolumeSlider)
    {
        processor.setWetVolume (slider->getValue());
    }
}

void CabinEQPage::textEditorReturnKeyPressed (juce::TextEditor& textEditor)
{
    std::cout << "text editor return key pressed" << std::endl;
    if (textEditor.getText().isEmpty())
        return;
    
    // Add the profile and dismiss the window
    juce::String profileName = textEditor.getText();
    
    // Add & retrieve profile from processor
    if (! creatingDuplicate)
    {
        processor.addProfile (profileName);
    }
    else
    {
        processor.addDuplicateProfile (profileName, profileId);
    }
    
    loadDropdownOptions();
    
    // Select the new profile and go to it
    profileDropdown.setSelectedId (profileDropdown.getItemId (profileDropdown.getNumItems() - 2));
    goToProfileWithId (profileName);
    
    dismissAlertWindow();
}

void CabinEQPage::textEditorEscapeKeyPressed (juce::TextEditor& textEditor)
{
    dismissAlertWindow();
}

void CabinEQPage::textEditorFocusLost (juce::TextEditor& textEditor)
{
    dismissAlertWindow();
}

void CabinEQPage::comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &profileDropdown)
    {
        // Add a profile if you select "+ Add Profile"
        if (profileDropdown.getSelectedId() == profileDropdown.getNumItems() - 1)
        {
            // Create a present an alert for the user to enter the profile name into
            alertWindow = std::make_unique<juce::AlertWindow> ("Add Profile", "Enter your profile name", juce::MessageBoxIconType::NoIcon);
            creatingDuplicate = false;
            
            alertWindow->addTextEditor (textEditorName, "");
            alertWindow->getTextEditor (textEditorName)->addListener (this);
            alertWindow->setEscapeKeyCancels (true);
            
            alertWindow->enterModalState();
            
            profileDropdown.setSelectedId (lastSelectedId);
        }
        
        // Duplicate a profile if you select "Duplicate [profilename]"
        else if (profileDropdown.getSelectedId() == profileDropdown.getNumItems())
        {
            // Present option to add duplicate profile, and opportunity to name it
            // Create a present an alert for the user to enter the profile name into
            alertWindow = std::make_unique<juce::AlertWindow> ("Create Duplicate Profile", "Enter your profile name", juce::MessageBoxIconType::NoIcon);
            creatingDuplicate = true;
            
            alertWindow->addTextEditor (textEditorName, "");
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
        
        if (fftSize != fftSizeBefore)
        {
            flagFilterChanged();
            applyFilter();
        }
    }
}

void CabinEQPage::inputAttemptWhenModal()
{
    dismissAlertWindow();
}

void CabinEQPage::buttonClicked (juce::Button *button)
{
    if (button == &bypassButton)
    {
        toggleBypass();
        processor.setIsProcessing (! isBypassed);
    }
    else if (button == &applyButton)
    {
        applyFilter();
    }
    else if (button == &blindButton)
    {
        toggleBlind();
    }
}

void CabinEQPage::didLoadData()
{
    applyFilter();
    processor.setIsProcessing (! isBypassed);
    auto lastSelectedProfileName = processor.getLastSelectedProfileName();
    if (lastSelectedProfileName.has_value())
        goToProfileWithId (lastSelectedProfileName.value());
    loadDropdownOptions();
}

void CabinEQPage::timerCallback()
{
//    if (! isUnlocked && marketplaceStatus.isUnlocked())
//    {
//        isUnlocked = true;
//        unlockApp();
//    }
}

//=========================================
void CabinEQPage::flagFilterChanged()
{
    hasFilterChanged = true;
    updateButtonText();
}

void CabinEQPage::toggleBypass()
{
    isBypassed = ! isBypassed;
    cabinEQGraph.setGrayscale (isBypassed);
    updateButtonText();
}

void CabinEQPage::toggleBlind()
{
    isBlind = ! isBlind;
    blindButton.setButtonText (isBlind ? "UNBLIND" : "BLIND");
    cabinEQGraph.setBlinded (isBlind);
    updateButtonText();
}

void CabinEQPage::applyFilter()
{
    if (hasFilterChanged)
        processor.applyCurve (fftSize, profileId);
    hasFilterChanged = false;
    updateButtonText();
}

void CabinEQPage::loadDropdownOptions()
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
    profileDropdown.addItem ("[] Duplicate this profile", i + 1);
}

void CabinEQPage::dismissAlertWindow()
{
    alertWindow->getTextEditor (textEditorName)->removeListener (this);
    alertWindow.reset();
}
                                
void CabinEQPage::updateButtonText()
{
    if (! isBlind)
    {
        bypassButton.setButtonText (isBypassed ? "OFF" : (hasFilterChanged ? "ON*" : "ON"));
    }
    else
    {
        bypassButton.setButtonText ("[BLINDED]");
    }
    
    applyButton.setEnabled (hasFilterChanged); // only let people apply the filter when there is something to update
}

void CabinEQPage::showForm()
{
//    unlockForm.setVisible (true);
    bypassButton.setEnabled (true);
}

void CabinEQPage::unlockApp()
{
    bypassButton.setEnabled (true);
//    unlockLabel.setText ("Status: Unlocked", juce::dontSendNotification);
//    unlockLabel.setColour (juce::Label::textColourId, juce::Colours::green);
}

void CabinEQPage::goToProfileWithId (juce::String profileIdToGoTo)
{
    profileId = profileIdToGoTo;
    auto amplCurve = processor.getAmplCurve (profileIdToGoTo);
    if (amplCurve.has_value())
    {
        cabinEQGraph.setCurve (amplCurve->get()); // HARD CODING AMPL FOR NOW
        flagFilterChanged();
        applyFilter();
        processor.setLastSelectedProfileName (profileId);
        profileDropdown.setText (profileIdToGoTo);
    }
}
