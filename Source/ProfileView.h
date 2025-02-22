/*
  ==============================================================================

    ProfileView.h
    Created: 21 Feb 2025 9:45:57pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Listeners.h"
#include "BuildableComponent.h"
#include "Layout.h"
#include "ProfileList.h"

// This class hosts a profileList and allows users to also add profiles to the list
class ProfileView  : public BuildableComponent,
                     public ProfileListListener,
                     public ProfileListDataSource
{
public:
    ProfileView();

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setListener (ProfileViewListener* listener);
    void setDataSource (ProfileViewDataSource* dataSource);

    // ProfileListListener methods
    void selectedRow (int rowIdx) override;
    void duplicateProfile (int rowIdx) override;
    void renameProfile (int rowIdx) override;
    void deleteProfile (int rowIdx) override;

    std::vector<juce::String> getProfileNames() override;
    bool getIsProfileLocked (int rowIdx) override;

private:
    ProfileViewListener* listener = nullptr;
    ProfileViewDataSource* dataSource = nullptr;

//    ProfileList profileList;
    juce::Label titleLabel { "Profiles" };
    juce::TextButton addProfileButton { "Add Profile" };
};
