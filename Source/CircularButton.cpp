/*
  ==============================================================================

    CircularButton.cpp
    Created: 7 Jul 2024 12:14:21am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CircularButton.h"

CircularButton::CircularButton(const juce::String& buttonName)
    : button(buttonName)
{
    // Initial state
    isDeepRed = false;

    // Configure the button and label
    button.setClickingTogglesState(false);
    button.addListener(this); // Add this class as a listener to the button
    addAndMakeVisible(button);

    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

    // Set default size
    setSize(100, 120);
}

void CircularButton::setLabelText(const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
}

void CircularButton::setDeepRed (bool deepRed)
{
    isDeepRed = deepRed;
    repaint();
}

void CircularButton::paint(juce::Graphics& g)
{
    auto circleBounds = getLocalBounds().withTrimmedBottom(20).reduced(10).toFloat();

    g.setColour(isDeepRed ? juce::Colours::darkred : juce::Colours::red.withAlpha(0.2f));
    g.fillEllipse(circleBounds);

    button.setBounds(circleBounds.toNearestInt());
}

void CircularButton::resized()
{
    auto area = getLocalBounds();
    auto circleArea = area.withTrimmedBottom(20).reduced(10);
    button.setBounds(circleArea);
    label.setBounds(area.removeFromBottom(20));
}

void CircularButton::buttonClicked(juce::Button* button)
{
    if (button == &this->button)
    {
        if (listener != nullptr)
            listener->buttonClicked(button);
    }
}

void CircularButton::buttonStateChanged(juce::Button* button)
{
    if (button == &this->button)
    {
        if (listener != nullptr)
            listener->buttonStateChanged(button);
    }
}

void CircularButton::addListener(juce::Button::Listener* newListener)
{
    listener = newListener;
}

void CircularButton::removeListener()
{
    listener = nullptr;
}

juce::Button* CircularButton::getButtonPointer()
{
    return &button;
}
