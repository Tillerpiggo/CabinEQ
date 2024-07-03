///*
//  ==============================================================================
//
//    ClearSoundLookAndFeel.h
//    Created: 3 Jul 2024 1:17:38pm
//    Author:  Tyler Gee
//
//  ==============================================================================
//*/
//
//#pragma once
//
//#include <JuceHeader.h>
//#include "PluginProcessor.h"
//
//class ClearSoundLookAndFeel : public juce::LookAndFeel_V4
//{
//public:
//    ClearSoundLookAndFeel()
//    {
//        setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
//        setColour(juce::TextButton::buttonColourId, juce::Colours::white);
//        setColour(juce::TextButton::buttonOnColourId, juce::Colours::white);
//        setColour(juce::TextButton::textColourOnId, juce::Colours::black);
//        setColour(juce::TextButton::textColourOffId, juce::Colours::black);
//        setColour(juce::ToggleButton::textColourId, juce::Colours::black);
//        setColour(juce::ToggleButton::tickColourId, juce::Colours::darkgrey);
//        setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
//    }
//
//    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
//                              bool isMouseOverButton, bool isButtonDown) override
//    {
//        auto buttonArea = button.getLocalBounds();
//        auto edge = 4;
//        auto shadowOffset = 2;
//
//        // Draw shadow
//        g.setColour(juce::Colours::lightgrey);
//        g.fillRect(buttonArea.translated(shadowOffset, shadowOffset));
//
//        // Draw button
//        g.setColour(backgroundColour);
//        g.fillRect(buttonArea);
//    }
//
//    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonDown) override
//    {
//        const juce::Font font("Arial", 15.0f, juce::Font::bold);
//        g.setFont(font);
//        g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
//                                                              : juce::TextButton::textColourOffId));
//
//        auto textBounds = button.getLocalBounds().reduced(4);
//        g.drawFittedText(button.getButtonText(), textBounds, juce::Justification::centred, 1);
//    }
//
//    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
//                          bool isMouseOverButton, bool isButtonDown) override
//    {
//        auto fontSize = juce::jmin(15.0f, button.getHeight() * 0.75f);
//        auto tickWidth = fontSize * 1.1f;
//
//        drawTickBox(g, button, 4.0f, (button.getHeight() - tickWidth) / 2.0f,
//                    tickWidth, tickWidth,
//                    button.getToggleState(),
//                    button.isEnabled(),
//                    isMouseOverButton,
//                    isButtonDown);
//
//        g.setColour(button.findColour(juce::ToggleButton::textColourId));
//        g.setFont(fontSize);
//
//        if (!button.isEnabled())
//            g.setOpacity(0.5f);
//
//        auto textX = tickWidth + 5.0f;
//
//        g.drawFittedText(button.getButtonText(),
//                         textX, 0, button.getWidth() - textX - 2, button.getHeight(),
//                         juce::Justification::centredLeft, 10);
//    }
//};
