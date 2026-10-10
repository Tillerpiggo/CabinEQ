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
    const juce::Identifier idRelease { "calibrationRelease" };
    const juce::Identifier idAttack { "calibrationAttack" };
    const juce::Identifier idFloor { "calibrationFloor" };
    const juce::Identifier idSteepNoise { "calibrationSteepNoise" };
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
    setUpSlider (releaseSlider, juce::Slider::LinearHorizontal, 0.0, CalibrationPlayer::maxReleaseMs, 10.0,
                 (double) state.getProperty (idRelease, CalibrationPlayer::defaultReleaseMs));
    releaseSlider.setSkewFactorFromMidPoint (600.0);
    releaseSlider.textFromValueFunction = [] (double v) { return v >= 1000.0 ? juce::String (v / 1000.0, 2) + " s" : juce::String ((int) v) + " ms"; };
    releaseSlider.setTooltip ("How long each burst takes to fade out. 0 ms cuts it off straight after the attack, for a short tick.");
    setUpSlider (attackSlider, juce::Slider::LinearHorizontal, 0.0, CalibrationPlayer::maxAttackMs, 1.0,
                 (double) state.getProperty (idAttack, CalibrationPlayer::defaultAttackMs));
    attackSlider.setSkewFactorFromMidPoint (30.0);
    attackSlider.textFromValueFunction = [] (double v) { return juce::String ((int) v) + " ms"; };
    attackSlider.setTooltip ("How long each burst takes to reach full level. Longer softens the start; 0 ms is the sharpest.");
    setUpLabel (attackLabel, "Attack");
    setUpSlider (floorSlider, juce::Slider::LinearHorizontal, CalibrationPlayer::minFloorDb, 0.0, 1.0,
                 (double) state.getProperty (idFloor, CalibrationPlayer::defaultFloorDb));
    floorSlider.textFromValueFunction = [] (double v) { return juce::String ((int) v) + " dB"; };
    setUpLabel (floorLabel, "Floor");
    for (auto* slider : { &volumeSlider, &depthSlider, &speedSlider, &releaseSlider, &attackSlider, &floorSlider })
        slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    speedSlider.updateText();
    releaseSlider.updateText();
    attackSlider.updateText();
    floorSlider.updateText();
    setUpLabel (releaseLabel, "Release");
    setUpLabel (volumeLabel, "Volume");
    setUpLabel (depthLabel, "Depth");
    setUpLabel (speedLabel, "Speed");
    depthSlider.textFromValueFunction = [] (double v) { return juce::String ((int) v) + (v > 1 ? " times" : " time"); };
    depthSlider.updateText();
    depthSlider.setTooltip ("Plays each position this many times, quietest first, climbing from the floor to the volume in even steps");
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

    // The noise the bursts are made of
    pinkNoiseButton.setTooltip ("Pink noise: the same power in every octave (it falls 3 dB an octave)");
    steepNoiseButton.setTooltip ("Steeper noise: it falls 4.5 dB an octave, and each burst is turned up 1.5 dB for every octave its "
                                 "lowest frequency is above 20 Hz, so it's as strong as pink at its bottom edge and softer above");
    pinkNoiseButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    steepNoiseButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    pinkNoiseButton.onClick = [this] { setSteepNoise (false); };
    steepNoiseButton.onClick = [this] { setSteepNoise (true); };
    for (auto* button : { &pinkNoiseButton, &steepNoiseButton })
    {
        button->setColour (juce::TextButton::buttonOnColourId, Theme::accent);
        button->setColour (juce::TextButton::textColourOnId, Theme::graph);
        addAndMakeVisible (button);
    }
    setSteepNoise ((bool) processor.parameters.state.getProperty (idSteepNoise, false));

    // Spots on the EQ graph, or the grid
    for (auto* button : { &spotsModeButton, &gridModeButton })
    {
        button->setClickingTogglesState (false);
        addAndMakeVisible (*button);
    }
    gridModeButton.onClick = [this] { setMode (CalibrationPlayer::Mode::grid); };
    spotsModeButton.onClick = [this] { setMode (CalibrationPlayer::Mode::spots); };
    spotsModeButton.setTooltip ("Play 2 to 4 spots that you place on the EQ graph, for lining up with bands exactly");

    for (int count = 2; count <= CalibrationPlayer::maxSpots; ++count)
    {
        auto& button = spotCountButtons[(size_t) (count - 2)];
        button.setButtonText (juce::String (count) + " spots");
        button.onClick = [this, count] { CalibrationSettings::setSpotCount (processor.parameters.state, player, count); refreshSpots(); };
        addChildComponent (button);
    }

    // The pan range the spots play across (two thumbs), and in how many steps
    spotsPan.setSliderStyle (juce::Slider::TwoValueHorizontal);
    spotsPan.setRange (-1.0, 1.0, 0.01);
    spotsPan.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    spotsPan.setTooltip ("The range the spots play across, from your left ear to your right. With more than one pan step, they sweep across it.");
    spotsPan.onValueChange = [this]
    {
        CalibrationSettings::setPanRange (processor.parameters.state, player, (float) spotsPan.getMinValue(), (float) spotsPan.getMaxValue());
        repaint();
    };
    addChildComponent (spotsPan);

    panStepsSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    panStepsSlider.setRange (1, CalibrationPlayer::maxPanSteps, 1);
    panStepsSlider.setValue (CalibrationSettings::getPanSteps (processor.parameters.state), juce::dontSendNotification);
    panStepsSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    panStepsSlider.textFromValueFunction = [] (double v) { return juce::String ((int) v) + ((int) v == 1 ? " step" : " steps"); };
    panStepsSlider.updateText();
    panStepsSlider.setTooltip ("Plays each spot at this many positions across the pan range, left to right, each with its whole depth run");
    panStepsSlider.onValueChange = [this] { CalibrationSettings::setPanSteps (processor.parameters.state, player, (int) panStepsSlider.getValue()); };
    addChildComponent (panStepsSlider);
    panStepsLabel.setText ("Pan steps", juce::dontSendNotification);
    panStepsLabel.setFont (Theme::font (12.0f));
    panStepsLabel.setColour (juce::Label::textColourId, Theme::textDim);
    addChildComponent (panStepsLabel);

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
    for (int i = 0; i < (int) spotCountButtons.size(); ++i)
    {
        spotCountButtons[(size_t) i].setToggleState (count == i + 2, juce::dontSendNotification);
        spotCountButtons[(size_t) i].setVisible (spots);
    }

    for (auto* component : std::initializer_list<juce::Component*> { &rowsSlider, &columnsSlider })
        component->setVisible (! spots);
    for (auto* component : std::initializer_list<juce::Component*> { &spotsPan, &panStepsSlider, &panStepsLabel })
        component->setVisible (spots);

    const auto pan = CalibrationSettings::getPanRange (processor.parameters.state);
    if (! spotsPan.isMouseButtonDown())
    {
        spotsPan.setMinAndMaxValues (pan.getStart(), pan.getEnd(), juce::dontSendNotification);
    }
    updateButtons();
}

