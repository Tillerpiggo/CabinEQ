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

// This provides a UI to adjust a single Step, which for now is just a difficulty slider, a button to play the reference audio, and instruction text
class StepView  : public juce::Component,
                  public juce::Slider::Listener,
                  public juce::Button::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        
        virtual void setDifficulty (float difficulty) = 0;
        virtual void playReferencePattern() = 0;
        virtual void goToNextStep() = 0;
        virtual void goToPrevStep() = 0;
    };
    
    StepView();
    ~StepView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void buttonClicked (juce::Button *button) override;
    
    void setListener (Listener* listener);
    
private:
    Listener* listener = nullptr;
    
    juce::Slider difficultySlider;
    juce::TextButton playButton { "Play" };
    juce::TextButton prevButton { "PREV" };
    juce::TextButton nextButton { "NEXT" };
};
