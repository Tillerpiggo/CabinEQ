/*
  ==============================================================================

    CalibrationPanel.cpp

  ==============================================================================
*/

#include "CalibrationPanel.h"
#include "Theme.h"
#include "CalibrationSettings.h"
#include "Format.h"

namespace
{
    const juce::Identifier idRows { "calibrationRows" };
    const juce::Identifier idColumns { "calibrationColumns" };
    const juce::Identifier idLevel { "calibrationLevel" };
    const juce::Identifier idDepth { "calibrationDepth" };
    const juce::Identifier idSpeed { "calibrationSpeed" };
}

CalibrationPanel::CalibrationPanel (CabinEqAudioProcessor& p)
    : processor (p), player (p.getCalibration())
{
    auto& state = processor.parameters.state;

    auto setUpSlider = [this] (juce::Slider& slider, juce::Slider::SliderStyle style, double min, double max, double step, double value)
    {
        slider.setSliderStyle (style);
        slider.setRange (min, max, step);
        slider.setValue (value, juce::dontSendNotification);
        slider.onValueChange = [this] { applySettings(); };
        addAndMakeVisible (slider);
    };
    auto setUpLabel = [this] (juce::Label& label, const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setFont (Theme::font (12.0f));
        label.setColour (juce::Label::textColourId, Theme::textDim);
        addAndMakeVisible (label);
    };

    // Rows up the side of the grid, columns along the bottom: the grid's own size, not a number to read
    setUpSlider (rowsSlider, juce::Slider::LinearVertical, 1, CalibrationPlayer::maxRows, 1, (int) state.getProperty (idRows, CalibrationPlayer::defaultRows));
    setUpSlider (columnsSlider, juce::Slider::LinearHorizontal, 1, CalibrationPlayer::maxColumns, 1, (int) state.getProperty (idColumns, CalibrationPlayer::defaultColumns));
    for (auto* slider : { &rowsSlider, &columnsSlider })
    {
        slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider->setPopupDisplayEnabled (true, true, this);
    }
    rowsSlider.setTooltip ("Rows: how many low cuts to step through");
    columnsSlider.setTooltip ("Columns: how many positions from your left ear to your right");

    setUpSlider (volumeSlider, juce::Slider::LinearHorizontal, -60, 0, 0.5, (double) state.getProperty (idLevel, -20.0));
    setUpSlider (depthSlider, juce::Slider::LinearHorizontal, 1, CalibrationPlayer::maxDepth, 1, (int) state.getProperty (idDepth, 1));
    setUpSlider (speedSlider, juce::Slider::LinearHorizontal, CalibrationPlayer::minRate, CalibrationPlayer::maxRate, 0.1,
                 (double) state.getProperty (idSpeed, CalibrationPlayer::defaultRate));
    speedSlider.setSkewFactorFromMidPoint (2.5);
    speedSlider.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " / s"; };
    speedSlider.setTooltip ("How many bursts play each second");
    for (auto* slider : { &volumeSlider, &depthSlider, &speedSlider })
        slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    speedSlider.updateText();
    setUpLabel (volumeLabel, "Volume");
    setUpLabel (depthLabel, "Depth");
    setUpLabel (speedLabel, "Speed");
    depthSlider.textFromValueFunction = [] (double v) { return juce::String ((int) v) + (v > 1 ? " times" : " time"); };
    depthSlider.updateText();
    depthSlider.setTooltip ("Plays each position this many times, quietest first, 10 dB louder each time");
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

    // Grid, or spots on the EQ graph
    for (auto* button : { &gridModeButton, &spotsModeButton })
    {
        button->setClickingTogglesState (false);
        addAndMakeVisible (*button);
    }
    gridModeButton.onClick = [this] { setMode (CalibrationPlayer::Mode::grid); };
    spotsModeButton.onClick = [this] { setMode (CalibrationPlayer::Mode::spots); };
    spotsModeButton.setTooltip ("Play 2 or 3 spots that you place on the EQ graph, for lining up with bands exactly");

    twoSpotsButton.onClick = [this] { CalibrationSettings::setSpotCount (processor.parameters.state, player, 2); refreshSpots(); };
    threeSpotsButton.onClick = [this] { CalibrationSettings::setSpotCount (processor.parameters.state, player, 3); refreshSpots(); };
    addChildComponent (twoSpotsButton);
    addChildComponent (threeSpotsButton);

    for (int i = 0; i < CalibrationPlayer::maxSpots; ++i)
    {
        auto& controls = spotControls[(size_t) i];
        controls.frequency = std::make_unique<ValueField> ("Base frequency", 20.0, 16000.0, 1000.0, ValueField::Scale::logarithmic);
        controls.frequency->format = [] (double v) { return Format::frequency (v); };
        controls.frequency->parse = Format::parseFrequency;
        controls.frequency->setAccentColour (CalibrationSettings::spotColour (i));
        controls.frequency->setTooltip ("Where it starts: it plays everything from here up. You can also drag it on the graph.");
        controls.frequency->onValueChange = [this, i] (double v)
        {
            auto spot = CalibrationSettings::getSpot (processor.parameters.state, i);
            spot.frequency = (float) v;
            CalibrationSettings::setSpot (processor.parameters.state, player, i, spot);
        };
        addChildComponent (*controls.frequency);

        controls.pan.setSliderStyle (juce::Slider::LinearHorizontal);
        controls.pan.setRange (-1.0, 1.0, 0.01);
        controls.pan.setDoubleClickReturnValue (true, 0.0);
        controls.pan.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 20);
        controls.pan.textFromValueFunction = [] (double v) { return CalibrationSettings::describePan ((float) v); };
        controls.pan.setTooltip ("Where it is, from your left ear to your right. Double-click for the centre.");
        controls.pan.updateText();
        controls.pan.onValueChange = [this, i]
        {
            auto spot = CalibrationSettings::getSpot (processor.parameters.state, i);
            spot.pan = (float) spotControls[(size_t) i].pan.getValue();
            CalibrationSettings::setSpot (processor.parameters.state, player, i, spot);
        };
        addChildComponent (controls.pan);
    }

    CalibrationSettings::apply (processor.parameters.state, player);

    setWantsKeyboardFocus (true);
    applySettings();
    refreshSpots();
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

