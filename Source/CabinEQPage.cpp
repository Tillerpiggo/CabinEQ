/*
  ==============================================================================

    CabinEQPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQPage.h"

CabinEQPage::CabinEQPage (StartupMVPAudioProcessor& p)
    : processor (p), profileId ("NO_PROFILE"), cabinEQGraph()//, unlockForm (marketplaceStatus)
{
    dropdownProfiles.addItem ("+ Add Profile", 1);
    
    referenceSlider.setRange (-24.0f, 24.0f);
    referenceSlider.setValue (0.0f);
    
    wetVolumeSlider.setRange (-20.0f, 20.0f);
    dryVolumeSlider.setRange (-20.0f, 20.0f);
    wetVolumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    dryVolumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    wetVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    dryVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    
    cabinEQGraph.addListener (this);
    dropdownProfiles.addListener (this);
    referenceSlider.addListener (this);
    bypassButton.addListener (this);
    blindButton.addListener (this);
    applyButton.addListener (this);
    processor.addListener (this);
    dryVolumeSlider.addListener (this);
    wetVolumeSlider.addListener (this);
    
    addAndMakeVisible (cabinEQGraph);
    addAndMakeVisible (dropdownProfiles);
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
    
    dropdownProfiles.removeListener (this);
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
    int buttonsY = padding + graphHeight + padding;

    cabinEQGraph.setBounds(0, padding, getWidth(), graphHeight);
    dropdownProfiles.setBounds(padding, buttonsY, dropdownWidth, dropdownHeight);
    bypassButton.setBounds(padding + dropdownWidth, buttonsY, buttonWidth, dropdownHeight);

    int currentX = padding + dropdownWidth + buttonWidth;
    applyButton.setBounds(currentX, buttonsY, applyButtonWidth, dropdownHeight);
    currentX += applyButtonWidth;

    blindButton.setBounds (currentX, buttonsY, duplicateButtonWidth, dropdownHeight);

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
    
    // Select the new profile
    dropdownProfiles.setSelectedId (dropdownProfiles.getItemId (dropdownProfiles.getNumItems() - 2));
    
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
    if (comboBoxThatHasChanged == &dropdownProfiles)
    {
        // Add a profile if you select "+ Add Profile"
        if (dropdownProfiles.getSelectedId() == dropdownProfiles.getNumItems() - 1)
        {
            // Create a present an alert for the user to enter the profile name into
            alertWindow = std::make_unique<juce::AlertWindow> ("Add Profile", "Enter your profile name", juce::MessageBoxIconType::NoIcon);
            creatingDuplicate = false;
            
            alertWindow->addTextEditor (textEditorName, "");
            alertWindow->getTextEditor (textEditorName)->addListener (this);
            alertWindow->setEscapeKeyCancels (true);
            
            alertWindow->enterModalState();
            
            dropdownProfiles.setSelectedId (lastSelectedId);
        }
        
        // Duplicate a profile if you select "Duplicate [profilename]"
        else if (dropdownProfiles.getSelectedId() == dropdownProfiles.getNumItems())
        {
            // Present option to add duplicate profile, and opportunity to name it
            // Create a present an alert for the user to enter the profile name into
            alertWindow = std::make_unique<juce::AlertWindow> ("Create Duplicate Profile", "Enter your profile name", juce::MessageBoxIconType::NoIcon);
            creatingDuplicate = true;
            
            alertWindow->addTextEditor (textEditorName, "");
            alertWindow->getTextEditor (textEditorName)->addListener (this);
            alertWindow->setEscapeKeyCancels (true);
            
            alertWindow->enterModalState();
            
            dropdownProfiles.setSelectedId (lastSelectedId);
        }
        
        // Go to a profile if you select the profile
        else
        {
            int selectedIndex = dropdownProfiles.indexOfItemId (dropdownProfiles.getSelectedId());
            juce::String profileIdSelected = dropdownProfiles.getItemText (selectedIndex);
            profileId = profileIdSelected;
            cabinEQGraph.setCurve (processor.getAmplCurve (profileIdSelected)->get()); // HARD CODING AMPL FOR NOW
            flagFilterChanged();
            loadDropdownOptions();
        }
        
        lastSelectedId = dropdownProfiles.getSelectedId();
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
    loadDropdownOptions();
    applyFilter();
    processor.setIsProcessing (! isBypassed);
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
    hasFilterChanged = false;
    processor.applyCurve (profileId);
    updateButtonText();
}

void CabinEQPage::loadDropdownOptions()
{
    dropdownProfiles.clear();
    
    // Add existing profiles to dropdown menu
    int i = 1;
    for (const auto& name : processor.getProfileNames())
    {
        dropdownProfiles.addItem (name, i);
        i++;
    }
    
    if (dropdownProfiles.getNumItems() > 0)
    {
        dropdownProfiles.addSeparator();
        dropdownProfiles.setSelectedId (dropdownProfiles.getItemId (1));
    }
    
    dropdownProfiles.addItem ("+ Add Profile", i);
    dropdownProfiles.addItem ("[] Duplicate this profile", i + 1);
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
