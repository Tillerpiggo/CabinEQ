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
        if (listener != nullptr) listener->setDifficulty (difficultySlider.getValue());
    });
    addButtonAction (&playButton, [this](juce::Button*) {
        if (listener != nullptr) listener->playReferencePattern();
    });
    addButtonAction (&prevButton, [this](juce::Button*) {
        if (listener != nullptr) listener->goToPrevStep();
    });
    addButtonAction (&nextButton, [this](juce::Button*) {
        if (listener != nullptr) listener->goToNextStep();
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
    layout.layoutComponentsInGrid({ { &difficultySlider, &playButton },
                                    { &prevButton, &nextButton }});
}
