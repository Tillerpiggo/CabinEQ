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
//    panGraph = std::make_unique<CabinEqGraph>();
//    phaseGraph = std::make_unique<CabinEqGraph>();
    
//    auto backgroundColor = juce::Colour::fromRGB (0.1, 0.1, 0.2); // DRY violation; redundant w/ CabinEqGraph BACKGROUND_COLOR
//    graphs.addTab ("Volume", backgroundColor, amplGraph.get(), false);
//    graphs.addTab ("Left/Right", backgroundColor, panGraph.get(), false);
//    graphs.addTab ("Phase", backgroundColor, phaseGraph.get(), false);
    
//    filterQualityDropdown.addItem ("Utopian", 1);
//    filterQualityDropdown.addItem ("Fantastic", 2);
//    filterQualityDropdown.addItem ("Great", 3);
//    filterQualityDropdown.addItem ("Good", 4);
//    filterQualityDropdown.addItem ("Economy", 5);
//    filterQualityDropdown.setSelectedId (3);
//    profileDropdown.addItem ("+ Add Profile", 1);
    
    volumeSlider.setRange (-20.0f, 20.0f);
    volumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    
    amplGraph->addListener (this);
//    panGraph->addListener (this);
//    phaseGraph->addListener (this);
    profileDropdown.addListener (this);
    filterQualityDropdown.addListener (this);
    bypassButton.addListener (this);
    processor.addListener (this);
    volumeSlider.addListener (this);
    
    addAndMakeVisible (amplGraph.get());
//    addAndMakeVisible (graphs);
    addAndMakeVisible (profileDropdown);
    addAndMakeVisible (bypassButton);
    addAndMakeVisible (volumeSlider);
    
    didLoadData();
}

CabinEqPage::~CabinEqPage()
{
    profileDropdown.removeListener (this);
//    filterQualityDropdown.removeListener (this);
    bypassButton.removeListener (this);
    volumeSlider.removeListener (this);
    
    amplGraph->removeListener();
//    panGraph->removeListener();
//    phaseGraph->removeListener();
    
    processor.removeListener();
}

void CabinEqPage::paint (juce::Graphics& g)
{
//    g.fillAll (colorTheme->getBarBackgroundColor());
}

void CabinEqPage::resized()
{
    int padding = 20; // padding on the top and bottom
    int componentPadding = 10; // padding between graph, slider, and dropdown
    int dropdownHeight = 30;
    int sliderHeight = 30;
    int labelHeight = 15;
    int buttonWidth = 100;
    int applyButtonWidth = 100;
    int duplicateButtonWidth = 100;
    int toggleButtonHeight = 30;
    int totalButtonWidth = buttonWidth + duplicateButtonWidth + applyButtonWidth;

    // Get heights for each component
    int availableHeight = getHeight() - (2 * padding);
    int graphHeight = availableHeight - dropdownHeight - sliderHeight - toggleButtonHeight - labelHeight - 4 * componentPadding;
    
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
    int currentX = padding + dropdownWidth + buttonWidth;
    bypassButton.setBounds (padding + dropdownWidth, buttonsY, buttonWidth, dropdownHeight);

    // Set bounds for sliders
    int sliderY = buttonsY + dropdownHeight + componentPadding;
    int sliderWidth = (getWidth() - (3 * padding)); // Two sliders with padding in between
    volumeSlider.setBounds (padding, sliderY, sliderWidth, sliderHeight);
}

// ====================================================
int CabinEqPage::addBand (float freq, float ampl, float bandwidth, CabinPeqGraph* sender)
{
    std::cout << "Listener got adding band" << std::endl;
    if (sender == amplGraph.get())
    {
        int addedBandId = processor.addBand (freq, ampl, bandwidth, profileId);
        amplGraph->setBands (processor.getBands (profileId));
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
        amplGraph->setBands (processor.getBands (profileId));
        processor.updateFilter (profileId);
    }
}

void CabinEqPage::removeBand (int id, CabinPeqGraph* sender)
{
    if (sender == amplGraph.get())
    {
        processor.removeBand (id, profileId);
        amplGraph->setBands (processor.getBands (profileId));
        processor.updateFilter (profileId);
    }
}

void CabinEqPage::startNoisePatternAt (int id, CabinPeqGraph* sender)
{
    if (sender == amplGraph.get())
    {
        processor.startNoisePatternAt (id, profileId);
    }
}

void CabinEqPage::updateNoisePatternAt (int id, CabinPeqGraph* sender)
{
    if (sender == amplGraph.get())
    {
        processor.updateNoisePatternAt  (id, profileId);
    }
}

void CabinEqPage::stopNoisePattern()
{
    processor.stopNoisePattern();
}

void CabinEqPage::setNoisePatternSolo (bool solo)
{
    processor.setNoisePatternSolo (solo);
}

// ====================================================
void CabinEqPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &volumeSlider)
    {
        processor.setVolume (slider->getValue());
    }
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
    bypassButton.setButtonText (isBypassed ? "OFF" : (hasFilterChanged ? "ON*" : "ON"));
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
    amplGraph->setBands (processor.getBands (profileId));
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