void CalibrationPanel::setSteepNoise (bool shouldBeSteep)
{
    player.setSteepNoise (shouldBeSteep);
    processor.parameters.state.setProperty (idSteepNoise, shouldBeSteep, nullptr); // not undoable, like the rest
    pinkNoiseButton.setToggleState (! shouldBeSteep, juce::dontSendNotification);
    steepNoiseButton.setToggleState (shouldBeSteep, juce::dontSendNotification);
}

void CalibrationPanel::applySettings()
{
    player.setGrid (rows(), columns());
    player.setLevelDb ((float) volumeSlider.getValue());
    player.setDepth ((int) depthSlider.getValue());
    player.setFloorDb ((float) floorSlider.getValue());

    // The floor is where a depth run starts, so it means nothing with a depth of one
    const bool hasRange = depthSlider.getValue() > 1.0;
    floorSlider.setEnabled (hasRange);
    floorLabel.setEnabled (hasRange);
    floorSlider.setTooltip (hasRange ? "The quietest of each position's plays, as dB below the volume. The rest climb from here to the volume in even steps."
                                     : "Turn the depth up to play each position at several levels; this sets the quietest of them");
    player.setRate ((float) speedSlider.getValue());
    player.setReleaseMs ((float) releaseSlider.getValue());
    player.setAttackMs ((float) attackSlider.getValue());

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
    state.setProperty (idFloor, floorSlider.getValue(), nullptr);
    state.setProperty (idSpeed, speedSlider.getValue(), nullptr);
    state.setProperty (idRelease, releaseSlider.getValue(), nullptr);
    state.setProperty (idAttack, attackSlider.getValue(), nullptr);

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
        // Which spot is playing, as a row of coloured dots
        const int count = CalibrationSettings::getSpotCount (processor.parameters.state);
        for (int i = 0; i < count; ++i)
        {
            auto chip = spotDots.withWidth (spotDots.getHeight()).translated (i * (spotDots.getHeight() + 10), 0).toFloat();
            const auto colour = CalibrationSettings::spotColour (i);
            const bool isPlaying = player.isPlaying() && shownPosition == i;
            if (isPlaying)
            {
                g.setColour (colour.withAlpha (0.3f));
                g.fillEllipse (chip.expanded (4.0f));
            }
            g.setColour (isPlaying ? colour : colour.withAlpha (0.45f));
            g.fillEllipse (chip);
            g.setColour (Theme::graph);
            g.setFont (Theme::font (12.0f, true));
            g.drawText (CalibrationSettings::spotName (i), chip, juce::Justification::centred);
        }

        g.setColour (Theme::textDim);
        g.setFont (Theme::font (12.0f));
        g.drawText ("Pan", spotsPan.getBounds().translated (-40, 0).withWidth (36), juce::Justification::centredRight);
        g.drawText (CalibrationSettings::describePanRange (CalibrationSettings::getPanRange (processor.parameters.state)),
                    spotsPan.getBounds().translated (spotsPan.getWidth() + 8, 0).withWidth (120), juce::Justification::centredLeft);

        g.setColour (Theme::textFaint);
        g.setFont (Theme::font (11.5f));
        auto hint = spotsArea;
        g.drawText ("On the graph, drag the lowest or highest spot to resize them around the middle, and the middle to move them.",
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
    spotsModeButton.setBounds (header.removeFromLeft (84).withSizeKeepingCentre (84, 26));
    header.removeFromLeft (4);
    gridModeButton.setBounds (header.removeFromLeft (64).withSizeKeepingCentre (64, 26));
    header.removeFromLeft (16);
    pinkNoiseButton.setBounds (header.removeFromLeft (52).withSizeKeepingCentre (52, 26));
    steepNoiseButton.setBounds (header.removeFromLeft (92).withSizeKeepingCentre (92, 26));

    area.removeFromBottom (10);
    auto controls = area.removeFromRight (std::min (300, area.getWidth() / 3));
    controls.removeFromLeft (20);
    for (auto [slider, label] : { std::pair { &volumeSlider, &volumeLabel }, std::pair { &speedSlider, &speedLabel }, std::pair { &attackSlider, &attackLabel },
                                 std::pair { &releaseSlider, &releaseLabel },
                                 std::pair { &depthSlider, &depthLabel }, std::pair { &floorSlider, &floorLabel } })
    {
        auto row = controls.removeFromTop (30);
        label->setBounds (row.removeFromLeft (64));
        slider->setBounds (row.withSizeKeepingCentre (row.getWidth(), 28));
    }

    // Spots mode: how many, the pan range, and which one's playing
    spotsArea = area;
    {
        auto spots = area.withTrimmedRight (10);
        auto countRow = spots.removeFromTop (30);
        for (auto& button : spotCountButtons)
        {
            button.setBounds (countRow.removeFromLeft (72).withSizeKeepingCentre (72, 26));
            countRow.removeFromLeft (4);
        }
        spots.removeFromTop (12);

        auto panRow = spots.removeFromTop (30).withTrimmedLeft (40);
        spotsPan.setBounds (panRow.withWidth (std::min (panRow.getWidth() - 130, 320)).withSizeKeepingCentre (std::min (panRow.getWidth() - 130, 320), 28));
        spots.removeFromTop (14);
        spotDots = spots.removeFromTop (22).withTrimmedLeft (2);
    }

    // Pan steps sits under depth, which it stacks with
    {
        auto row = controls.removeFromTop (36);
        panStepsLabel.setBounds (row.removeFromLeft (64));
        panStepsSlider.setBounds (row.withSizeKeepingCentre (row.getWidth(), 28));
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
