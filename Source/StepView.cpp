/*
  ==============================================================================

    StepView.cpp
    Created: 30 Oct 2024 2:36:57pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "StepView.h"

StepView::StepView()
{
    difficultySlider.setRange (0.0f, 1.0f);
    difficultySlider.setValue (0.5f);
    difficultySlider.setSliderStyle (juce::Slider::LinearHorizontal);
    
    difficultySlider.addListener (this);
    playButton.addListener (this);
    prevButton.addListener (this);
    nextButton.addListener (this);
    
    addAndMakeVisible (difficultySlider);
    addAndMakeVisible (playButton);
    addAndMakeVisible (prevButton);
    addAndMakeVisible (nextButton);
}

StepView::~StepView()
{
    
}

void StepView::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::red);
}

void StepView::resized()
{
    Layout layout (getBounds(), 4);
    layout.layoutComponentsInGrid({ { &difficultySlider, &playButton },
                                    { &prevButton, &nextButton }});
}

void StepView::sliderValueChanged (juce::Slider *slider)
{
    if (listener == nullptr)
        return;
    
    if (slider == &difficultySlider)
    {
        listener->setDifficulty (difficultySlider.getValue());
    }
}

void StepView::buttonClicked (juce::Button *button)
{
    if (listener == nullptr)
        return;
    
    if (button == &playButton)
    {
        listener->playReferencePattern();
    }
    else if (button == &prevButton)
    {
        listener->goToPrevStep();
    }
    else if (button == &nextButton)
    {
        listener->goToNextStep();
    }
    
}
