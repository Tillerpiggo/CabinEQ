///*
//  ==============================================================================
//
//    SliderPage.cpp
//    Created: 9 Jul 2024 10:58:26pm
//    Author:  Tyler Gee
//
//  ==============================================================================
//*/
//
//#include "SliderPage.h"
//
//SliderPage::SliderPage(StartupMVPAudioProcessor& p)
//    : processor(p)
//{
//    for (int i = 0; i < sliders.size(); ++i)
//    {
//        sliders[i] = std::make_unique<juce::Slider> (juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
//        auto& slider = *sliders[i];
//        addAndMakeVisible (sliderContainer);
//        sliderContainer.addAndMakeVisible (slider);
//        slider.getProperties().set("idx", i);
//
//        auto paramID = "gain_" + std::to_string(i);
//        sliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, paramID, slider);
//        
//        slider.addListener (this);
//    }
//
//    viewport.setViewedComponent (&sliderContainer, true);
//    addAndMakeVisible (viewport);
//    
//    startTimer (10);
//}
//
//SliderPage::~SliderPage()
//{
//    for (auto& slider : sliders)
//    {
//        slider->removeListener (this);
//    }
//}
//
//void SliderPage::resized()
//{
//    auto area = getLocalBounds();
//    viewport.setBounds(area);
//
//    int sliderWidth = 50;
//    int sliderHeight = area.getHeight();
//    int totalWidth = sliders.size() * sliderWidth;
//
//    sliderContainer.setSize(totalWidth, sliderHeight);
//
//    for (int i = 0; i < sliders.size(); ++i)
//    {
//        auto& slider = *sliders[i];
//        slider.setBounds(i * sliderWidth, 0, sliderWidth, sliderHeight);
//    }
//}
//
//void SliderPage::sliderValueChanged (juce::Slider *slider)
//{
//    int i = slider->getProperties().getWithDefault("idx", -1);
//    processor.getSliderCalibrationManager().setAmplitudeAtIdx (i, slider->getValue());
//}
//
//void SliderPage::sliderDragStarted (juce::Slider *slider)
//{
//    SliderCalibrationManager& sliderCalibrationManager = processor.getSliderCalibrationManager();
//    sliderCalibrationManager.setIsSlidingSlider (true);
//    sliderCalibrationManager.setCurrIdx (slider->getProperties().getWithDefault("idx", -1));
//    
//}
//
//void SliderPage::sliderDragEnded (juce::Slider *slider)
//{
//    processor.getSliderCalibrationManager().setIsSlidingSlider (false);
//}
//
//void SliderPage::timerCallback()
//{
//    std::cout << "timer callback to access sliders" << std::endl;
//    int playingIdx = processor.getSliderCalibrationManager().getCurrentlyPlayingIdx();
//    
//    for (int i = 0; i < sliders.size(); ++i)
//    {
//        std::cout << "sliders at i: " << i << std::endl;
//        if (i == playingIdx)
//        {
//            sliders.at (i)->setColour (juce::Slider::thumbColourId, juce::Colours::red);
//        }
//        else
//        {
//            sliders.at (i)->setColour (juce::Slider::thumbColourId, juce::Colours::blue);
//        }
//    }
//    std::cout << "accessed sliders" << std::endl;
//}