bool CalibrationPanel::isShowingSpots() const
{
    return CalibrationSettings::getMode (processor.parameters.state) == CalibrationPlayer::Mode::spots;
}

void CalibrationPanel::setMode (CalibrationPlayer::Mode mode)
{
    CalibrationSettings::setMode (processor.parameters.state, player, mode);
    refreshSpots();
    resized();
    repaint();
    if (onModeChanged)
        onModeChanged();
}

void CalibrationPanel::refreshSpots()
{
    const bool spots = isShowingSpots();
    const int count = CalibrationSettings::getSpotCount (processor.parameters.state);

    gridModeButton.setToggleState (! spots, juce::dontSendNotification);
    spotsModeButton.setToggleState (spots, juce::dontSendNotification);
    twoSpotsButton.setToggleState (count == 2, juce::dontSendNotification);
    threeSpotsButton.setToggleState (count == 3, juce::dontSendNotification);

    for (auto* component : std::initializer_list<juce::Component*> { &rowsSlider, &columnsSlider })
        component->setVisible (! spots);
    twoSpotsButton.setVisible (spots);
    threeSpotsButton.setVisible (spots);

    for (int i = 0; i < CalibrationPlayer::maxSpots; ++i)
    {
        auto& controls = spotControls[(size_t) i];
        const bool shown = spots && i < count;
        controls.frequency->setVisible (shown);
        controls.pan.setVisible (shown);

        // The graph may have moved it
        const auto spot = CalibrationSettings::getSpot (processor.parameters.state, i);
        controls.frequency->setValue (spot.frequency);
        if (! controls.pan.isMouseButtonDown())
            controls.pan.setValue (spot.pan, juce::dontSendNotification);
    }
    updateButtons();
}

void CalibrationPanel::applySettings()
{
    player.setGrid (rows(), columns());
    player.setLevelDb ((float) volumeSlider.getValue());
    player.setDepth ((int) depthSlider.getValue());
    player.setRate ((float) speedSlider.getValue());

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
    state.setProperty (idDepth, (int) depthSlider.getValue(), nullptr);
    state.setProperty (idSpeed, speedSlider.getValue(), nullptr);

    updateButtons();
    repaint();
}

