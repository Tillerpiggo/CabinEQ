/*
  ==============================================================================

    PlainTextButtonLookAndFeel.h
    Created: 20 Jan 2025 5:50:11pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

// Custom LookAndFeel for a plain text button
class PlainTextButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                               const juce::Colour& backgroundColour, 
                               bool isMouseOverButton, bool isButtonDown) override
    {
        // Do nothing here to leave the background transparent
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool isMouseOverButton, bool /*isButtonDown*/) override
    {
        auto font = button.getFont();
        g.setFont(font);

        // Change the text color when hovered or not
        if (isMouseOverButton)
            g.setColour(juce::Colours::blue); // Hover color
        else
            g.setColour(juce::Colours::black); // Normal color

        // Optionally underline the text
        auto bounds = button.getLocalBounds();
        g.drawFittedText(button.getButtonText(), bounds, juce::Justification::centred, 1);

        // Draw underline manually
        auto textWidth = font.getStringWidthFloat(button.getButtonText());
        auto textHeight = font.getHeight();
        auto underlineY = bounds.getCentreY() + (textHeight / 2.5f); // Adjust underline position
        g.drawLine(bounds.getCentreX() - (textWidth / 2), underlineY,
                   bounds.getCentreX() + (textWidth / 2), underlineY, 1.0f);
    }
};