/*
  ==============================================================================

    CircularButton.h
    Created: 7 Jul 2024 12:14:21am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//class CircularButton : public juce::Component, public juce::Button::Listener
//{
//public:
//    CircularButton (const juce::String& buttonName = "CircularButton");
//
//    void setLabelText (const juce::String& text);
//    void setDeepRed (bool deepRed);
//
//    void paint (juce::Graphics& g) override;
//    void resized() override;
//
//    // Button::Listener overrides
//    void buttonClicked (juce::Button* button) override;
//    void buttonStateChanged (juce::Button* button) override;
//
//    void addListener (juce::Button::Listener* newListener);
//    void removeListener();
//    
//    juce::Button* getButtonPointer(); // TODO: This seems like very bad and problematic code TBH
//
//private:
//    juce::TextButton button;
//    juce::Label label;
//    bool isDeepRed;
//    juce::Button::Listener* listener = nullptr;
//
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircularButton)
//};

#pragma once

#include <JuceHeader.h>

class CircularButton : public juce::Component, public juce::Button::Listener
{
public:
    CircularButton(const juce::String& buttonName = "CircularButton");

    void setLabelText(const juce::String& text);
    void setDeepRed(bool deepRed);

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Button::Listener overrides
    void buttonClicked(juce::Button* button) override;
    void buttonStateChanged(juce::Button* button) override;

    void addListener(juce::Button::Listener* newListener);
    void removeListener();
    
    juce::Button* getButtonPointer(); // TODO: This seems like very bad and problematic code TBH

private:
    class InvisibleButton : public juce::Button
    {
    public:
        InvisibleButton (const juce::String& name) : juce::Button (name) {}
        void paintButton (juce::Graphics&, bool, bool) override {}
    };

    InvisibleButton button;
    juce::Label label;
    bool isDeepRed;
    juce::Button::Listener* listener = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircularButton)
};
