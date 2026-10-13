/*
  ==============================================================================

    CurveShapePanel.h

    Two pads that shape the middle one of three selected curve points, sitting
    on the graph while they're selected:

      Frequency   across: where it is between the two outer points
                  up:     how much higher it is in the left ear than the right
      Gain        up:     its gain
                  across: how much louder it is in the left ear (to the left)
                          or the right

    The panel only knows positions from 0 to 1; the graph turns them into
    frequencies and gains, and back.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Theme.h"

/// A square you drag a dot around in. Positions are 0 to 1, from the bottom left.
class XYPad : public juce::Component,
              public juce::SettableTooltipClient
{
public:
    std::function<void()> onGestureStart;
    std::function<void (juce::Point<float>)> onChange;
    std::function<void()> onGestureEnd;
    std::function<void()> onReset; // double-click

    juce::String topLabel, bottomLabel, leftLabel, rightLabel;
    juce::Point<float> neutral { 0.5f, 0.5f }; // where the faint cross is drawn, if it's on the pad
    bool drawsNeutralX = true, drawsNeutralY = true;

    void setPosition (juce::Point<float> newPosition)
    {
        newPosition = { juce::jlimit (0.0f, 1.0f, newPosition.x), juce::jlimit (0.0f, 1.0f, newPosition.y) };
        if (newPosition != position)
        {
            position = newPosition;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat();
        g.setColour (Theme::graph);
        g.fillRoundedRectangle (area, 6.0f);
        g.setColour (Theme::border);
        g.drawRoundedRectangle (area.reduced (0.5f), 6.0f, 1.0f);

        const auto inner = padArea();
        g.setColour (Theme::zeroLine.withAlpha (0.8f));
        if (drawsNeutralX)
            g.drawVerticalLine (juce::roundToInt (inner.getX() + neutral.x * inner.getWidth()), inner.getY(), inner.getBottom());
        if (drawsNeutralY)
            g.drawHorizontalLine (juce::roundToInt (inner.getBottom() - neutral.y * inner.getHeight()), inner.getX(), inner.getRight());

        g.setColour (Theme::textFaint);
        g.setFont (Theme::font (9.5f));
        g.drawText (topLabel, area.reduced (5.0f, 3.0f), juce::Justification::centredTop);
        g.drawText (bottomLabel, area.reduced (5.0f, 3.0f), juce::Justification::centredBottom);
        g.drawText (leftLabel, area.reduced (5.0f, 3.0f), juce::Justification::centredLeft);
        g.drawText (rightLabel, area.reduced (5.0f, 3.0f), juce::Justification::centredRight);

        const juce::Point<float> thumb { inner.getX() + position.x * inner.getWidth(), inner.getBottom() - position.y * inner.getHeight() };
        const bool isHot = isMouseButtonDown() || isMouseOver();
        g.setColour (Theme::accent.withAlpha (isHot ? 0.35f : 0.22f));
        g.fillEllipse (juce::Rectangle<float> (20.0f, 20.0f).withCentre (thumb));
        g.setColour (Theme::accentBright);
        g.fillEllipse (juce::Rectangle<float> (10.0f, 10.0f).withCentre (thumb));
    }

    void mouseDown (const juce::MouseEvent& event) override
    {
        if (onGestureStart)
            onGestureStart();
        mouseDrag (event);
    }

    void mouseDrag (const juce::MouseEvent& event) override
    {
        const auto inner = padArea();
        const juce::Point<float> at { juce::jlimit (0.0f, 1.0f, (event.position.x - inner.getX()) / inner.getWidth()),
                                      juce::jlimit (0.0f, 1.0f, (inner.getBottom() - event.position.y) / inner.getHeight()) };
        if (onChange)
            onChange (at);
    }

    void mouseDoubleClick (const juce::MouseEvent&) override
    {
        if (onReset)
            onReset();
    }

    void mouseEnter (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override  { repaint(); }
    void mouseUp (const juce::MouseEvent&) override
    {
        if (onGestureEnd)
            onGestureEnd();
        repaint();
    }

private:
    juce::Rectangle<float> padArea() const { return getLocalBounds().toFloat().reduced (12.0f); }
    juce::Point<float> position { 0.5f, 0.5f };
};

//==============================================================================
class CurveShapePanel : public juce::Component
{
public:
    static constexpr int width = 268, height = 150;

    XYPad frequencyPad, gainPad;
    juce::String frequencyText, gainText; // what the pads are set to, under each

    CurveShapePanel()
    {
        frequencyPad.topLabel = "L higher";
        frequencyPad.bottomLabel = "R higher";
        frequencyPad.drawsNeutralX = false;
        frequencyPad.setTooltip ("The middle point's frequency: across moves it between the two outer points; up or down makes it "
                                 "higher in one ear than the other. Double-click to make the ears the same.");
        gainPad.leftLabel = "L";
        gainPad.rightLabel = "R";
        gainPad.setTooltip ("The middle point's gain: up or down sets it; across makes it louder in the left ear or the right. "
                            "Double-click to make the ears the same.");
        addAndMakeVisible (frequencyPad);
        addAndMakeVisible (gainPad);
    }

    void setTexts (const juce::String& newFrequencyText, const juce::String& newGainText)
    {
        if (newFrequencyText != frequencyText || newGainText != gainText)
        {
            frequencyText = newFrequencyText;
            gainText = newGainText;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat();
        g.setColour (Theme::panel.withAlpha (0.96f));
        g.fillRoundedRectangle (area, 8.0f);
        g.setColour (Theme::border);
        g.drawRoundedRectangle (area.reduced (0.5f), 8.0f, 1.0f);

        g.setFont (Theme::font (10.5f, true));
        g.setColour (Theme::textDim);
        g.drawText ("FREQUENCY", frequencyPad.getBounds().withY (6).withHeight (14), juce::Justification::centredLeft);
        g.drawText ("GAIN", gainPad.getBounds().withY (6).withHeight (14), juce::Justification::centredLeft);

        g.setFont (Theme::font (10.5f));
        g.setColour (Theme::text);
        g.drawText (frequencyText, frequencyPad.getBounds().withY (frequencyPad.getBottom() + 3).withHeight (14), juce::Justification::centredLeft);
        g.drawText (gainText, gainPad.getBounds().withY (gainPad.getBottom() + 3).withHeight (14), juce::Justification::centredLeft);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10, 0).withTrimmedTop (23).withTrimmedBottom (21);
        frequencyPad.setBounds (area.removeFromLeft ((area.getWidth() - 8) / 2));
        area.removeFromLeft (8);
        gainPad.setBounds (area);
    }

    // Clicks on the panel's own background shouldn't fall through to the graph
    void mouseDown (const juce::MouseEvent&) override {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CurveShapePanel)
};
