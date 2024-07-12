/*
  ==============================================================================

    SliderPage.h
    Created: 9 Jul 2024 10:58:26pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SliderPage   : public juce::Component,
                     public juce::Slider::Listener,
                     public juce::Timer
{
public:
    SliderPage(StartupMVPAudioProcessor& p);
    ~SliderPage() override;

    void resized() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void sliderDragStarted (juce::Slider *slider) override;
    void sliderDragEnded (juce::Slider *slider) override;
    
    void timerCallback() override;

private:
    StartupMVPAudioProcessor& processor;

    juce::Viewport viewport;
    juce::Component sliderContainer;
    std::array<std::unique_ptr<juce::Slider>, 23> sliders;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 23> sliderAttachments;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderPage)
};
