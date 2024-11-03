/*
  ==============================================================================

    StepView.h
    Created: 30 Oct 2024 2:36:57pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Layout.h"
#include "BuildableComponent.h"
#include "QualityStep.h"

// This provides a UI to adjust a single Step, which for now is just a difficulty slider, a button to play the reference audio, and instruction text
class StepView  : public BuildableComponent,
                  public juce::Timer
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        
        virtual void setDifficulty (float difficulty) = 0;
        virtual void setOctaveShift (float octaveShift) = 0;
        virtual void setIsPlaying (bool isPlaying) = 0;
        virtual void setIsCycling (bool isCycling) = 0;
        virtual void setStage (int stage) = 0;
        virtual void goToNextStep() = 0;
        virtual void goToPrevStep() = 0;
    };
    
    class DataSource
    {
    public:
        virtual ~DataSource() = default;
        virtual int getCurrStage() = 0;
    };
    
    StepView();
    ~StepView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (Listener* listener);
    void setDataSource (DataSource* dataSource);
    void updateWithQualityStep (QualityStep qualityStep);
    void setStage (int stage);
    
    void timerCallback() override;
    
private:
    Listener* listener = nullptr;
    DataSource* dataSource = nullptr;
    
    juce::Slider difficultySlider;
    juce::Label difficultySliderLabel;
    juce::Slider pitchSlider;
    juce::Label pitchSliderLabel;
    juce::TextButton playButton { "Play" };
    juce::TextButton prevButton { "PREV" };
    juce::TextButton nextButton { "NEXT" };
    std::vector<std::unique_ptr<juce::TextButton>> stageButtons;
    
    bool isPlaying = false;
    bool isCycling = false;
    int stage = -1;
    
    void updateStageButtonColours();
};
