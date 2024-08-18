/*
  ==============================================================================

    CabinEQPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQPage.h"

CabinEQPage::CabinEQPage (StartupMVPAudioProcessor& p)
    : processor (p), profileId ("NO_PROFILE"), cabinEQGraph(), unlockForm (marketplaceStatus)
{
    dropdownProfiles.addItem ("+ Add Profile", 1);
    
    referenceSlider.setRange (-24.0f, 24.0f);
    referenceSlider.setValue (0.0f);
    
    bypassButton.setEnabled (false);
    
    cabinEQGraph.addListener (this);
    dropdownProfiles.addListener (this);
    referenceSlider.addListener (this);
    bypassButton.addListener (this);
    unlockButton.addListener (this);
    duplicateButton.addListener (this);
    processor.addListener (this);
    
    addAndMakeVisible (cabinEQGraph);
    addAndMakeVisible (dropdownProfiles);
    addAndMakeVisible (referenceSlider);
    addAndMakeVisible (bypassButton);
    addAndMakeVisible (unlockButton);
    addAndMakeVisible (duplicateButton);
    addAndMakeVisible (unlockForm);
    
    didLoadData();
}

CabinEQPage::~CabinEQPage()
{
    referenceSlider.removeListener (this);
    cabinEQGraph.removeListener();
}

void CabinEQPage::paint (juce::Graphics& g)
{
    g.fillAll (backgroundColor);
}

void CabinEQPage::resized()
{
    int padding = 10;
    int dropdownHeight = 30;
    int buttonWidth = 100;
    int unlockButtonWidth = 100;
    int duplicateButtonWidth = 100;
    int totalButtonWidth = buttonWidth + duplicateButtonWidth;
    
    if (!isUnlocked)
    {
        totalButtonWidth += unlockButtonWidth;
    }

    int graphHeight = getHeight() - (2 * padding) - dropdownHeight;
    int dropdownWidth = getWidth() - (2 * padding) - totalButtonWidth;
    int bottomY = getHeight() - padding - dropdownHeight;

    cabinEQGraph.setBounds(0, padding, getWidth(), graphHeight);
    dropdownProfiles.setBounds(padding, bottomY, dropdownWidth, dropdownHeight);
    bypassButton.setBounds(padding + dropdownWidth, bottomY, buttonWidth, dropdownHeight);

    int currentX = padding + dropdownWidth + buttonWidth;
    
    if (!isUnlocked)
    {
        unlockButton.setBounds(currentX, bottomY, unlockButtonWidth, dropdownHeight);
        currentX += unlockButtonWidth;
    }

    duplicateButton.setBounds(currentX, bottomY, duplicateButtonWidth, dropdownHeight);
}

// ====================================================
int CabinEQPage::addNode (float freq, float ampl)
{
    flagFilterChanged();
    return processor.addEQNode (freq, ampl, 0.0f, profileId, currChannel);
}

void CabinEQPage::updateNode (int id, float freq, float ampl)
{
    flagFilterChanged();
    processor.updateEQNode (id, freq, ampl, 0.0f, profileId, currChannel);
}

void CabinEQPage::removeNode (int id)
{
    flagFilterChanged();
    processor.removeEQNode (id, profileId, currChannel);
}

void CabinEQPage::startPlayingValueAt (float freq, float ampl)
{
    processor.startCalibratingEQNode (EQNode (-1, freq, ampl, 0.0f));
}

void CabinEQPage::playValueAt (float freq, float ampl)
{
    processor.updateCalibratingEQNode (EQNode (-1, freq, ampl, 0.0f));
}

void CabinEQPage::testValueAt (float freq)
{
    processor.startTestingAt (freq, profileId, currChannel);
}

void CabinEQPage::stopPlaying()
{
    processor.endCalibratingEQNode();
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
    applyFilterIfProcessing();
}

// ====================================================
void CabinEQPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &referenceSlider)
    {
        processor.setReferenceVolume (slider->getValue());
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
        if (dropdownProfiles.getSelectedId() == dropdownProfiles.getNumItems())
        {
            // Create a present an alert for the user to enter the profile name into
            alertWindow = std::make_unique<juce::AlertWindow> ("Add Profile", "Enter your profile name", juce::MessageBoxIconType::NoIcon);
            creatingDuplicate = false;
            
            alertWindow->addTextEditor (textEditorName, "");
            alertWindow->getTextEditor (textEditorName)->addListener (this);
            alertWindow->setEscapeKeyCancels (true);
            
            alertWindow->enterModalState();
            
            dropdownProfiles.setSelectedId (lastSelectedId);
            flagFilterChanged();
            applyFilterIfProcessing();
        }
        
        // Go to a profile if you select the profile
        else
        {
            int selectedIndex = dropdownProfiles.indexOfItemId (dropdownProfiles.getSelectedId());
            juce::String profileIdSelected = dropdownProfiles.getItemText (selectedIndex);
            profileId = profileIdSelected;
            cabinEQGraph.setCurve (processor.getCurve (profileIdSelected, currChannel)->get());
            flagFilterChanged();
            applyFilterIfProcessing();
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
        applyFilterIfProcessing();
    }
    else if (button == &unlockButton)
    {
        showForm();
    }
    else if (button == &duplicateButton)
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
        flagFilterChanged();
        applyFilterIfProcessing();
    }
}

void CabinEQPage::didLoadData()
{
    loadDropdownOptions();
    applyFilterIfProcessing();
    processor.setIsProcessing (! isBypassed);
}

void CabinEQPage::timerCallback()
{
    if (! isUnlocked && marketplaceStatus.isUnlocked())
    {
        isUnlocked = true;
        unlockApp();
    }
}

//=========================================
void CabinEQPage::flagFilterChanged()
{
    hasFilterChanged = true;
    bypassButton.setButtonText ("ON*");
}

void CabinEQPage::toggleBypass()
{
    isBypassed = ! isBypassed;
    cabinEQGraph.setGrayscale (isBypassed);
    bypassButton.setButtonText (isBypassed ? "OFF" : "ON");
}

void CabinEQPage::applyFilterIfProcessing()
{
    if (! isBypassed && hasFilterChanged)
    {
        processor.applyCurve (profileId);
        hasFilterChanged = false;
        bypassButton.setButtonText ("ON");
    }
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
}

void CabinEQPage::dismissAlertWindow()
{
    alertWindow->getTextEditor (textEditorName)->removeListener (this);
    alertWindow.reset();
}

void CabinEQPage::showForm()
{
    unlockForm.setVisible (true);
    bypassButton.setEnabled (true);
}

void CabinEQPage::unlockApp()
{
    bypassButton.setEnabled (true);
//    unlockLabel.setText ("Status: Unlocked", juce::dontSendNotification);
//    unlockLabel.setColour (juce::Label::textColourId, juce::Colours::green);
}
