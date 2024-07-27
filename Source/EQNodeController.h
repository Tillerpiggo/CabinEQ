/*
  ==============================================================================

    EQNodeController.h
    Created: 25 Jul 2024 9:00:37pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EQNode.h"

/// Any class that wants to act on user input given to an EQNodeController must implement this, or the EQNodeController won't do anything.
class EQNodeControllerListener
{
public:
    virtual ~EQNodeControllerListener() = default;

    virtual void eqNodeStartedChange (int id, float frequency, float amplitude, float pan) = 0;
    virtual void eqNodeChanged (int id, float frequency, float amplitude, float pan) = 0;
    virtual void eqNodeEndedChange (int id, float frequency, float amplitude, float pan) = 0;
    virtual void removeButtonClicked (int id) = 0;
};

/// This controls a single EQNode with three sliders - one for frequency, one for amplitude, and one for pan. It also has a remove button that lets you remove the node.
class EQNodeController   : public juce::Component,
                           public juce::Slider::Listener,
                           public juce::Button::Listener
{
public:
    EQNodeController (int id, float frequency, float amplitude, float pan);
    EQNodeController (EQNode eqNode);
    ~EQNodeController() override;

    void resized() override;
    void paint (juce::Graphics& g) override;
    
    void sliderDragStarted (juce::Slider *slider) override;
    void sliderValueChanged (juce::Slider *slider) override;
    void sliderDragEnded (juce::Slider *slider) override;
    void buttonClicked (juce::Button *button) override;
    
    void setListener (EQNodeControllerListener *listener);
    void removeListener();
    
private:
    juce::Slider frequencySlider;
    juce::Slider amplitudeSlider;
    juce::Slider panSlider;
    juce::TextButton removeButton { "Remove Node" };
    
    EQNodeControllerListener* listener;
    
    int id;
    float frequency, amplitude, pan;
};
