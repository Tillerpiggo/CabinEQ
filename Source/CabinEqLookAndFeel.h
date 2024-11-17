/*
  ==============================================================================

    CabinEqLookAndFeel.h
    Created: 17 Nov 2024 12:33:48am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "UIConstants.h"

class CabinEqLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CabinEqLookAndFeel()
    {
        setColour (juce::Slider::thumbColourId, SLIDER_THUMB_COLOR);
        setColour (juce::Slider::backgroundColourId, SLIDER_BACKGROUND_COLOR);
        setColour (juce::Slider::trackColourId, SLIDER_TRACK_COLOR);
        setColour (juce::TextButton::buttonColourId, BUTTON_OFF_COLOR);
        setColour (juce::TextButton::buttonOnColourId, BUTTON_ON_COLOR);
        setColour (juce::TextButton::textColourOffId, BUTTON_OFF_TEXT_COLOR);
        setColour (juce::TextButton::textColourOnId, BUTTON_ON_TEXT_COLOR);
        setColour (juce::ComboBox::outlineColourId, BUTTON_OUTLINE_COLOR);
        setColour (juce::ComboBox::backgroundColourId, BUTTON_OFF_COLOR);
    }
    
private:
};
