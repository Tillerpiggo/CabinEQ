/*
  ==============================================================================

    Slider.h
    Created: 27 Jun 2024 5:22:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

//#pragma once
//
//#include <JuceHeader.h>
//
//constexpr float pi = 3.14159265358979323846;
//
//class SliderGroup : public juce::Component
//{
//public:
//    SliderGroup(juce::AudioProcessorValueTreeState& parameters, const int i)
//    {
//        std::string idx = std::to_string(i);
//        
//        addSlider (gainSlider, gainLabel, "Gain Slider " + idx, "gain_" + idx, parameters, i, -24.0f, 48.0f);
//        addSlider (panSlider, panLabel, "Balance Slider " + idx, "pan_" + idx, parameters, i + SetPointManager::NUM_SET_POINTS, -24.0f, 24.0f);
//        addSlider (phaseSlider, phaseLabel, "Phase Slider " + idx, "phase_" + idx, parameters, i + 2 * SetPointManager::NUM_SET_POINTS, -1 * pi, pi);
//    }
//    
//    void resized() override
//    {
//        auto area = getLocalBounds();
//        int padding = 10;
//        int labelHeight = 20;
//        int sliderHeight = 60;
//        int totalComponentsHeight = 3 * (labelHeight + sliderHeight) + 2 * padding;
//
//        // Calculate the available height for each set of label and slider
//        int availableHeight = area.getHeight() - totalComponentsHeight;
//        int topPadding = availableHeight / 2;
//
//        // Position each slider and label with padding
//        gainLabel.setBounds(area.removeFromTop(labelHeight).reduced(padding));
//        gainSlider.setBounds(area.removeFromTop(sliderHeight).reduced(padding));
//        area.removeFromTop(padding);
//
//        panLabel.setBounds(area.removeFromTop(labelHeight).reduced(padding));
//        panSlider.setBounds(area.removeFromTop(sliderHeight).reduced(padding));
//        area.removeFromTop(padding);
//
//        phaseLabel.setBounds(area.removeFromTop(labelHeight).reduced(padding));
//        phaseSlider.setBounds(area.removeFromTop(sliderHeight).reduced(padding));
//    }
//    
//    void addSliderListener(juce::Slider::Listener* listener)
//    {
//        gainSlider.addListener (listener);
//        panSlider.addListener (listener);
//        phaseSlider.addListener (listener);
//    }
//    
//    void removeSliderListener(juce::Slider::Listener* listener)
//    {
//        gainSlider.removeListener (listener);
//        panSlider.removeListener (listener);
//        phaseSlider.removeListener (listener);
//    }
//
//private:
//    void addSlider(juce::Slider& slider, juce::Label& label, const std::string& name, const std::string& paramName,
//                   juce::AudioProcessorValueTreeState& parameters, const int i, const float lower, const float higher)
//    {
//        slider.setSliderStyle (juce::Slider::LinearHorizontal);
//        slider.setRange (lower, higher, 0.1f);
//        slider.getProperties().set ("index", i);
//        addAndMakeVisible (slider);
//
//        label.setText (name, juce::dontSendNotification);
//        label.attachToComponent (&slider, true);
//        addAndMakeVisible (label);
//
//        attachments.emplace_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
//            parameters, paramName, slider));
//    }
//
//
//    juce::Slider gainSlider;
//    juce::Slider panSlider;
//    juce::Slider phaseSlider;
//    
//    juce::Label gainLabel;
//    juce::Label panLabel;
//    juce::Label phaseLabel;
//
//    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
//};
