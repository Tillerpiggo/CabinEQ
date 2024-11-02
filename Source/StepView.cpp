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
    // Add components
    addSliderAndLabel (difficultySlider, difficultySliderLabel, "Difficulty", 0.0f, 1.0f, 0.5f);
    addButton (playButton);
    addButton (prevButton);
    addButton (nextButton);
    
    // Add actions
    addSliderAction (&difficultySlider, [this](juce::Slider*) {
        if (listener != nullptr)
            listener->setDifficulty (difficultySlider.getValue());
    });
    addButtonAction (&playButton, [this](juce::Button*) {
        isPlaying = ! isPlaying;
        if (listener != nullptr)
            listener->setIsPlaying (isPlaying);
    });
    addButtonAction (&prevButton, [this](juce::Button*) {
        if (listener != nullptr)
            listener->goToPrevStep();
    });
    addButtonAction (&nextButton, [this](juce::Button*) {
        if (listener != nullptr)
            listener->goToNextStep();
    });
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
    
    // difficultySlider + playButton
    layout.addRowWithRectWidths ({ FlexibleLayoutDimension::fill(), FlexibleLayoutDimension::fixed (80) });
    
    // stageButtons
    layout.addRowWithRectWidths(std::vector<FlexibleLayoutDimension>(stageButtons.size() + 2, FlexibleLayoutDimension::fill()));
    
    // prevButton + nextButton
    layout.addRowWithRectWidths ({ FlexibleLayoutDimension::fill(), FlexibleLayoutDimension::fixed (80), FlexibleLayoutDimension::fixed (80), FlexibleLayoutDimension::fill() }); // prevButton + nextButton
    
    // apply to components
    difficultySlider.setBounds (layout.getBoundsAt (0, 0));
    playButton.setBounds (layout.getBoundsAt (0, 1));
    for (int i = 0; i < stageButtons.size(); ++i)
    {
        stageButtons[i]->setBounds (layout.getBoundsAt (1, i + 1));
    }
    prevButton.setBounds (layout.getBoundsAt (2, 1));
    nextButton.setBounds (layout.getBoundsAt (2, 2));
}

void StepView::setListener (Listener* listener)
{
    this->listener = listener;
}

void StepView::updateWithQualityStep (QualityStep qualityStep)
{
    // TODO - update button row with appropriate # of stages in this quality step
}
