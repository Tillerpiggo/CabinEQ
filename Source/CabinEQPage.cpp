/*
  ==============================================================================

    CabinEQPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQPage.h"

CabinEQPage::CabinEQPage (StartupMVPAudioProcessor& p)
    : processor (p), profileId ("NO_PROFILE"), cabinEQGraph(),
      alertWindow ("Add Profile", "Enter your profile name", juce::MessageBoxIconType::NoIcon)
{
    dropdownProfiles.addItem ("+ Add Profile", 1);
    
    referenceSlider.setRange (-24.0f, 24.0f);
    referenceSlider.setValue (0.0f);
    
    alertWindow.addTextEditor (textEditorName, "");
    alertWindow.getTextEditor (textEditorName)->addListener (this);
    alertWindow.setEscapeKeyCancels (true);
    
    cabinEQGraph.addListener (this);
    dropdownProfiles.addListener (this);
    referenceSlider.addListener (this);
    processor.addListener (this);
    
    addAndMakeVisible (cabinEQGraph);
    addAndMakeVisible (dropdownProfiles);
    addAndMakeVisible (referenceSlider);
    
    didLoadData();
}

CabinEQPage::~CabinEQPage()
{
    referenceSlider.removeListener (this);
    alertWindow.getTextEditor (textEditorName)->removeListener (this);
    cabinEQGraph.removeListener();
}

void CabinEQPage::paint (juce::Graphics& g)
{
    g.fillAll (backgroundColor);
}

void CabinEQPage::resized()
{
    int padding = 10;
    int graphHeight = getHeight() - (2 * padding) - 30; // Adjust for dropdown height

    cabinEQGraph.setBounds(0, 0, getWidth(), graphHeight);
    dropdownProfiles.setBounds(padding, getHeight() - padding - 30, getWidth() - (2 * padding), 30);
}

// ====================================================
int CabinEQPage::addNode (float freq, float ampl)
{
    return processor.addEQNode (freq, ampl, 0.0f, profileId);
}

void CabinEQPage::updateNode (int id, float freq, float ampl)
{
    processor.updateEQNode (id, freq, ampl, 0.0f, profileId);
}

void CabinEQPage::removeNode (int id)
{
    processor.removeEQNode (id, profileId);
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
    processor.startTestingAt (freq, profileId);
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
    // Add the profile and dismiss the window
    juce::String profileName = textEditor.getText();
    
    // Add & retrieve profile from processor
    processor.addProfile (profileName);
    loadDropdownOptions();
    
    // Select the new profile
    dropdownProfiles.setSelectedId (dropdownProfiles.getItemId (dropdownProfiles.getNumItems() - 2));
    
    alertWindow.exitModalState();
}

void CabinEQPage::comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &dropdownProfiles)
    {
        // Add a profile if you select "+ Add Profile"
        if (dropdownProfiles.getSelectedId() == dropdownProfiles.getNumItems())
        {
            alertWindow.enterModalState();
        }
        
        // Go to a profile if you select the profile
        else
        {
            int selectedIndex = dropdownProfiles.indexOfItemId (dropdownProfiles.getSelectedId());
            juce::String profileIdSelected = dropdownProfiles.getItemText (selectedIndex);
            profileId = profileIdSelected;
            cabinEQGraph.setCurve (processor.getCurve (profileIdSelected)->get());
        }
    }
}

void CabinEQPage::didLoadData()
{
    loadDropdownOptions();
//    dropdownProfiles.clear();
//    dropdownProfiles.addItem ("+ Add Profile", 1);
//    
//    // Add existing profiles to dropdown menu
//    int i = 2;
//    for (const auto& name : processor.getProfileNames())
//    {
//        dropdownProfiles.addItem (name, i);
//        i++;
//    }
//    
//    if (dropdownProfiles.getNumItems() > 1)
//        dropdownProfiles.setSelectedId (dropdownProfiles.getItemId (2));
}

//=========================================
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
