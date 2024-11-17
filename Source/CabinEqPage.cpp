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
{
    amplGraph = std::make_unique<CabinPeqGraph>();
    
    addAndMakeVisible (glyphView);
    // TODO: set glyph view listener
    
    // Sliders
    addVerticalSlider (&masterVolumeSlider, -20.0f, 20.0f, 0.0f);
    
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
    
    amplGraph->addListener (this);
    amplGraph->addDataSource (this);
    profileDropdown.addListener (this);
    processor.addListener (this);
    
    // Extra stuff, will clean up later
    addAndMakeVisible (amplGraph.get());
    addAndMakeVisible (profileDropdown);
    
    didLoadData();
    
    glyphView.setListener (this);
    glyphView.setDataSource (this);
    
    setLookAndFeel (&cabinEqLookAndFeel);
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
    
    
    juce::ColourGradient fadeGradient (BACKGROUND_GRADIENT_LIGHT, 0, 0, // Bottom
        BACKGROUND_GRADIENT_DARK, 900, 300, // Top edge
        true);
    
    g.setGradientFill(fadeGradient);
    g.fillRect(getLocalBounds());
}

void CabinEqPage::resized()
{
    float sidebarWidth = 120.0f;
    
    Layout layout (getBounds(), 8.0f);
    layout.addRow ({ Space (&bypassButton).withFixedSize (50), Space (&profileDropdown), Space (sidebarWidth) }, 40);
    layout.addRow ({ Space (amplGraph.get()), Space (&masterVolumeSlider).withFixedSize (sidebarWidth) }, 0.6);
    layout.addRow ({ Space (&glyphView) });
    layout.updateComponentBounds();
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

void CabinEqPage::setSpeed (float speedFactor)
{
    processor.setSpeedFactor (speedFactor);
}

void CabinEqPage::setBandwidth (float bandwidth)
{
    processor.setBandwidth (bandwidth);
}

void CabinEqPage::setIsPlaying (bool isPlaying)
{
    processor.setIsPlaying (isPlaying);
}

void CabinEqPage::goToNextGlyph()
{
    processor.goToNextGlyph();
}

void CabinEqPage::goToPrevGlyph()
{
    processor.goToPrevGlyph();
}

// GlyphView::DataSource
Glyph CabinEqPage::getCurrGlyph()
{
    return processor.getCurrGlyph();
}

bool CabinEqPage::hasNextGlyph()
{
    return processor.hasNextGlyph();
}

bool CabinEqPage::hasPrevGlyph()
{
    return processor.hasPrevGlyph();
}

void CabinEqPage::setSizeFactor (float sizeFactor)
{
    processor.setSizeFactor (sizeFactor);
}

void CabinEqPage::setCenterPos (juce::Point<float> centerPos)
{
    processor.setCenterPos (centerPos);
}

float CabinEqPage::getSizeFactor()
{
    return processor.getSizeFactor();
}

juce::Point<float> CabinEqPage::getCenterPos()
{
    return processor.getCenterPos();
}

float CabinEqPage::getCurrPlayingTime()
{
    return processor.getCurrPlayingTime();
}


// ====================================================
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
}

void CabinEqPage::inputAttemptWhenModal()
{
    dismissAlertWindow();
}

void CabinEqPage::didLoadData()
{
    addingFirstProfile = true;
    processor.setIsFilterOn (! isBypassed);
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
