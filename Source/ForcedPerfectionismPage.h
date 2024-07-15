/*
  ==============================================================================

    ForcedPerfectionismPage.h
    Created: 14 Jul 2024 11:14:06am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ForcedPerfectionismPage   : public juce::Component,
                                  public juce::Slider::Listener,
                                  public juce::Button::Listener,
                                  public juce::Timer
{
public:
    ForcedPerfectionismPage (StartupMVPAudioProcessor& p);
    ~ForcedPerfectionismPage() override;

    void resized() override;
    void paint (juce::Graphics& g) override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void sliderDragStarted (juce::Slider *slider) override;
    void sliderDragEnded (juce::Slider *slider) override;
    
    void buttonClicked (juce::Button *button) override;
    
    void timerCallback() override;
    
private:
    void parameterChangedCallback (float newValue, int idx);
    void incrementCurrValueBy (float increment);
    
    StartupMVPAudioProcessor& processor;
    std::array<std::unique_ptr<juce::ParameterAttachment>, SliderSetPointManager::NUM_PTS> parameterAttachments;
    
    juce::Rectangle<float> circleOne; // for the reference tone
    juce::Rectangle<float> circleTwo; // for the controlled tone
    juce::Slider slider;
    juce::TextButton largePlusButton { "++" };
    juce::TextButton smallPlusButton { "+" };
    juce::TextButton smallMinusButton { "-" };
    juce::TextButton largeMinusButton { "--" };
    juce::TextButton toggleCalibrationButton { "Start Calibrating" };
    juce::TextButton nextButton { "Next >" };
    juce::Label numCompletedLabel;
    
    juce::Colour circleColorOff = juce::Colours::lightblue;
    juce::Colour circleColorOn = juce::Colours::blue;
    juce::Colour backgroundColor = juce::Colours::white;
    
    int currIdx = 0;
    int currPlayingCircle = -1;
    bool isCalibrating = false;
};
