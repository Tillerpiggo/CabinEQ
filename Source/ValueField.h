/*
  ==============================================================================

    ValueField.h

    A small labelled number. Drag it up or down, scroll over it, or double-click
    to type a value. Shift makes dragging and scrolling finer, and Alt-click
    (Option-click) resets it.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Theme.h"

class ValueField : public juce::Component,
                   public juce::SettableTooltipClient,
                   private juce::TextEditor::Listener
{
public:
    enum class Scale { linear, logarithmic };

    ValueField (const juce::String& caption, double minimum, double maximum, double defaultValue, Scale scale = Scale::linear);

    void setValue (double newValue); // doesn't call onValueChange
    void commitEditing() { hideEditor (true); } // applies whatever's been typed, if anything
    double getValue() const { return value; }
    void setRange (double newMinimum, double newMaximum);

    std::function<juce::String (double)> format = [] (double v) { return juce::String (v, 2); };
    std::function<std::optional<double> (const juce::String&)> parse; // defaults to reading a number

    std::function<void()> onGestureStart, onGestureEnd;
    std::function<void (double)> onValueChange;

    void setAccentColour (juce::Colour colour) { accentColour = colour; repaint(); }

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void mouseEnter (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }
    void enablementChanged() override { repaint(); }

private:
    double toProportion (double v) const;
    double fromProportion (double proportion) const;
    void change (double newValue);
    void showEditor();
    void hideEditor (bool apply);

    void textEditorReturnKeyPressed (juce::TextEditor&) override { hideEditor (true); }
    void textEditorEscapeKeyPressed (juce::TextEditor&) override { hideEditor (false); }
    void textEditorFocusLost (juce::TextEditor&) override { hideEditor (true); }

    juce::String caption;
    double minimum, maximum, defaultValue, value;
    Scale scale;

    double dragProportion = 0.0;
    juce::Point<float> lastDragPosition;
    bool isDragging = false;
    juce::Colour accentColour = Theme::accent;
    std::unique_ptr<juce::TextEditor> editor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueField)
};
