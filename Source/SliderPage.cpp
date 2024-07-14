/*
  ==============================================================================

    SliderPage.cpp
    Created: 9 Jul 2024 10:58:26pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderPage.h"

SliderPage::SliderPage(StartupMVPAudioProcessor& p)
    : processor(p)
{
    std::cout << "Initializing sliders" << std::endl;
    for (int i = 0; i < sliders.size(); ++i)
    {
        sliders[i] = std::make_unique<juce::Slider> (juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
        auto& slider = *sliders[i];
        addAndMakeVisible (sliderContainer);
        sliderContainer.addAndMakeVisible (slider);
        slider.getProperties().set("idx", i);

        auto paramID = "gain_" + std::to_string(i);
        sliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, paramID, slider);
        
        slider.addListener (this);
    }

    viewport.setViewedComponent (&sliderContainer, true);
    addAndMakeVisible (viewport);
    
    addAndMakeVisible (incrementReferenceToneIdxButton);
    addAndMakeVisible (decrementReferenceToneIdxButton);
    incrementReferenceToneIdxButton.addListener (this);
    decrementReferenceToneIdxButton.addListener (this);
    
    addAndMakeVisible (tempoSlider);
    tempoSlider.setRange (10000, 100000);
    tempoSlider.setValue (20000);
    tempoSlider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
    tempoSlider.addListener (this);
    
    startTimer (10);
    std::cout << "finish initializing sliders" << std::endl;
}

SliderPage::~SliderPage()
{
    for (auto& slider : sliders)
    {
        slider->removeListener (this);
    }
    
    incrementReferenceToneIdxButton.removeListener (this);
    decrementReferenceToneIdxButton.removeListener (this);
    tempoSlider.removeListener (this);
}

void SliderPage::resized()
{
    auto area = getLocalBounds();
    int padding = 10;
    
    viewport.setBounds(area);

    int sliderWidth = 50;
    int sliderHeight = area.getHeight();
    int totalWidth = sliders.size() * sliderWidth;

    sliderContainer.setSize(totalWidth + 2 * padding, sliderHeight);

    for (int i = 0; i < sliders.size(); ++i)
    {
        auto& slider = *sliders[i];
        slider.setBounds(padding + i * sliderWidth, 0, sliderWidth, sliderHeight);
    }

    // Set the bounds for the increment and decrement buttons
    int buttonWidth = 50;
    int buttonHeight = 30;
    int buttonMargin = 10;

    incrementReferenceToneIdxButton.setBounds(area.getRight() - buttonWidth - buttonMargin,
                                              area.getY() + buttonMargin,
                                              buttonWidth,
                                              buttonHeight);

    decrementReferenceToneIdxButton.setBounds(area.getRight() - buttonWidth - buttonMargin,
                                              incrementReferenceToneIdxButton.getBottom() + buttonMargin,
                                              buttonWidth,
                                              buttonHeight);

    // Set the bounds for the tempoSlider
    int tempoSliderWidth = 50;
    int tempoSliderHeight = 200; // Adjust the height as needed
    int tempoSliderMargin = 10;

    tempoSlider.setBounds(area.getRight() - tempoSliderWidth - tempoSliderMargin,
                          decrementReferenceToneIdxButton.getBottom() + tempoSliderMargin,
                          tempoSliderWidth,
                          tempoSliderHeight);

    std::cout << "finish resizing" << std::endl;
}

void SliderPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &tempoSlider)
    {
        processor.getSliderCalibrationManager().changeNoteLength (slider->getValue());
        return;
    }
    
    int i = slider->getProperties().getWithDefault("idx", -1);
    processor.getSliderCalibrationManager().setAmplitudeAtIdx (i, slider->getValue());
}

void SliderPage::sliderDragStarted (juce::Slider *slider)
{
    if (slider == &tempoSlider) return;
    
    SliderCalibrationManager& sliderCalibrationManager = processor.getSliderCalibrationManager();
    sliderCalibrationManager.setIsCalibrating (true);
    sliderCalibrationManager.setCurrIdx (slider->getProperties().getWithDefault("idx", -1));
}

void SliderPage::sliderDragEnded (juce::Slider *slider)
{
    if (slider == &tempoSlider) return;
    processor.getSliderCalibrationManager().setIsCalibrating (false);
}

void SliderPage::buttonClicked (juce::Button *button)
{
    if (button == &incrementReferenceToneIdxButton)
    {
        processor.getSliderCalibrationManager().incrementReferenceToneIndex();
    }
    else if (button == &decrementReferenceToneIdxButton)
    {
        processor.getSliderCalibrationManager().decrementReferenceToneIndex();
    }
}

void SliderPage::timerCallback()
{
    int playingIdx = processor.getSliderCalibrationManager().getCurrentlyPlayingIdx();
    
    for (int i = 0; i < sliders.size(); ++i)
    {
        auto& slider = *sliders[i];
        
        if (i == playingIdx)
        {
            slider.setColour (juce::Slider::thumbColourId, juce::Colour::fromRGB(255, 69, 0)); // Orange-Red for active sliders
        }
        else
        {
            slider.setColour (juce::Slider::thumbColourId, juce::Colour::fromRGB(30, 144, 255)); // DodgerBlue for inactive sliders
        }

        slider.setColour(juce::Slider::trackColourId, juce::Colour::fromRGB(70, 70, 70)); // Dark grey track
        slider.setColour(juce::Slider::backgroundColourId, juce::Colour::fromRGB(40, 40, 40)); // Matching dark background
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(50, 50, 50)); // Slightly lighter background for text box
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromRGB(255, 255, 255)); // White text in the text box
    }
}

void SliderPage::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB(30, 30, 30)); // Dark background color
}
