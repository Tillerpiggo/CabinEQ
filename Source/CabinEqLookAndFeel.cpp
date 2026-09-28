/*
  ==============================================================================

    CabinEqLookAndFeel.cpp

  ==============================================================================
*/

#include "CabinEqLookAndFeel.h"

CabinEqLookAndFeel::CabinEqLookAndFeel()
{
    using namespace juce;

    setColour (ResizableWindow::backgroundColourId, Theme::background);
    setColour (DocumentWindow::textColourId, Theme::text);

    setColour (TextButton::buttonColourId, Theme::raised);
    setColour (TextButton::buttonOnColourId, Theme::accent.withAlpha (0.22f));
    setColour (TextButton::textColourOffId, Theme::text);
    setColour (TextButton::textColourOnId, Theme::accentBright);
    setColour (ToggleButton::textColourId, Theme::text);
    setColour (ToggleButton::tickColourId, Theme::accent);

    setColour (Label::textColourId, Theme::text);
    setColour (Label::textWhenEditingColourId, Theme::text);
    setColour (Label::backgroundWhenEditingColourId, Theme::raised);
    setColour (Label::outlineWhenEditingColourId, Theme::accent);

    setColour (TextEditor::backgroundColourId, Theme::raised);
    setColour (TextEditor::textColourId, Theme::text);
    setColour (TextEditor::highlightColourId, Theme::accent.withAlpha (0.35f));
    setColour (TextEditor::highlightedTextColourId, Theme::text);
    setColour (TextEditor::outlineColourId, Theme::border);
    setColour (TextEditor::focusedOutlineColourId, Theme::accent);
    setColour (CaretComponent::caretColourId, Theme::accentBright);

    setColour (ComboBox::backgroundColourId, Theme::raised);
    setColour (ComboBox::textColourId, Theme::text);
    setColour (ComboBox::outlineColourId, Theme::border);
    setColour (ComboBox::arrowColourId, Theme::textDim);
    setColour (ComboBox::focusedOutlineColourId, Theme::accent);

    setColour (PopupMenu::backgroundColourId, Theme::panel);
    setColour (PopupMenu::textColourId, Theme::text);
    setColour (PopupMenu::highlightedBackgroundColourId, Theme::raisedHover);
    setColour (PopupMenu::highlightedTextColourId, Theme::text);
    setColour (PopupMenu::headerTextColourId, Theme::textDim);

    setColour (Slider::backgroundColourId, Theme::raised);
    setColour (Slider::trackColourId, Theme::accent);
    setColour (Slider::thumbColourId, Theme::text);
    setColour (Slider::textBoxTextColourId, Theme::text);
    setColour (Slider::textBoxBackgroundColourId, Colours::transparentBlack);
    setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
    setColour (Slider::textBoxHighlightColourId, Theme::accent.withAlpha (0.35f));

    setColour (AlertWindow::backgroundColourId, Theme::panel);
    setColour (AlertWindow::textColourId, Theme::text);
    setColour (AlertWindow::outlineColourId, Theme::border);

    setColour (TooltipWindow::backgroundColourId, Theme::raised);
    setColour (TooltipWindow::textColourId, Theme::text);
    setColour (TooltipWindow::outlineColourId, Theme::border);

    setColour (ScrollBar::thumbColourId, Theme::raisedHover);
    setColour (ListBox::backgroundColourId, Colours::transparentBlack);

    setColour (ResizableWindow::backgroundColourId, Theme::background);
    getCurrentColourScheme().setUIColour (ColourScheme::UIColour::windowBackground, Theme::background);
    getCurrentColourScheme().setUIColour (ColourScheme::UIColour::widgetBackground, Theme::raised);
    getCurrentColourScheme().setUIColour (ColourScheme::UIColour::menuBackground, Theme::panel);
    getCurrentColourScheme().setUIColour (ColourScheme::UIColour::outline, Theme::border);
    getCurrentColourScheme().setUIColour (ColourScheme::UIColour::defaultText, Theme::text);
    getCurrentColourScheme().setUIColour (ColourScheme::UIColour::highlightedFill, Theme::accent);
}

