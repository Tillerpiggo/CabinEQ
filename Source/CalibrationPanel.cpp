/*
  ==============================================================================

    CalibrationPanel.cpp

  ==============================================================================
*/

#include "CalibrationPanel.h"
#include "Theme.h"

namespace
{
    const juce::Identifier idRows { "calibrationRows" };
    const juce::Identifier idColumns { "calibrationColumns" };
    const juce::Identifier idLevel { "calibrationLevel" };
}

CalibrationPanel::CalibrationPanel (CabinEqAudioProcessor& p)
    : processor (p), player (p.getCalibration())
{
    auto& state = processor.parameters.state;

    auto setUp = [this] (juce::Slider& slider, juce::Label& label, const juce::String& text, double min, double max, double step, double value)
    {
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 52, 20);
        slider.setRange (min, max, step);
        slider.setValue (value, juce::dontSendNotification);
        slider.onValueChange = [this] { applySettings(); };
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setFont (Theme::font (12.0f));
        label.setColour (juce::Label::textColourId, Theme::textDim);
        addAndMakeVisible (label);
    };

    setUp (rowsSlider, rowsLabel, "Rows", 1, CalibrationPlayer::maxRows, 1, (int) state.getProperty (idRows, CalibrationPlayer::defaultRows));
    setUp (columnsSlider, columnsLabel, "Columns", 1, CalibrationPlayer::maxColumns, 1, (int) state.getProperty (idColumns, CalibrationPlayer::defaultColumns));
    setUp (volumeSlider, volumeLabel, "Volume", -60, 0, 0.5, (double) state.getProperty (idLevel, -20.0));
    volumeSlider.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " dB"; };
    volumeSlider.updateText();
    volumeSlider.setTooltip ("How loud the calibration sounds are, separately from everything else");

    playButton.setClickingTogglesState (true);
    playButton.onClick = [this]
    {
        player.setPlaying (playButton.getToggleState());
        updateButtons();
    };
    allButton.onClick = [this]
    {
        player.setSelection ({});
        updateButtons();
    };
    allButton.setTooltip ("Go back to playing every position in turn");
    closeButton.onClick = [this] { if (onCloseClicked) onCloseClicked(); };

    for (auto* button : { &playButton, &allButton, &closeButton })
        addAndMakeVisible (button);

    applySettings();
    updateButtons();
    startTimerHz (30);
}

CalibrationPanel::~CalibrationPanel()
{
    stop();
}

void CalibrationPanel::stop()
{
    player.setPlaying (false);
    playButton.setToggleState (false, juce::dontSendNotification);
    updateButtons();
}

void CalibrationPanel::applySettings()
{
    player.setGrid (rows(), columns());
    player.setLevelDb ((float) volumeSlider.getValue());

    // Forget selected positions that aren't on the grid any more
    auto selection = player.getSelection();
    for (auto it = selection.begin(); it != selection.end();)
        it = *it >= rows() * columns() ? selection.erase (it) : std::next (it);
    player.setSelection (selection);

    // Not undoable: it's how you like to calibrate, not part of the EQ
    auto& state = processor.parameters.state;
    state.setProperty (idRows, rows(), nullptr);
    state.setProperty (idColumns, columns(), nullptr);
    state.setProperty (idLevel, volumeSlider.getValue(), nullptr);

    updateButtons();
    repaint();
}

void CalibrationPanel::updateButtons()
{
    playButton.setButtonText (player.isPlaying() ? "Stop" : "Play");
    allButton.setVisible (! player.getSelection().empty());
    repaint();
}

void CalibrationPanel::timerCallback()
{
    const int current = player.getCurrentPosition();
    if (current != shownPosition)
    {
        shownPosition = current;
        repaint (gridArea.expanded (8));
    }
}

//==============================================================================
juce::Point<float> CalibrationPanel::centreOf (int position) const
{
    const int row = position / columns(), column = position % columns();
    const float cellWidth = (float) gridArea.getWidth() / (float) columns();
    const float cellHeight = (float) gridArea.getHeight() / (float) rows();
    return { (float) gridArea.getX() + cellWidth * ((float) column + 0.5f),
             (float) gridArea.getY() + cellHeight * ((float) row + 0.5f) };
}

int CalibrationPanel::positionAt (juce::Point<float> point) const
{
    if (! gridArea.toFloat().contains (point))
        return -1;
    const int column = juce::jlimit (0, columns() - 1, (int) ((point.x - (float) gridArea.getX()) / (float) gridArea.getWidth() * (float) columns()));
    const int row = juce::jlimit (0, rows() - 1, (int) ((point.y - (float) gridArea.getY()) / (float) gridArea.getHeight() * (float) rows()));
    return row * columns() + column;
}

