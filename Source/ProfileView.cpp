/*
  ==============================================================================

    ProfileView.cpp
    Created: 21 Feb 2025 9:45:57pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ProfileView.h"

ProfileView::ProfileView()
{
    // addAndMakeVisible (profileList);
    addAndMakeVisible (addProfileButton);
    addAndMakeVisible (titleLabel);

    titleLabel.setText ("Profiles", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::left);

    addProfileButton.setButtonText ("Add Profile");
    addProfileButton.onClick = [this] {
        if (listener != nullptr)
            listener->addProfile();
    };

    // profileList.setListener (this);
    // profileList.setDataSource (this);
}

void ProfileView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void ProfileView::resized()
{
    Layout layout (getBounds().withX (0).withY (0), 8.0f);
    layout.addRow ({ Space(&titleLabel) }, 20.0f);
    layout.addRow ({ Space(&addProfileButton) }, 40.0f);
    // layout.addRow ({ Space(&profileList) });
    layout.updateComponentBounds();
}

void ProfileView::setListener (ProfileViewListener* listener)
{
    this->listener = listener;
}

void ProfileView::setDataSource (ProfileViewDataSource* dataSource)
{
    this->dataSource = dataSource;
}

void ProfileView::selectedRow (int rowIdx)
{
//    if (listener != nullptr)
//        listener->selectedRow (rowIdx);
}

void ProfileView::duplicateProfile (int rowIdx)
{
    if (listener != nullptr)
        listener->duplicateProfile (rowIdx);
}

void ProfileView::renameProfile (int rowIdx)
{
    if (listener != nullptr)
        listener->renameProfile (rowIdx);
}

void ProfileView::deleteProfile (int rowIdx)
{
    if (listener != nullptr)
        listener->deleteProfile (rowIdx);
}

std::vector<BandProfile> ProfileView::getProfiles()
{
    if (dataSource != nullptr)
        return dataSource->getProfiles();
    return {};
}

bool ProfileView::getIsProfileLocked (int rowIdx)
{
    if (dataSource != nullptr)
        return dataSource->getIsProfileLocked (rowIdx);
    return false;
}












