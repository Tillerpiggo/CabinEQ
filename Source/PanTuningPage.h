/*
  ==============================================================================

    PanTuningPage.h
    Created: 13 Jul 2024 3:48:22pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PanTuningPage  : public juce::Component,
                       public juce::Button::Listener,
                       public juce::Timer
{
public:
    PanTuningPage(StartupMVPAudioProcessor& p);
    ~PanTuningPage() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setCircleLitUp(int index);

    void buttonClicked (juce::Button *button) override;
    void timerCallback() override;

private:
    void parameterChangedCallback (float newValue, int idx);
    int sliderIndexForTuningIndex (int tuningIdx);
    
    StartupMVPAudioProcessor& processor;
    
    int litUpIndex = -1;
    bool isCalibrating = false;
    
    std::array<juce::TextButton, 4> plusButtons;
    std::array<juce::TextButton, 4> minusButtons;
    std::array<juce::Rectangle<float>, 4> circles;
    juce::TextButton toggleCalibrationButton { "Start Calibrating" };
    juce::TextButton nextButton { "Next >" };
    std::array<std::unique_ptr<juce::ParameterAttachment>, SliderSetPointManager::NUM_PTS> sliderAttachments;
    
    juce::Colour circleColorOff = juce::Colours::lightblue;
    juce::Colour circleColorOn = juce::Colours::blue;
    juce::Colour backgroundColor = juce::Colours::white;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanTuningPage)
};