void CalibrationPanel::mouseDown (const juce::MouseEvent& event)
{
    const int position = positionAt (event.position);
    if (position < 0)
        return;

    // Click selects just this position, Shift- or Cmd-click adds or removes it,
    // and dragging across positions adds each one
    auto selection = player.getSelection();
    const bool adding = event.mods.isShiftDown() || event.mods.isCommandDown();
    if (! adding)
        selection = { position };
    else if (selection.count (position) > 0)
        selection.erase (position);
    else
        selection.insert (position);

    dragAdds = ! adding || selection.count (position) > 0;
    player.setSelection (selection);

    if (! player.isPlaying())
    {
        player.setPlaying (true);
        playButton.setToggleState (true, juce::dontSendNotification);
    }
    updateButtons();
}

void CalibrationPanel::mouseDrag (const juce::MouseEvent& event)
{
    const int position = positionAt (event.position);
    if (position < 0)
        return;

    auto selection = player.getSelection();
    const bool changed = dragAdds ? selection.insert (position).second : selection.erase (position) > 0;
    if (changed)
    {
        player.setSelection (selection);
        updateButtons();
    }
}

void CalibrationPanel::mouseMove (const juce::MouseEvent& event)
{
    const int position = positionAt (event.position);
    if (position != hoverPosition)
    {
        hoverPosition = position;
        setMouseCursor (position >= 0 ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint (gridArea.expanded (8));
    }
}

void CalibrationPanel::mouseExit (const juce::MouseEvent&)
{
    hoverPosition = -1;
    repaint (gridArea.expanded (8));
}

//==============================================================================
void CalibrationPanel::paint (juce::Graphics& g)
{
    g.fillAll (Theme::sidebar);
    g.setColour (Theme::border);
    g.drawHorizontalLine (0, 0.0f, (float) getWidth());

    g.setColour (Theme::textFaint);
    g.setFont (Theme::font (11.0f, true));
    g.drawText ("CALIBRATION", getLocalBounds().reduced (16, 0).removeFromTop (36), juce::Justification::centredLeft);

    // Which way is which
    g.setFont (Theme::font (10.5f));
    g.drawText ("Left", gridArea.getX(), gridArea.getBottom() + 4, 60, 14, juce::Justification::centredLeft);
    g.drawText ("Right", gridArea.getRight() - 60, gridArea.getBottom() + 4, 60, 14, juce::Justification::centredRight);
    // Each row's low cut
    for (int row = 0; row < rows(); ++row)
    {
        const double cutoff = CalibrationPlayer::cutoffForRow (row, rows());
        const auto text = cutoff <= 0.0 ? juce::String ("Full")
                        : cutoff >= 1000.0 ? juce::String (cutoff / 1000.0, cutoff >= 10000.0 ? 0 : 1) + "k"
                                           : juce::String (juce::roundToInt (cutoff));
        g.drawText (text, gridArea.getX() - 46, (int) centreOf (row * columns()).y - 7, 38, 14, juce::Justification::centredRight);
    }

    g.setColour (Theme::panel);
    g.fillRoundedRectangle (gridArea.toFloat().expanded (6.0f), Theme::cornerRadius);

    const int count = rows() * columns();
    const float radius = juce::jlimit (4.0f, 9.0f, std::min ((float) gridArea.getWidth() / (float) columns(), (float) gridArea.getHeight() / (float) rows()) * 0.22f);

    for (int position = 0; position < count; ++position)
    {
        const auto centre = centreOf (position);
        auto dot = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);
        const bool isCurrent = position == shownPosition;

        if (isCurrent)
        {
            g.setColour (Theme::accent.withAlpha (0.3f));
            g.fillEllipse (dot.expanded (radius * 0.9f));
        }

        g.setColour (isCurrent ? Theme::accentBright : position == hoverPosition ? Theme::textDim : Theme::raisedHover);
        g.fillEllipse (dot);

        if (player.isSelected (position))
        {
            g.setColour (Theme::text);
            g.drawEllipse (dot.expanded (3.0f), 1.5f);
        }
    }
}

void CalibrationPanel::resized()
{
    auto area = getLocalBounds().reduced (16, 0);
    auto header = area.removeFromTop (36);
    closeButton.setBounds (header.removeFromRight (60).withSizeKeepingCentre (60, 26));
    header.removeFromRight (8);
    playButton.setBounds (header.removeFromRight (70).withSizeKeepingCentre (70, 26));
    header.removeFromRight (8);
    allButton.setBounds (header.removeFromRight (78).withSizeKeepingCentre (78, 26));

    area.removeFromBottom (12);
    auto controls = area.removeFromRight (std::min (300, area.getWidth() / 3));
    controls.removeFromLeft (20);
    for (auto [slider, label] : { std::pair { &rowsSlider, &rowsLabel }, std::pair { &columnsSlider, &columnsLabel }, std::pair { &volumeSlider, &volumeLabel } })
    {
        auto row = controls.removeFromTop (36);
        label->setBounds (row.removeFromLeft (64));
        slider->setBounds (row.withSizeKeepingCentre (row.getWidth(), 28));
    }

    gridArea = area.withTrimmedLeft (44).withTrimmedBottom (20).reduced (6);
}
