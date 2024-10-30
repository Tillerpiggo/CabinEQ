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
    
}

void StepView::resized()
{
    int padding = 20;
    int componentY = getHeight() / 2.0f;
    int componentHeight = 40;
    
    // Find slider + button dimensions
    int availableWidth = getWidth() - 2 * padding;
    int sliderWidth = availableWidth * 3.0f / 4.0f;
    int buttonWidth = availableWidth * 1.0f / 4.0f - padding / 2.0f;
    
    // Place slider + button on screen
    difficultySlider.setBounds (padding, componentY, sliderWidth, componentHeight);
    playButton.setBounds (getWidth() - padding - buttonWidth, componentY, buttonWidth, componentHeight);
}

void StepView::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &difficultySlider && listener != nullptr)
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
