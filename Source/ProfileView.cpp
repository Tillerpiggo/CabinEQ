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
    addAndMakeVisible (profileList);
    addAndMakeVisible (addProfileButton);
    addAndMakeVisible (titleLabel);

    titleLabel.setText ("Profiles", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::left);

    addProfileButton.onClick = [this] {
//        if (listener != nullptr)
//            listener->addedProfile();
        profileList.setIsAddingProfile (true);
    };

    profileList.setListener (this);
    profileList.setDataSource (this);
    std::cout << "ProfileView constructor called" << std::endl;
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
    layout.addRow ({ Space(&profileList) });
    layout.updateComponentBounds();
}

void ProfileView::setListener (ProfileViewListener* listener)
{
    this->listener = listener;
}

void ProfileView::setDataSource (ProfileViewDataSource* dataSource)
{
    this->dataSource = dataSource;
    profileList.updateContent();
}

void ProfileView::selectedRow (int rowIdx)
{
    if (listener == nullptr || dataSource == nullptr || dataSource->getProfileNames().size() == 0)
        return;
    juce::String profileName = dataSource->getProfileNames()[rowIdx];
    listener->selectProfile (profileName);
}

void ProfileView::duplicateProfile (int rowIdx)
{
    if (listener == nullptr || dataSource == nullptr || dataSource->getProfileNames().size() == 0)
        return;

    // Create the duplicate profile name
    juce::String oldProfileName = dataSource->getProfileNames()[rowIdx];
    juce::String duplicateProfileName = oldProfileName + " copy";

    while (isDuplicateProfileName (duplicateProfileName))
    {
        duplicateProfileName += " copy";
    }

    listener->addDuplicateProfile (duplicateProfileName, oldProfileName);
    profileList.updateContent();
    profileList.scrollToBottom();
}

void ProfileView::renameProfile (int rowIdx, juce::String newProfileName)
{
    if (listener == nullptr || dataSource == nullptr || dataSource->getProfileNames().size() == 0)
        return;
    juce::String oldProfileName = dataSource->getProfileNames()[rowIdx];
    listener->renameProfile (oldProfileName, newProfileName);
    profileList.updateContent();
}

void ProfileView::deleteProfile (int rowIdx)
{
    if (listener == nullptr || dataSource == nullptr || dataSource->getProfileNames().size() == 0)
        return;
    juce::String profileName = dataSource->getProfileNames()[rowIdx];
    listener->deleteProfile (profileName);
    profileList.updateContent();
}

void ProfileView::addProfile (juce::String profileName)
{
    if (listener == nullptr || dataSource == nullptr)
        return;
    listener->addProfile (profileName);
    profileList.updateContent();
}

std::vector<juce::String> ProfileView::getProfileNames()
{
    if (dataSource != nullptr)
        return dataSource->getProfileNames();
    return {};
}

bool ProfileView::getIsProfileLocked (int rowIdx)
{
    if (dataSource != nullptr)
        return dataSource->getIsProfileLocked (rowIdx);
    return false;
}

bool ProfileView::isDuplicateProfileName (juce::String profileName)
{
    if (dataSource == nullptr)
        return false;
    auto profileNames = dataSource->getProfileNames();
    for (const auto& existingProfileName : profileNames)
        if (profileName == existingProfileName)
            return true;
    return false;
}










