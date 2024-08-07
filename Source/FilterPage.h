/*
  ==============================================================================

    FilterPage.h
    Created: 11 Jul 2024 11:53:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// This page displays the curve of the current filter, contains a slider to adjust the balance of volumes
// and and buttons to apply the filter and bypass it
class FilterPage   : public juce::Component,
                     public juce::Slider::Listener,
                     public juce::Button::Listener
{
public:
    FilterPage(StartupMVPAudioProcessor& p);
    ~FilterPage() override;
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void buttonClicked (juce::Button *button) override;
    
private:
    void updateApplyFilterButtonText();
    
    StartupMVPAudioProcessor& processor;
    
    juce::TextButton applyFilterButton { "HeadphoneEQ Inactive" };
    juce::TextButton bypassButton { "Bypass" };
    juce::Slider balanceSlider;
    
    bool isBypassed;
    bool isHeadphoneEQSelected;
};
