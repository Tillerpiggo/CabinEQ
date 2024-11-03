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
    addSliderAndLabel (&difficultySlider, &difficultySliderLabel, "Difficulty", 0.0f, 1.0f, 0.5f);
    addButton (&playButton);
    addButton (&prevButton);
    addButton (&nextButton);
    
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
    
    startTimer (10);
}

StepView::~StepView()
{
    stopTimer();
}

void StepView::timerCallback()
{
    if (dataSource != nullptr)
        setStage (dataSource->getCurrStage());
}

void StepView::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::red);
}

void StepView::resized()
{
    Layout layout (getBounds(), 4);
    
    // difficultySlider + playButton
    layout.addRowWithRectWidths ({ FlexibleLayoutDimension::fixed (80), FlexibleLayoutDimension::fill(), FlexibleLayoutDimension::fixed (80) });
    
    // stageButtons
    layout.addRowWithRectWidths(std::vector<FlexibleLayoutDimension>(stageButtons.size() + 2, FlexibleLayoutDimension::fill()));
    
    // prevButton + nextButton
    layout.addRowWithRectWidths ({ FlexibleLayoutDimension::fill(), FlexibleLayoutDimension::fixed (80), FlexibleLayoutDimension::fixed (80), FlexibleLayoutDimension::fill() }); // prevButton + nextButton
    
    // apply to components
    difficultySlider.setBounds (layout.getBoundsAt (0, 1));
    playButton.setBounds (layout.getBoundsAt (0, 2));
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

void StepView::setDataSource (DataSource* dataSource)
{
    this->dataSource = dataSource;
}

void StepView::updateWithQualityStep (QualityStep qualityStep)
{
    // Remove the current stage buttons
    for (int i = 0; i < stageButtons.size(); ++i)
    {
        removeButton (stageButtons[i].get());
    }
    stageButtons.clear();
    
    // Add in the new stage buttons with appropriate actions, based on the number of stages in the quality step
    for (int i = 0; i < qualityStep.getNumStages(); ++i)
    {
        stageButtons.push_back (std::make_unique<juce::TextButton> ("Stage " + std::to_string (i + 1)));
        addButton (stageButtons[i].get());
        addButtonAction (stageButtons[i].get(), [this, i](juce::Button* buttonPtr) {
            listener->setStage (i);
        });
    }
    
    // Add a final button
    stageButtons.push_back (std::make_unique<juce::TextButton> ("CYCLE"));
    addButton (stageButtons[stageButtons.size() - 1].get());
    addButtonAction (stageButtons[stageButtons.size() - 1].get(), [this](juce::Button* buttonPtr) {
        isCycling = ! isCycling;
        listener->setIsCycling (isCycling);
    });
    
    resized();
}

void StepView::setStage (int stage)
{
    if (stage != this->stage)
    {
        this->stage = stage;
        updateStageButtonColours();
    }
}

void StepView::updateStageButtonColours()
{
    for (int i = 0; i < stageButtons.size() - 1; ++i)
    {
        juce::Colour colour = (stage == i) ? juce::Colours::lightblue : juce::Colours::blue;
        stageButtons[i]->setColour (juce::TextButton::buttonColourId, colour);
    }
}
