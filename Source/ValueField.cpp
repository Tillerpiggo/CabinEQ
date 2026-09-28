/*
  ==============================================================================

    ValueField.cpp

  ==============================================================================
*/

#include "ValueField.h"

namespace
{
    constexpr float pixelsForFullRange = 400.0f;
}

ValueField::ValueField (const juce::String& caption, double minimum, double maximum, double defaultValue, Scale scale)
    : caption (caption), minimum (minimum), maximum (maximum), defaultValue (defaultValue), value (defaultValue), scale (scale)
{
    parse = [] (const juce::String& text) -> std::optional<double>
    {
        auto trimmed = text.trim();
        if (! trimmed.containsAnyOf ("0123456789"))
            return std::nullopt;
        return trimmed.getDoubleValue();
    };

    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    setWantsKeyboardFocus (false);
}

void ValueField::setValue (double newValue)
{
    newValue = juce::jlimit (minimum, maximum, newValue);
    if (newValue != value)
    {
        value = newValue;
        repaint();
    }
}

void ValueField::setRange (double newMinimum, double newMaximum)
{
    minimum = newMinimum;
    maximum = newMaximum;
    setValue (value);
}

double ValueField::toProportion (double v) const
{
    if (scale == Scale::logarithmic)
        return std::log (v / minimum) / std::log (maximum / minimum);
    return (v - minimum) / (maximum - minimum);
}

double ValueField::fromProportion (double proportion) const
{
    proportion = juce::jlimit (0.0, 1.0, proportion);
    if (scale == Scale::logarithmic)
        return minimum * std::pow (maximum / minimum, proportion);
    return minimum + proportion * (maximum - minimum);
}

void ValueField::change (double newValue)
{
    newValue = juce::jlimit (minimum, maximum, newValue);
    if (newValue == value)
        return;

    value = newValue;
    repaint();
    if (onValueChange)
        onValueChange (value);
}

//==============================================================================
void ValueField::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const bool hot = isMouseOver() || isDragging;

    g.setColour (hot && isEnabled() ? Theme::raisedHover : Theme::raised);
    g.fillRoundedRectangle (bounds, Theme::cornerRadius);

    if (isDragging)
    {
        g.setColour (accentColour.withAlpha (0.6f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), Theme::cornerRadius, 1.0f);
    }

    auto content = bounds.reduced (getWidth() < 80 ? 7.0f : 10.0f, 5.0f);
    if (caption.isNotEmpty())
    {
        g.setColour (Theme::textFaint);
        g.setFont (Theme::font (10.5f));
        g.drawText (caption.toUpperCase(), content.removeFromTop (13.0f), juce::Justification::centredLeft, true);
    }

    if (editor == nullptr)
    {
        g.setColour (isEnabled() ? Theme::text : Theme::textFaint);
        g.setFont (Theme::font (caption.isNotEmpty() ? 14.0f : 13.0f));
        g.drawFittedText (format (value), content.toNearestInt(), juce::Justification::centredLeft, 1, 0.8f);
    }
}

void ValueField::resized()
{
    if (editor != nullptr)
        editor->setBounds (getLocalBounds().reduced (4, caption.isNotEmpty() ? 0 : 3).withTrimmedTop (caption.isNotEmpty() ? 17 : 0).withTrimmedBottom (caption.isNotEmpty() ? 3 : 0));
}

void ValueField::mouseDown (const juce::MouseEvent& event)
{
    if (! isEnabled() || editor != nullptr)
        return;

    if (event.mods.isAltDown())
    {
        if (onGestureStart) onGestureStart();
        change (defaultValue);
        if (onGestureEnd) onGestureEnd();
        return;
    }

    isDragging = true;
    dragProportion = toProportion (value);
    lastDragPosition = event.position;
    if (onGestureStart)
        onGestureStart();
    repaint();
}

void ValueField::mouseDrag (const juce::MouseEvent& event)
{
    if (! isDragging)
        return;

    // Drag up (or right) to increase. Shift is ten times finer, and can be pressed mid-drag.
    const auto delta = event.position - lastDragPosition;
    lastDragPosition = event.position;
    const double sensitivity = event.mods.isShiftDown() ? 0.1 : 1.0;
    dragProportion = juce::jlimit (0.0, 1.0, dragProportion + (-delta.y + delta.x * 0.25f) / pixelsForFullRange * sensitivity);
    change (fromProportion (dragProportion));
}

void ValueField::mouseUp (const juce::MouseEvent&)
{
    if (! isDragging)
        return;

    isDragging = false;
    if (onGestureEnd)
        onGestureEnd();
    repaint();
}

void ValueField::mouseDoubleClick (const juce::MouseEvent& event)
{
    if (isEnabled() && ! event.mods.isAltDown())
        showEditor();
}

void ValueField::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (! isEnabled() || editor != nullptr)
        return;

    const float delta = (std::abs (wheel.deltaY) > std::abs (wheel.deltaX) ? wheel.deltaY : -wheel.deltaX) * (wheel.isReversed ? -1.0f : 1.0f);
    if (delta == 0.0f)
        return;

    const double step = (event.mods.isShiftDown() ? 0.02 : 0.2) * delta;
    if (onGestureStart) onGestureStart();
    change (fromProportion (toProportion (value) + step));
    if (onGestureEnd) onGestureEnd();
}

//==============================================================================
void ValueField::showEditor()
{
    editor = std::make_unique<juce::TextEditor>();
    editor->setFont (Theme::font (14.0f));
    editor->setJustification (juce::Justification::centredLeft);
    editor->setIndents (6, 0);
    editor->setText (format (value).upToFirstOccurrenceOf (" ", false, false), false);
    editor->setSelectAllWhenFocused (true);
    editor->addListener (this);
    addAndMakeVisible (*editor);
    resized();
    editor->grabKeyboardFocus();
    editor->selectAll();
    repaint();
}

void ValueField::hideEditor (bool apply)
{
    if (editor == nullptr)
        return;

    auto text = editor->getText();
    editor->removeListener (this);

    // Delete it after this callback returns, since the editor is the one calling us
    editor->setVisible (false);
    juce::MessageManager::callAsync ([safeThis = juce::Component::SafePointer<ValueField> (this), old = editor.release()]
    {
        delete old;
        if (safeThis != nullptr)
            safeThis->repaint();
    });

    if (apply)
    {
        if (auto parsed = parse (text))
        {
            if (onGestureStart) onGestureStart();
            change (*parsed);
            if (onGestureEnd) onGestureEnd();
        }
    }
    repaint();
}