void CalibrationPanel::updateButtons()
{
    playButton.setButtonText (player.isPlaying() ? "Stop" : "Play");
    allButton.setVisible (! isShowingSpots() && ! player.getSelection().empty());
    repaint();
}

void CalibrationPanel::timerCallback()
{
    const int current = player.getCurrentPosition();
    if (current != shownPosition)
    {
        shownPosition = current;
        repaint();
    }

    if (isShowingSpots())
        refreshSpots(); // pick up drags on the graph
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
    if (isShowingSpots())
        return;
    const int position = positionAt (event.position);
    if (position < 0)
        return;

    grabKeyboardFocus(); // so the arrow keys move the selection
    cursor = position;

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
    if (isShowingSpots())
        return;
    const int position = positionAt (event.position);
    if (position < 0)
        return;

    cursor = position;
    auto selection = player.getSelection();
    const bool changed = dragAdds ? selection.insert (position).second : selection.erase (position) > 0;
    if (changed)
    {
        player.setSelection (selection);
        updateButtons();
    }
}

bool CalibrationPanel::keyPressed (const juce::KeyPress& key)
{
    if (isShowingSpots())
        return false;
    int rowStep = 0, columnStep = 0;
    if (key.getKeyCode() == juce::KeyPress::upKey)         rowStep = -1;
    else if (key.getKeyCode() == juce::KeyPress::downKey)  rowStep = 1;
    else if (key.getKeyCode() == juce::KeyPress::leftKey)  columnStep = -1;
    else if (key.getKeyCode() == juce::KeyPress::rightKey) columnStep = 1;
    else return false;

    auto selection = player.getSelection();
    if (cursor < 0 || cursor >= rows() * columns())
        cursor = selection.empty() ? 0 : *selection.begin();

    auto moved = [this, rowStep, columnStep] (int position) -> int
    {
        const int row = position / columns() + rowStep, column = position % columns() + columnStep;
        return (row < 0 || row >= rows() || column < 0 || column >= columns()) ? -1 : row * columns() + column;
    };

    if (selection.empty())
    {
        // Nothing selected yet: start where the cursor is
        selection = { cursor };
    }
    else if (key.getModifiers().isShiftDown())
    {
        // Shift+arrow grows the selection one position that way
        if (const int next = moved (cursor); next >= 0)
        {
            selection.insert (next);
            cursor = next;
        }
    }
    else
    {
        // Arrows move the whole selection, as long as all of it stays on the grid
        std::set<int> shifted;
        for (int position : selection)
        {
            const int next = moved (position);
            if (next < 0)
                return true;
            shifted.insert (next);
        }
        selection = shifted;
        cursor = moved (cursor) >= 0 ? moved (cursor) : cursor;
    }

    player.setSelection (selection);
    if (! player.isPlaying())
    {
        player.setPlaying (true);
        playButton.setToggleState (true, juce::dontSendNotification);
    }
    updateButtons();
    return true;
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

    if (isShowingSpots())
    {
        // A row per spot, the playing one lit up
        const int count = CalibrationSettings::getSpotCount (processor.parameters.state);
        for (int i = 0; i < count; ++i)
        {
            const auto& controls = spotControls[(size_t) i];
            const auto colour = CalibrationSettings::spotColour (i);
            const bool isPlaying = player.isPlaying() && shownPosition == i;

            if (isPlaying)
            {
                g.setColour (colour.withAlpha (0.10f));
                g.fillRoundedRectangle (controls.row.toFloat().expanded (6.0f, 2.0f), Theme::cornerRadius);
            }

            auto chip = controls.row.withWidth (28).toFloat().withSizeKeepingCentre (26.0f, 26.0f);
            g.setColour (colour);
            g.fillEllipse (chip);
            g.setColour (Theme::graph);
            g.setFont (Theme::font (13.0f, true));
            g.drawText (CalibrationSettings::spotName (i), chip, juce::Justification::centred);

            g.setColour (Theme::textDim);
            g.setFont (Theme::font (12.0f));
            g.drawText ("Pan", controls.pan.getBounds().translated (-40, 0).withWidth (36), juce::Justification::centredRight);
        }

        g.setColour (Theme::textFaint);
        g.setFont (Theme::font (11.5f));
        auto hint = spotsArea;
        g.drawText ("Each spot plays from its frequency up. Drag a line on the graph to move them all, or a chip to move one.",
                    hint.removeFromBottom (18), juce::Justification::centredLeft, true);
        return;
    }

    // Which way is which, and what the axis sliders are
    g.setFont (Theme::font (10.5f));
    g.drawText ("Left", gridArea.getX(), gridArea.getBottom() + 4, 60, 14, juce::Justification::centredLeft);
    g.drawText ("Right", gridArea.getRight() - 60, gridArea.getBottom() + 4, 60, 14, juce::Justification::centredRight);
    g.drawText (juce::String (rows()) + (rows() == 1 ? " row" : " rows"), rowsCaption, juce::Justification::centred);
    g.drawText (juce::String (columns()) + (columns() == 1 ? " column" : " columns"), columnsCaption, juce::Justification::centredRight);
    // Each row's low cut
    for (int row = 0; row < rows(); ++row)
    {
        const double cutoff = CalibrationPlayer::cutoffForRow (row, rows());
        const auto text = cutoff >= 1000.0 ? juce::String (cutoff / 1000.0, cutoff >= 10000.0 ? 0 : 1) + "k"
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

    header.removeFromLeft (100); // the title
    gridModeButton.setBounds (header.removeFromLeft (64).withSizeKeepingCentre (64, 26));
    header.removeFromLeft (4);
    spotsModeButton.setBounds (header.removeFromLeft (84).withSizeKeepingCentre (84, 26));

    area.removeFromBottom (10);
    auto controls = area.removeFromRight (std::min (300, area.getWidth() / 3));
    controls.removeFromLeft (20);
    for (auto [slider, label] : { std::pair { &volumeSlider, &volumeLabel }, std::pair { &speedSlider, &speedLabel }, std::pair { &depthSlider, &depthLabel } })
    {
        auto row = controls.removeFromTop (36);
        label->setBounds (row.removeFromLeft (64));
        slider->setBounds (row.withSizeKeepingCentre (row.getWidth(), 28));
    }

    // Spots mode: how many, then a row each
    spotsArea = area;
    {
        auto spots = area.withTrimmedRight (10);
        auto countRow = spots.removeFromTop (30);
        twoSpotsButton.setBounds (countRow.removeFromLeft (72).withSizeKeepingCentre (72, 26));
        countRow.removeFromLeft (4);
        threeSpotsButton.setBounds (countRow.removeFromLeft (72).withSizeKeepingCentre (72, 26));
        spots.removeFromTop (6);

        for (auto& controls : spotControls)
        {
            controls.row = spots.removeFromTop (42);
            spots.removeFromTop (4);
            auto row = controls.row.withTrimmedLeft (40);
            controls.frequency->setBounds (row.removeFromLeft (140).withSizeKeepingCentre (140, 38));
            row.removeFromLeft (52);
            controls.pan.setBounds (row.withSizeKeepingCentre (std::min (row.getWidth(), 320), 28).withX (row.getX()));
        }
    }

    // Rows slider up the left, then the rows' cutoffs, then the grid, with columns along the bottom
    auto rowsStrip = area.removeFromLeft (56);
    auto columnsStrip = area.removeFromBottom (28);
    area.removeFromBottom (20); // Left / Right
    area.removeFromLeft (46);   // the cutoff labels
    gridArea = area.reduced (6);

    rowsSlider.setBounds (juce::Rectangle<int> (rowsStrip.getCentreX() - 14, gridArea.getY() - 6, 28, gridArea.getHeight() + 12));
    columnsSlider.setBounds (juce::Rectangle<int> (gridArea.getX() - 6, columnsStrip.getY(), gridArea.getWidth() + 12, 28));

    // Captions in the corner the two sliders leave free
    rowsCaption = juce::Rectangle<int> (rowsStrip.getX(), gridArea.getBottom() + 4, rowsStrip.getWidth(), 14);
    columnsCaption = juce::Rectangle<int> (rowsStrip.getX(), columnsStrip.getY(), gridArea.getX() - 12 - rowsStrip.getX(), 28);
}