juce::Typeface::Ptr CabinEqLookAndFeel::getTypefaceForFont (const juce::Font& font)
{
    if (font.getTypefaceName() == juce::Font::getDefaultSansSerifFontName())
        return Theme::typeface();
    return LookAndFeel_V4::getTypefaceForFont (font);
}

//==============================================================================
void CabinEqLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                               bool isHighlighted, bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto colour = backgroundColour;

    if (! button.isEnabled())
        colour = colour.withMultipliedAlpha (0.5f);
    else if (isDown)
        colour = colour.brighter (0.15f);
    else if (isHighlighted)
        colour = colour.brighter (0.08f);

    g.setColour (colour);
    g.fillRoundedRectangle (bounds, Theme::cornerRadius);

    if (button.getToggleState())
    {
        g.setColour (Theme::accent.withAlpha (0.55f));
        g.drawRoundedRectangle (bounds, Theme::cornerRadius, 1.0f);
    }
}

juce::Font CabinEqLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return Theme::font (std::min (13.0f, (float) buttonHeight * 0.5f));
}

void CabinEqLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool isHighlighted, bool)
{
    // A switch on the left, text to its right
    auto bounds = button.getLocalBounds().toFloat();
    auto track = bounds.removeFromLeft (30.0f).withSizeKeepingCentre (28.0f, 16.0f);
    const bool on = button.getToggleState();

    g.setColour (on ? Theme::accent : (isHighlighted ? Theme::raisedHover : Theme::raised));
    g.fillRoundedRectangle (track, 8.0f);

    auto knob = juce::Rectangle<float> (12.0f, 12.0f).withCentre ({ on ? track.getRight() - 8.0f : track.getX() + 8.0f, track.getCentreY() });
    g.setColour (on ? juce::Colours::white : Theme::textDim);
    g.fillEllipse (knob);

    g.setColour (button.isEnabled() ? Theme::text : Theme::textFaint);
    g.setFont (Theme::font (13.0f));
    g.drawText (button.getButtonText(), bounds.withTrimmedLeft (6.0f), juce::Justification::centredLeft, true);
}

//==============================================================================
void CabinEqLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    auto bounds = juce::Rectangle<float> (0, 0, (float) width, (float) height);
    g.setColour (Theme::panel);
    g.fillRect (bounds);
    g.setColour (Theme::border);
    g.drawRect (bounds, 1.0f);
}

void CabinEqLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                                            bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
                                            const juce::String& shortcutKeyText, const juce::Drawable*, const juce::Colour* textColour)
{
    if (isSeparator)
    {
        g.setColour (Theme::border);
        g.fillRect (area.reduced (8, 0).withHeight (1).withY (area.getCentreY()));
        return;
    }

    auto bounds = area.toFloat().reduced (2.0f, 1.0f);
    if (isHighlighted && isActive)
    {
        g.setColour (Theme::raisedHover);
        g.fillRoundedRectangle (bounds, 4.0f);
    }

    auto content = bounds.reduced (10.0f, 0.0f);
    auto tickArea = content.removeFromLeft (16.0f);
    if (isTicked)
    {
        juce::Path tick;
        tick.startNewSubPath (tickArea.getX() + 2.0f, tickArea.getCentreY());
        tick.lineTo (tickArea.getX() + 6.0f, tickArea.getCentreY() + 4.0f);
        tick.lineTo (tickArea.getRight() - 2.0f, tickArea.getCentreY() - 4.0f);
        g.setColour (Theme::accentBright);
        g.strokePath (tick, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    auto colour = textColour != nullptr ? *textColour : Theme::text;
    g.setColour (isActive ? colour : Theme::textFaint);
    g.setFont (getPopupMenuFont());
    g.drawFittedText (text, content.toNearestInt(), juce::Justification::centredLeft, 1);

    if (shortcutKeyText.isNotEmpty())
    {
        g.setColour (Theme::textFaint);
        g.drawText (shortcutKeyText, content.withTrimmedRight (hasSubMenu ? 12.0f : 0.0f), juce::Justification::centredRight);
    }

    if (hasSubMenu)
    {
        juce::Path arrow;
        const float x = content.getRight() - 4.0f, y = content.getCentreY();
        arrow.startNewSubPath (x - 3.0f, y - 4.0f);
        arrow.lineTo (x + 1.0f, y);
        arrow.lineTo (x - 3.0f, y + 4.0f);
        g.setColour (Theme::textDim);
        g.strokePath (arrow, juce::PathStrokeType (1.4f));
    }
}

juce::Font CabinEqLookAndFeel::getPopupMenuFont()
{
    return Theme::font (13.0f);
}

void CabinEqLookAndFeel::getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int, int& idealWidth, int& idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = 9;
        return;
    }
    idealHeight = 28;
    idealWidth = (int) Theme::textWidth (getPopupMenuFont(), text) + 64;
}

void CabinEqLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0, 0, (float) width, (float) height).reduced (0.5f);
    g.setColour (box.isMouseOver (true) ? Theme::raisedHover : Theme::raised);
    g.fillRoundedRectangle (bounds, Theme::cornerRadius);

    juce::Path chevron;
    const float cx = bounds.getRight() - 12.0f, cy = bounds.getCentreY();
    chevron.startNewSubPath (cx - 4.0f, cy - 2.0f);
    chevron.lineTo (cx, cy + 2.0f);
    chevron.lineTo (cx + 4.0f, cy - 2.0f);
    g.setColour (Theme::textDim);
    g.strokePath (chevron, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font CabinEqLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return Theme::font (13.0f);
}

void CabinEqLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (4, 1, box.getWidth() - 24, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

//==============================================================================
void CabinEqLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos,
                                           float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::TwoValueHorizontal)
    {
        auto track = juce::Rectangle<float> ((float) x, (float) y + height * 0.5f - 2.0f, (float) width, 4.0f);
        g.setColour (Theme::raised);
        g.fillRoundedRectangle (track, 2.0f);
        g.setColour (slider.isEnabled() ? Theme::accent : Theme::textFaint);
        g.fillRoundedRectangle (track.withLeft (minSliderPos).withRight (std::max (minSliderPos + 4.0f, maxSliderPos)), 2.0f);
        g.setColour (slider.isEnabled() ? Theme::text : Theme::textDim);
        for (float position : { minSliderPos, maxSliderPos })
            g.fillEllipse (juce::Rectangle<float> (14.0f, 14.0f).withCentre ({ position, track.getCentreY() }));
        return;
    }

    if (style == juce::Slider::LinearVertical)
    {
        auto track = juce::Rectangle<float> ((float) x + width * 0.5f - 2.0f, (float) y, 4.0f, (float) height);
        g.setColour (Theme::raised);
        g.fillRoundedRectangle (track, 2.0f);
        g.setColour (slider.isEnabled() ? Theme::accent : Theme::textFaint);
        g.fillRoundedRectangle (track.withTop (sliderPos), 2.0f);
        g.setColour (slider.isEnabled() ? Theme::text : Theme::textDim);
        g.fillEllipse (juce::Rectangle<float> (14.0f, 14.0f).withCentre ({ track.getCentreX(), sliderPos }));
        return;
    }

    if (style != juce::Slider::LinearHorizontal)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, 0, 0, style, slider);
        return;
    }

    auto track = juce::Rectangle<float> ((float) x, (float) y + height * 0.5f - 2.0f, (float) width, 4.0f);
    g.setColour (Theme::raised);
    g.fillRoundedRectangle (track, 2.0f);

    g.setColour (slider.isEnabled() ? Theme::accent : Theme::textFaint);
    g.fillRoundedRectangle (track.withRight (sliderPos), 2.0f);

    g.setColour (slider.isEnabled() ? Theme::text : Theme::textDim);
    g.fillEllipse (juce::Rectangle<float> (14.0f, 14.0f).withCentre ({ sliderPos, track.getCentreY() }));
}

