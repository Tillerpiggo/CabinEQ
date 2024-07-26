/*
  ==============================================================================

    EQProfilePage.h
    Created: 25 Jul 2024 9:00:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EQNodeController.h"

/// This page lets the user manage a list of EQNodes. They can use it to add, maodify, or remove EQNodes via sliders.
class EQProfilePage   : public juce::Component,
                        public EQNodeControllerListener,
                        public juce::Button::Listener
{
public:
    EQProfilePage (StartupMVPAudioProcessor& p);
    ~EQProfilePage() override;
    
    void resized() override;
    void paint (juce::Graphics& g) override;
    
    void eqNodeChanged (int id, float frequency, float amplitude, float pan) override;
    void removeButtonClicked (int id) override;
    void buttonClicked (juce::Button *button) override;
    
private:
    void updateUIWithEQNodes (const std::vector<EQNode>& eqNodes);
    
    StartupMVPAudioProcessor& processor;
    
    juce::TextButton addEQNodeButton { "Add Node" };
    juce::Viewport viewport;
    juce::Component eqNodeControllerContainer; // contains the list of all eqNodeControllers
    std::vector<std::unique_ptr<EQNodeController>> eqNodeControllers;
};
