/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CurveComponent.h"
#include "SliderGroup.h"
#include "ClearSoundLookAndFeel.h"
#include "CircleComponent.h"

//==============================================================================
/**
*/
class StartupMVPAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        public juce::Button::Listener,
                                        public juce::Timer,
                                        public juce::Slider::Listener
{
public:
    StartupMVPAudioProcessorEditor (StartupMVPAudioProcessor&);
    ~StartupMVPAudioProcessorEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void buttonClicked (juce::Button *button) override;
    void sliderValueChanged (juce::Slider *slider) override;
    
private:
    juce::Slider& addSlider (std::string name, std::string paramName, int idx);
    void timerCallback() override;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    StartupMVPAudioProcessor& audioProcessor;
    
    CurveComponent curveComponent;
    juce::ToggleButton toggleCalibrationButton { "TOGGLE CALIBRATION" };
    juce::TextButton lowerPreferredButton { "Lower Preferred" };
    juce::TextButton higherPreferredButton { "Higher Preferred" };
    juce::TextButton theSameButton { "They're the same, about " };
    juce::TextButton bypassButton { "BYPASS" };
    juce::TextButton applyCurveButton { "APPLY CURVE" };
    
    CircleComponent redCircle;
    CircleComponent blueCircle;
    
    juce::Slider referenceSlider;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartupMVPAudioProcessorEditor)
};
