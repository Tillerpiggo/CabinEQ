/*
  ==============================================================================

    CabinEqPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqPage.h"

CabinEqPage::CabinEqPage (CabinEqAudioProcessor& p)
    : processor (p), profileId ("NO_PROFILE"), graphs (juce::TabbedButtonBar::Orientation::TabsAtTop)
{
    leftAmplGraph = std::make_unique<CabinEqGraph>();
    rightAmplGraph = std::make_unique<CabinEqGraph>();
    
    auto backgroundColor = juce::Colour::fromRGB (0.1, 0.1, 0.2); // DRY violation; redundant w/ CabinEqGraph BACKGROUND_COLOR
    graphs.addTab ("Left Volume", backgroundColor, leftAmplGraph.get(), false);
    graphs.addTab ("Right Volume", backgroundColor, rightAmplGraph.get(), false);
    
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
    wetVolumeLabel.setText ("Calibrated Volume", juce::dontSendNotification);
    dryVolumeLabel.setText ("Normal Volume", juce::dontSendNotification);
    wetVolumeLabel.setJustificationType (juce::Justification::centred);
    dryVolumeLabel.setJustificationType (juce::Justification::centred);
    
    leftAmplGraph->addListener (this);
    rightAmplGraph->addListener (this);
    profileDropdown.addListener (this);
    filterQualityDropdown.addListener (this);
    referenceSlider.addListener (this);
    bypassButton.addListener (this);
    blindButton.addListener (this);
    applyButton.addListener (this);
    processor.addListener (this);
    dryVolumeSlider.addListener (this);
    wetVolumeSlider.addListener (this);
    
    addAndMakeVisible (graphs);
    addAndMakeVisible (profileDropdown);
    addAndMakeVisible (filterQualityDropdown);
    addAndMakeVisible (referenceSlider);
    addAndMakeVisible (bypassButton);
    addAndMakeVisible (applyButton);
    addAndMakeVisible (blindButton);
    addAndMakeVisible (dryVolumeSlider);
    addAndMakeVisible (wetVolumeSlider);
    addAndMakeVisible (dryVolumeLabel);
    addAndMakeVisible (wetVolumeLabel);
    
    didLoadData();
}

CabinEqPage::~CabinEqPage()
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
    
    leftAmplGraph->removeListener();
    rightAmplGraph->removeListener();
    
    processor.removeListener();
}

void CabinEqPage::paint (juce::Graphics& g)
{
    g.fillAll (backgroundColor);
}

void CabinEqPage::resized()
{
//    graphs.setBounds (getLocalBounds());
    
    int padding = 20; // padding on the top and bottom
    int componentPadding = 10; // padding between graph, slider, and dropdown
    int dropdownHeight = 30;
    int sliderHeight = 30;
    int labelHeight = 15;
    int buttonWidth = 100;
    int applyButtonWidth = 100;
    int duplicateButtonWidth = 100;
    int totalButtonWidth = buttonWidth + duplicateButtonWidth + applyButtonWidth;

    // Get heights for each component
    int availableHeight = getHeight() - (2 * padding);
    int graphHeight = availableHeight - dropdownHeight - sliderHeight - labelHeight - 3 * componentPadding; // Remaining height for the graph
    
    // Get widths for each component
    int dropdownWidth = getWidth() - (2 * padding) - totalButtonWidth;
    int profileDropdownWidth = static_cast<int>(dropdownWidth * 0.75); // 75% width
    int filterQualityDropdownWidth = dropdownWidth - profileDropdownWidth; // Remaining 25% width

    int buttonsY = padding + graphHeight + componentPadding;

    // Set bounds for graph and buttons
    graphs.setBounds (0, padding, getWidth(), graphHeight);
    profileDropdown.setBounds (padding, buttonsY, profileDropdownWidth, dropdownHeight);
    filterQualityDropdown.setBounds (padding + profileDropdownWidth, buttonsY, filterQualityDropdownWidth, dropdownHeight);
    int currentX = padding + dropdownWidth + buttonWidth;
    bypassButton.setBounds (padding + dropdownWidth, buttonsY, buttonWidth, dropdownHeight);
    applyButton.setBounds (currentX, buttonsY, applyButtonWidth, dropdownHeight);
    currentX += applyButtonWidth;
    blindButton.setBounds (currentX, buttonsY, duplicateButtonWidth, dropdownHeight);

    // Set bounds for sliders
    int sliderY = buttonsY + dropdownHeight + componentPadding;
    int sliderWidth = (getWidth() - (3 * padding)) / 2; // Two sliders with padding in between
    wetVolumeSlider.setBounds (padding, sliderY, sliderWidth, sliderHeight);
    dryVolumeSlider.setBounds (padding + sliderWidth + padding, sliderY, sliderWidth, sliderHeight);
    
    // Set bounds for labels
    int labelY = sliderY + sliderHeight + componentPadding;
    int labelWidth = sliderWidth;
    wetVolumeLabel.setBounds (padding, labelY, labelWidth, labelHeight);
    dryVolumeLabel.setBounds (padding + labelWidth + padding, labelY, labelWidth, labelHeight);
}

// ====================================================
int CabinEqPage::addCurvePt (float freq, float val, CabinEqGraph* sender)
{
    flagFilterChanged();
    if (sender == leftAmplGraph.get())
    {
        return processor.addLeftAmplPt (freq, val, profileId);
    }
    else if (sender == rightAmplGraph.get())
    {
        return processor.addRightAmplPt (freq, val, profileId);
    }
    
    std::cout << "WARNING: Add Curve Pt failed because sender was not ampl, pan, or phase graph";
    return -1;
}

void CabinEqPage::updateCurvePt (int id, float freq, float val, CabinEqGraph* sender)
{
    flagFilterChanged();
    if (sender == leftAmplGraph.get())
    {
        processor.updateLeftAmplPt (id, freq, val, profileId);
    }
    else if (sender == rightAmplGraph.get())
    {
        processor.updateRightAmplPt (id, freq, val, profileId);
    }
}

void CabinEqPage::removeCurvePt (int id, CabinEqGraph* sender)
{
    flagFilterChanged();
    if (sender == leftAmplGraph.get())
    {
        processor.removeLeftAmplPt (id, profileId);
    }
    else if (sender == rightAmplGraph.get())
    {
        processor.removeRightAmplPt (id, profileId);
    }
}

void CabinEqPage::startPlayingValueAt (float freq, CabinEqGraph* sender)
{
    if (sender == leftAmplGraph.get())
    {
        processor.startLeftAmplCalibration (freq, profileId);
    }
    else if (sender == rightAmplGraph.get())
    {
        processor.startRightAmplCalibration (freq, profileId);
    }
}

void CabinEqPage::updatePlayingValueAt (float freq, CabinEqGraph* sender)
{
    if (sender == leftAmplGraph.get())
    {
        processor.updateLeftAmplCalibration(freq, profileId);
    }
    else if (sender == rightAmplGraph.get())
    {
        processor.updateRightAmplCalibration (freq, profileId);
    }
}

void CabinEqPage::testValueAt (float freq)
{
    processor.startTestingAt (freq, profileId);
}

void CabinEqPage::stopPlaying()
{
    processor.stopCalibration();
    processor.endSineSweep();
}

void CabinEqPage::stopTesting()
{
    processor.endTesting();
}

float CabinEqPage::getCurrPlayingFreq()
{
    return processor.getCurrPlayingFreq();
}

float CabinEqPage::getCurrTestingFreq()
{
    return processor.getCurrTestingFreq();
}

void CabinEqPage::userStoppedDoingShit()
{
//    applyFilterIfProcessing(); // stop autosaving
}

// ====================================================
void CabinEqPage::sliderValueChanged (juce::Slider *slider)
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

void CabinEqPage::textEditorReturnKeyPressed (juce::TextEditor& textEditor)
{
    std::cout << "text editor return key pressed" << std::endl;
    if (textEditor.getText().isEmpty())
        return;
    
    std::cout << "not empty>?" << std::endl;
    
    // Add the profile and dismiss the window
    juce::String profileName = textEditor.getText();
    
    std::cout << "got the text" << std::endl;
    
    std::cout << "creating duplicate: " << creatingDuplicate << std::endl;
    
    // Add & retrieve profile from processor
    if (! creatingDuplicate)
    {
        std::cout << "adding profile" << std::endl;
        processor.addProfile (profileName);
        std::cout << "added profile" << std::endl;
    }
    else
    {
        processor.addDuplicateProfile (profileName, profileId);
    }
    
    std::cout << "loading dropdown options" << std::endl;
    
    loadDropdownOptions();
    
    // Select the new profile and go to it
    profileDropdown.setSelectedId (profileDropdown.getItemId (profileDropdown.getNumItems() - 2));
    std::cout << "about to go to profile with id" << std::endl;
    goToProfileWithId (profileName);
    std::cout << "went to profile with id" << std::endl;
    loadDropdownOptions();
    
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
        // Add a profile if you select "+ Add Profile"
        if (profileDropdown.getSelectedId() == profileDropdown.getNumItems() - 1 || profileDropdown.getNumItems() == 1)
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
        
        if (fftSize != fftSizeBefore)
        {
            flagFilterChanged();
            applyFilter();
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
    else if (button == &applyButton)
    {
        applyFilter();
    }
    else if (button == &blindButton)
    {
        toggleBlind();
    }
}

void CabinEqPage::didLoadData()
{
    applyFilter();
    processor.setIsProcessing (! isBypassed);
    auto lastSelectedProfileName = processor.getLastSelectedProfileName();
    if (lastSelectedProfileName.has_value())
    {
        goToProfileWithId (lastSelectedProfileName.value());
        loadDropdownOptions();
    }
}

void CabinEqPage::timerCallback()
{
//    if (! isUnlocked && marketplaceStatus.isUnlocked())
//    {
//        isUnlocked = true;
//        unlockApp();
//    }
}

//=========================================
void CabinEqPage::flagFilterChanged()
{
    hasFilterChanged = true;
    updateButtonText();
}

void CabinEqPage::toggleBypass()
{
    isBypassed = ! isBypassed;
    leftAmplGraph->setGrayscale (isBypassed);
    rightAmplGraph->setGrayscale (isBypassed);
    updateButtonText();
}

void CabinEqPage::toggleBlind()
{
    isBlind = ! isBlind;
    blindButton.setButtonText (isBlind ? "UNBLIND" : "BLIND");
    leftAmplGraph->setBlinded (isBlind);
    rightAmplGraph->setBlinded (isBlind);
    updateButtonText();
}

void CabinEqPage::applyFilter()
{
    if (hasFilterChanged)
        processor.applyCurve (fftSize, profileId);
    hasFilterChanged = false;
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
    profileDropdown.addItem ("[] Duplicate this profile", i + 1);
}

void CabinEqPage::dismissAlertWindow()
{
    alertWindow->getTextEditor (textEditorName)->removeListener (this);
    alertWindow.reset();
}
                                
void CabinEqPage::updateButtonText()
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
    auto leftAmplCurve = processor.getLeftAmplCurve (profileIdToGoTo);
    auto rightAmplCurve = processor.getRightAmplCurve (profileIdToGoTo);
    if (leftAmplCurve.has_value() && rightAmplCurve.has_value())
    {
        leftAmplGraph->setCurve (leftAmplCurve->get());
        rightAmplGraph->setCurve (rightAmplCurve->get());
        flagFilterChanged();
        applyFilter();
        processor.setLastSelectedProfileName (profileId);
        profileDropdown.setText (profileIdToGoTo);
    }
}