juce::Label* CabinEqLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* label = LookAndFeel_V4::createSliderTextBox (slider);
    label->setFont (Theme::font (12.0f));
    label->setColour (juce::Label::textColourId, Theme::textDim);
    label->setJustificationType (juce::Justification::centredRight);
    return label;
}

//==============================================================================
void CabinEqLookAndFeel::drawCallOutBoxBackground (juce::CallOutBox&, juce::Graphics& g, const juce::Path& path, juce::Image&)
{
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillPath (path, juce::AffineTransform::translation (0.0f, 3.0f));
    g.setColour (Theme::panel);
    g.fillPath (path);
    g.setColour (Theme::border);
    g.strokePath (path, juce::PathStrokeType (1.0f));
}

void CabinEqLookAndFeel::drawAlertBox (juce::Graphics& g, juce::AlertWindow& alert, const juce::Rectangle<int>& textArea, juce::TextLayout& textLayout)
{
    auto bounds = alert.getLocalBounds().toFloat();
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (bounds, 10.0f);
    g.setColour (Theme::border);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 10.0f, 1.0f);

    textLayout.draw (g, textArea.toFloat().withTrimmedLeft (4.0f));
}

juce::Font CabinEqLookAndFeel::getAlertWindowTitleFont()   { return Theme::font (16.0f, true); }
juce::Font CabinEqLookAndFeel::getAlertWindowMessageFont() { return Theme::font (13.0f); }
juce::Font CabinEqLookAndFeel::getAlertWindowFont()        { return Theme::font (13.0f); }

void CabinEqLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    auto bounds = juce::Rectangle<float> (0, 0, (float) width, (float) height);
    g.setColour (Theme::raised);
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (Theme::border);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);
    g.setColour (Theme::text);
    g.setFont (Theme::font (12.0f));
    g.drawFittedText (text, bounds.reduced (8.0f, 4.0f).toNearestInt(), juce::Justification::centredLeft, 3);
}

juce::Rectangle<int> CabinEqLookAndFeel::getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea)
{
    const int width = (int) std::min (320.0f, Theme::textWidth (Theme::font (12.0f), tipText) + 18.0f);
    const int height = 24;
    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (width + 12) : screenPos.x + 16,
                                 screenPos.y > parentArea.getCentreY() ? screenPos.y - (height + 6) : screenPos.y + 16,
                                 width, height).constrainedWithin (parentArea);
}

void CabinEqLookAndFeel::fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& editor)
{
    g.setColour (editor.findColour (juce::TextEditor::backgroundColourId));
    g.fillRoundedRectangle (juce::Rectangle<float> (0, 0, (float) width, (float) height), 4.0f);
}

void CabinEqLookAndFeel::drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& editor)
{
    if (! editor.isEnabled())
        return;

    const bool focused = editor.hasKeyboardFocus (true) && ! editor.isReadOnly();
    g.setColour (editor.findColour (focused ? juce::TextEditor::focusedOutlineColourId : juce::TextEditor::outlineColourId));
    g.drawRoundedRectangle (juce::Rectangle<float> (0, 0, (float) width, (float) height).reduced (0.5f), 4.0f, focused ? 1.5f : 1.0f);
}

void CabinEqLookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar&, int x, int y, int width, int height, bool isScrollbarVertical,
                                        int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown)
{
    juce::Rectangle<float> thumb = isScrollbarVertical
        ? juce::Rectangle<float> ((float) x + 2.0f, (float) thumbStartPosition, (float) width - 4.0f, (float) thumbSize)
        : juce::Rectangle<float> ((float) thumbStartPosition, (float) y + 2.0f, (float) thumbSize, (float) height - 4.0f);

    g.setColour (Theme::raisedHover.withAlpha (isMouseDown ? 1.0f : isMouseOver ? 0.9f : 0.6f));
    g.fillRoundedRectangle (thumb, std::min (thumb.getWidth(), thumb.getHeight()) * 0.5f);
}
