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
    const juce::Identifier idRepeats { "calibrationDepthRepeats" };
    const juce::Identifier idDepthUnit { "calibrationDepthUnit" };
}

//==============================================================================
/// What the Pin button shows: whether the bottom and top heights are pinned, and to what
class HeightPinsPopup : public juce::Component
{
public:
    HeightPinsPopup (CalibrationPlayer::Heights heights, std::function<void (bool, float, float)> onChangeToUse)
        : onChange (std::move (onChangeToUse))
    {
        pinToggle.setButtonText ("Pin the bottom and top");
        pinToggle.setToggleState (heights.pinned, juce::dontSendNotification);
        pinToggle.setTooltip ("The lowest height is exactly the bottom frequency and the highest exactly the top, with the rest spaced between. "
                              "Off, they're spread between 20 Hz and 20 kHz with room left at both ends.");
        pinToggle.onClick = [this] { changed(); };
        addAndMakeVisible (pinToggle);

        for (auto* field : { &bottomField, &topField })
        {
            field->format = [] (double v) { return Format::frequency (v); };
            field->parse = Format::parseFrequency;
            field->setTooltip ("Drag, scroll or double-click to type");
            field->onValueChange = [this] (double) { changed(); };
            addAndMakeVisible (*field);
        }
        bottomField.setValue (heights.low);
        topField.setValue (heights.high);
        updateEnablement();
        setSize (236, 92);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (4);
        pinToggle.setBounds (area.removeFromTop (26));
        area.removeFromTop (8);
        bottomField.setBounds (area.removeFromLeft ((area.getWidth() - 8) / 2));
        area.removeFromLeft (8);
        topField.setBounds (area);
    }

private:
    void changed()
    {
        updateEnablement();
        onChange (pinToggle.getToggleState(), (float) bottomField.getValue(), (float) topField.getValue());
    }

    void updateEnablement()
    {
        bottomField.setEnabled (pinToggle.getToggleState());
        topField.setEnabled (pinToggle.getToggleState());
    }

    std::function<void (bool, float, float)> onChange;
    juce::ToggleButton pinToggle;
    ValueField bottomField { "Bottom", 20.0, 20000.0, CalibrationPlayer::defaultPinLow, ValueField::Scale::logarithmic },
               topField { "Top", 20.0, 20000.0, CalibrationPlayer::defaultPinHigh, ValueField::Scale::logarithmic };
};

//==============================================================================
CalibrationPanel::CalibrationPanel (CabinEqAudioProcessor& p)
    : processor (p), player (p.getCalibration()), guidedView (p)
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
    depthSlider.setTooltip ("Plays each row this many times before the next row, a level louder each time, from the floor up to the volume");
    volumeSlider.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " dB"; };
    volumeSlider.updateText();
    volumeSlider.setTooltip ("How loud the calibration sounds are, separately from everything else");

    playButton.setClickingTogglesState (true);
    playButton.onClick = [this]
    {
        // Guided mode starts (or carries on) its search; the others just play
        if (! isGuidedMode())
            player.setPlaying (playButton.getToggleState());
        else if (playButton.getToggleState())
            guidedView.start();
        else
            guidedView.pause();
        updateButtons();
    };
    guidedView.onPlayingChanged = [this]
    {
        playButton.setToggleState (player.isPlaying(), juce::dontSendNotification);
        updateButtons();
    };
    addChildComponent (guidedView);
    allButton.onClick = [this]
    {
        player.setSelection ({});
        updateButtons();
    };
    allButton.setTooltip ("Go back to playing every position in turn");
    closeButton.onClick = [this] { if (onCloseClicked) onCloseClicked(); };

    for (auto* button : { &playButton, &allButton, &closeButton })
        addAndMakeVisible (button);

    // How many times a row plays at each depth level
    setUpSlider (repeatsSlider, juce::Slider::LinearHorizontal, 1, CalibrationPlayer::maxDepthRepeats, 1, (int) state.getProperty (idRepeats, 1));
    repeatsSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    repeatsSlider.textFromValueFunction = [] (double v) { return juce::String ((int) v) + (v > 1 ? " times" : " time"); };
    repeatsSlider.updateText();
    repeatsSlider.setTooltip ("Plays each row this many times at each depth level before going up to the next");
    setUpLabel (repeatsLabel, "Repeats");

    // What the depth and repeats work on
    const char* unitNames[] { "Dot", "Row", "Cycle" };
    const char* unitTips[] {
        "Each dot plays at every level (and its repeats) before the next dot",
        "Each row plays at every level (and its repeats) before the next row. On the graph, a row is a spot across its pan steps.",
        "Every dot plays at one level (and its repeats), then every dot at the next level"
    };
    for (size_t i = 0; i < depthUnitButtons.size(); ++i)
    {
        auto& button = depthUnitButtons[i];
        button.setButtonText (unitNames[i]);
        button.setTooltip (unitTips[i]);
        button.setConnectedEdges ((i > 0 ? juce::Button::ConnectedOnLeft : 0) | (i + 1 < depthUnitButtons.size() ? juce::Button::ConnectedOnRight : 0));
        button.onClick = [this, i] { setDepthUnit ((CalibrationPlayer::DepthUnit) i); };
        addAndMakeVisible (button);
    }
    setUpLabel (depthUnitLabel, "On each");
    setDepthUnit ((CalibrationPlayer::DepthUnit) juce::jlimit (0, 2, (int) state.getProperty (idDepthUnit, (int) CalibrationPlayer::DepthUnit::row)));

    // How the heights are spaced
    const char* spacingNames[] { "Log", "Mel", "Hz" };
    const char* spacingTips[] {
        "The low cuts are evenly spaced in octaves: each step up is the same ratio. The usual.",
        "The low cuts are evenly spaced in mels, roughly how pitch is heard: between the other two",
        "The low cuts are evenly spaced in Hz: each step up is the same number of Hz, so nearly all of them are high"
    };
    for (size_t i = 0; i < spacingButtons.size(); ++i)
    {
        auto& button = spacingButtons[i];
        button.setButtonText (spacingNames[i]);
        button.setTooltip (spacingTips[i]);
        button.setConnectedEdges ((i > 0 ? juce::Button::ConnectedOnLeft : 0) | (i + 1 < spacingButtons.size() ? juce::Button::ConnectedOnRight : 0));
        button.onClick = [this, i] { setHeightSpacing ((CalibrationPlayer::HeightSpacing) i); };
        addAndMakeVisible (button);
    }
    pinButton.setTooltip ("Pin the bottom and top heights to frequencies of your own, rather than spreading them between 20 Hz and 20 kHz");
    pinButton.onClick = [this] { showHeightPins(); };
    addAndMakeVisible (pinButton);
    setUpLabel (spacingLabel, "Heights");
    spacingLabel.setJustificationType (juce::Justification::centredRight);

    // Spots on the EQ graph, the grid, or a square to drag about
    guidedModeButton.onClick = [this] { setMode (CalibrationPlayer::Mode::sequence); };
    guidedModeButton.setTooltip ("Experimental: a guided search. It gives you one change to the curve at a time to try by ear, and keeps the ones that hold up.");
    for (auto* button : { &spotsModeButton, &gridModeButton, &squareModeButton, &guidedModeButton })
    {
        button->setClickingTogglesState (false);
        addAndMakeVisible (*button);
    }
    squareModeButton.onClick = [this] { setMode (CalibrationPlayer::Mode::square); };
    squareModeButton.setTooltip ("A square of dots you drag anywhere: across is from your left ear to your right, and up is a higher low cut");
    for (size_t i = 0; i < squareSizeButtons.size(); ++i)
    {
        const int size = (int) i + 2;
        auto& button = squareSizeButtons[i];
        button.setButtonText (juce::String (size) + " x " + juce::String (size));
        button.setTooltip (juce::String (size * size) + " dots, " + juce::String (size) + " along each side");
        button.onClick = [this, size]
        {
            auto square = player.getSquare();
            square.size = size;
            setSquare (square);
        };
        addChildComponent (button);
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
    panStepsSlider.setTooltip ("Plays each spot at this many positions across the pan range, left to right, before the next spot");
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
    guidedView.leave();
    player.setPlaying (false);
    playButton.setToggleState (false, juce::dontSendNotification);
    updateButtons();
}

CalibrationPlayer::Mode CalibrationPanel::mode() const
{
    return CalibrationSettings::getMode (processor.parameters.state);
}

bool CalibrationPanel::isShowingSpots() const
{
    return mode() == CalibrationPlayer::Mode::spots;
}

//==============================================================================
// Drag mode. The field is the grid's area: (0, 0) is its bottom left, (1, 1) its top right.
juce::Point<float> CalibrationPanel::toPanel (juce::Point<float> onField) const
{
    return { (float) gridArea.getX() + onField.x * (float) gridArea.getWidth(),
             (float) gridArea.getBottom() - onField.y * (float) gridArea.getHeight() };
}

juce::Point<float> CalibrationPanel::toField (juce::Point<float> onPanel) const
{
    return { (onPanel.x - (float) gridArea.getX()) / (float) std::max (1, gridArea.getWidth()),
             ((float) gridArea.getBottom() - onPanel.y) / (float) std::max (1, gridArea.getHeight()) };
}

juce::Rectangle<float> CalibrationPanel::squareOutline() const
{
    const auto square = player.getSquare();
    const auto topLeft = toPanel ({ square.x - square.width * 0.5f, square.y + square.height * 0.5f });
    const auto bottomRight = toPanel ({ square.x + square.width * 0.5f, square.y - square.height * 0.5f });
    return juce::Rectangle<float> (topLeft, bottomRight).expanded (15.0f);
}

juce::Rectangle<float> CalibrationPanel::squareHandle() const
{
    return juce::Rectangle<float> (16.0f, 16.0f).withCentre (squareOutline().getBottomRight().translated (-2.0f, -2.0f));
}

void CalibrationPanel::setSquare (CalibrationPlayer::Square square)
{
    CalibrationSettings::setSquare (processor.parameters.state, player, square);
    refreshSpots();
    repaint();
}

void CalibrationPanel::startPlaying()
{
    if (! player.isPlaying())
    {
        player.setPlaying (true);
        playButton.setToggleState (true, juce::dontSendNotification);
    }
    updateButtons();
}

void CalibrationPanel::paintSquareMode (juce::Graphics& g)
{
    const auto field = gridArea.toFloat();
    g.setFont (Theme::font (10.5f));
    g.setColour (Theme::textFaint);
    g.drawText ("Left", gridArea.getX(), gridArea.getBottom() + 4, 60, 14, juce::Justification::centredLeft);
    g.drawText ("Right", gridArea.getRight() - 60, gridArea.getBottom() + 4, 60, 14, juce::Justification::centredRight);
    g.drawText ("Drag the square to move it, and its corner to resize it. Higher up is a higher low cut.",
                gridArea.getX(), gridArea.getBottom() + 22, gridArea.getWidth(), 28, juce::Justification::centredLeft, true);

    g.setColour (Theme::panel);
    g.fillRoundedRectangle (field.expanded (6.0f), Theme::cornerRadius);

    // Up the side: the low cut at that height, at whichever frequencies are well apart with this spacing
    using Ticks = std::array<std::pair<float, const char*>, 4>;
    const auto heights = player.getHeights();
    const auto spacing = heights.spacing;
    auto ticks = spacing == CalibrationPlayer::HeightSpacing::hertz ? Ticks { { { 20.0f, "20" }, { 5000.0f, "5k" }, { 10000.0f, "10k" }, { 15000.0f, "15k" } } }
               : spacing == CalibrationPlayer::HeightSpacing::mels  ? Ticks { { { 20.0f, "20" }, { 1000.0f, "1k" }, { 3000.0f, "3k" }, { 10000.0f, "10k" } } }
                                                                    : Ticks { { { 20.0f, "20" }, { 100.0f, "100" }, { 1000.0f, "1k" }, { 10000.0f, "10k" } } };
    // Pinned, the field's between your own two frequencies: those, and two evenly between
    std::array<juce::String, 4> pinnedLabels;
    if (heights.pinned)
        for (size_t i = 0; i < ticks.size(); ++i)
        {
            ticks[i].first = (float) CalibrationPlayer::frequencyAtHeight ((double) i / 3.0, heights);
            pinnedLabels[i] = Format::frequencyShort (ticks[i].first);
            ticks[i].second = pinnedLabels[i].toRawUTF8();
        }
    for (auto [frequency, label] : ticks)
    {
        const float y = toPanel ({ 0.0f, (float) CalibrationPlayer::heightOfFrequency (frequency, heights) }).y;
        g.setColour (Theme::textFaint);
        g.drawText (label, gridArea.getX() - 46, (int) y - 7, 38, 14, juce::Justification::centredRight);
        if (frequency > ticks[0].first)
        {
            g.setColour (Theme::border.withAlpha (0.6f));
            g.drawHorizontalLine ((int) y, field.getX(), field.getRight());
        }
    }

    juce::Graphics::ScopedSaveState clip (g);
    g.reduceClipRegion (field.expanded (6.0f).toNearestInt());

    // The square, its corner, and its dots
    const auto outline = squareOutline();
    const bool isHot = squareDrag != SquareDrag::none || outline.contains (getMouseXYRelative().toFloat());
    g.setColour (Theme::accent.withAlpha (isHot ? 0.10f : 0.06f));
    g.fillRoundedRectangle (outline, 8.0f);
    g.setColour (Theme::accent.withAlpha (isHot ? 0.8f : 0.45f));
    g.drawRoundedRectangle (outline, 8.0f, 1.2f);

    const auto corner = outline.getBottomRight();
    juce::Path grip;
    for (float inset : { 5.0f, 9.0f })
    {
        grip.startNewSubPath (corner.x - inset - 3.0f, corner.y - 3.0f);
        grip.lineTo (corner.x - 3.0f, corner.y - inset - 3.0f);
    }
    g.strokePath (grip, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto square = player.getSquare();
    const float radius = 6.0f;
    for (int position = 0; position < square.size * square.size; ++position)
    {
        auto dot = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (toPanel (square.dot (position)));
        const bool isCurrent = position == shownPosition;
        if (isCurrent)
        {
            g.setColour (Theme::accent.withAlpha (0.3f));
            g.fillEllipse (dot.expanded (radius * 0.9f));
        }
        g.setColour (isCurrent ? Theme::accentBright : Theme::raisedHover.brighter (0.15f));
        g.fillEllipse (dot);
    }
}

void CalibrationPanel::setMode (CalibrationPlayer::Mode mode)
{
    // Leaving guided mode stops its search's playing, and drops whatever it was trying that hadn't been kept;
    // coming to it starts with nothing playing
    if (isGuidedMode() != (mode == CalibrationPlayer::Mode::sequence))
    {
        guidedView.leave();
        player.setPlaying (false);
        playButton.setToggleState (false, juce::dontSendNotification);
    }
    CalibrationSettings::setMode (processor.parameters.state, player, mode);
    refreshSpots();
    resized();
    repaint();
    if (onModeChanged)
        onModeChanged();
}

void CalibrationPanel::refreshSpots()
{
    const bool spots = isShowingSpots(), square = isSquareMode(), guided = isGuidedMode(), grid = ! spots && ! square && ! guided;
    guidedModeButton.setToggleState (guided, juce::dontSendNotification);
    guidedView.setVisible (guided);
    updateDepthControls();
    const int count = CalibrationSettings::getSpotCount (processor.parameters.state);

    gridModeButton.setToggleState (grid, juce::dontSendNotification);
    spotsModeButton.setToggleState (spots, juce::dontSendNotification);
    squareModeButton.setToggleState (square, juce::dontSendNotification);
    for (size_t i = 0; i < squareSizeButtons.size(); ++i)
    {
        squareSizeButtons[i].setVisible (square);
        squareSizeButtons[i].setToggleState (player.getSquare().size == (int) i + 2, juce::dontSendNotification);
    }
    for (int i = 0; i < (int) spotCountButtons.size(); ++i)
    {
        spotCountButtons[(size_t) i].setToggleState (count == i + 2, juce::dontSendNotification);
        spotCountButtons[(size_t) i].setVisible (spots);
    }

    for (auto* component : std::initializer_list<juce::Component*> { &rowsSlider, &columnsSlider })
        component->setVisible (grid);
    for (auto* component : std::initializer_list<juce::Component*> { &spotsPan, &panStepsSlider, &panStepsLabel })
        component->setVisible (spots);

    // On the graph you put the spots where you like, so there's nothing to space
    const auto spacing = (int) player.getHeightSpacing();
    for (size_t i = 0; i < spacingButtons.size(); ++i)
    {
        spacingButtons[i].setToggleState ((int) i == spacing, juce::dontSendNotification);
        spacingButtons[i].setEnabled (! spots);
    }
    pinButton.setEnabled (! spots);
    pinButton.setToggleState (player.getHeights().pinned, juce::dontSendNotification);
    spacingLabel.setEnabled (! spots);

    const auto pan = CalibrationSettings::getPanRange (processor.parameters.state);
    if (! spotsPan.isMouseButtonDown())
    {
        spotsPan.setMinAndMaxValues (pan.getStart(), pan.getEnd(), juce::dontSendNotification);
    }
    updateButtons();
}

void CalibrationPanel::setHeightSpacing (CalibrationPlayer::HeightSpacing spacing)
{
    CalibrationSettings::setHeightSpacing (processor.parameters.state, player, spacing); // not undoable, like the rest
    refreshSpots();
    repaint();
}

void CalibrationPanel::setHeightPins (bool shouldPin, float low, float high)
{
    CalibrationSettings::setHeightPins (processor.parameters.state, player, shouldPin, low, high); // not undoable either
    refreshSpots();
    repaint();
}

void CalibrationPanel::showHeightPins()
{
    auto content = std::make_unique<HeightPinsPopup> (player.getHeights(), [safeThis = juce::Component::SafePointer<CalibrationPanel> (this)] (bool pin, float low, float high)
    {
        if (safeThis != nullptr)
            safeThis->setHeightPins (pin, low, high);
    });
    content->setLookAndFeel (&getLookAndFeel());
    auto& box = juce::CallOutBox::launchAsynchronously (std::move (content), pinButton.getScreenBounds(), nullptr);
    box.setLookAndFeel (&getLookAndFeel());
}

void CalibrationPanel::setDepthUnit (CalibrationPlayer::DepthUnit unit)
{
    player.setDepthUnit (unit);
    processor.parameters.state.setProperty (idDepthUnit, (int) unit, nullptr); // not undoable, like the rest
    for (size_t i = 0; i < depthUnitButtons.size(); ++i)
        depthUnitButtons[i].setToggleState ((int) i == (int) unit, juce::dontSendNotification);
}

void CalibrationPanel::updateDepthControls()
{
    // Guided mode plays its own arrangements, at levels from the floor up: depth and repeats aren't its
    const bool guided = isGuidedMode();
    for (auto* component : std::initializer_list<juce::Component*> { &depthSlider, &depthLabel, &repeatsSlider, &repeatsLabel, &depthUnitLabel,
                                                                    &depthUnitButtons[0], &depthUnitButtons[1], &depthUnitButtons[2] })
        component->setEnabled (! guided);

    // Otherwise the floor is the quietest level, so it means nothing with a depth of one
    const bool hasRange = guided || depthSlider.getValue() > 1.0;
    floorSlider.setEnabled (hasRange);
    floorLabel.setEnabled (hasRange);
    floorSlider.setTooltip (guided ? "The quietest of the three levels a guided frame plays at, as dB below the volume. The middle one is halfway."
                            : hasRange ? "The quietest level a row plays at, as dB below the volume. Each level after it is a step louder, up to the volume."
                                       : "Turn the depth up to play each row at several levels; this sets the quietest of them");
}

void CalibrationPanel::applySettings()
{
    player.setGrid (rows(), columns());
    player.setLevelDb ((float) volumeSlider.getValue());
    player.setDepth ((int) depthSlider.getValue());
    player.setFloorDb ((float) floorSlider.getValue());
    player.setDepthRepeats ((int) repeatsSlider.getValue());
    updateDepthControls();
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
    state.setProperty (idRepeats, (int) repeatsSlider.getValue(), nullptr);
    state.setProperty (idSpeed, speedSlider.getValue(), nullptr);
    state.setProperty (idRelease, releaseSlider.getValue(), nullptr);
    state.setProperty (idAttack, attackSlider.getValue(), nullptr);

    updateButtons();
    repaint();
}

void CalibrationPanel::updateButtons()
{
    playButton.setButtonText (player.isPlaying() ? "Stop" : "Play");
    allButton.setVisible (mode() == CalibrationPlayer::Mode::grid && ! player.getSelection().empty());
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
    if (isShowingSpots() || isGuidedMode())
        return;

    if (isSquareMode())
    {
        // The corner resizes it; anywhere else on the field moves it, bringing it to the mouse first if it's elsewhere
        squareDrag = SquareDrag::none;
        if (squareHandle().contains (event.position))
        {
            // From the mouse to the bottom right dot, so the corner doesn't jump to the mouse
            const auto square = player.getSquare();
            squareDrag = SquareDrag::resize;
            squareGrab = event.position - toPanel ({ square.x + square.width * 0.5f, square.y - square.height * 0.5f });
        }
        else if (gridArea.toFloat().expanded (6.0f).contains (event.position))
        {
            auto square = player.getSquare();
            if (! squareOutline().contains (event.position))
            {
                const auto at = toField (event.position);
                square.x = at.x;
                square.y = at.y;
                setSquare (square);
                square = player.getSquare();
            }
            squareDrag = SquareDrag::move;
            squareGrab = toField (event.position) - juce::Point<float> (square.x, square.y);
        }
        if (squareDrag != SquareDrag::none)
        {
            grabKeyboardFocus();
            startPlaying();
        }
        return;
    }
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
    if (isShowingSpots() || isGuidedMode())
        return;

    if (isSquareMode())
    {
        auto square = player.getSquare();
        const auto at = toField (event.position);
        if (squareDrag == SquareDrag::move)
        {
            square.x = at.x - squareGrab.x;
            square.y = at.y - squareGrab.y;
        }
        else if (squareDrag == SquareDrag::resize)
        {
            // About its centre, which stays put: the bottom right dot follows the mouse
            const auto centre = toPanel ({ square.x, square.y });
            const auto corner = event.position - squareGrab;
            square.width = 2.0f * (corner.x - centre.x) / (float) std::max (1, gridArea.getWidth());
            square.height = 2.0f * (corner.y - centre.y) / (float) std::max (1, gridArea.getHeight());
            // No wider or taller than still fits where it is
            square.width = std::min (square.width, 2.0f * std::min (square.x, 1.0f - square.x));
            square.height = std::min (square.height, 2.0f * std::min (square.y, 1.0f - square.y));
        }
        else
        {
            return;
        }
        setSquare (square);
        return;
    }
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
    if (isShowingSpots() || isGuidedMode())
        return false;

    if (isSquareMode())
    {
        // Arrows move the square; with Shift, a small step
        const float step = key.getModifiers().isShiftDown() ? 0.005f : 0.025f;
        auto square = player.getSquare();
        if (key.getKeyCode() == juce::KeyPress::upKey)         square.y += step;
        else if (key.getKeyCode() == juce::KeyPress::downKey)  square.y -= step;
        else if (key.getKeyCode() == juce::KeyPress::leftKey)  square.x -= step;
        else if (key.getKeyCode() == juce::KeyPress::rightKey) square.x += step;
        else return false;
        setSquare (square);
        return true;
    }
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

void CalibrationPanel::mouseUp (const juce::MouseEvent&)
{
    squareDrag = SquareDrag::none;
    repaint();
}

void CalibrationPanel::mouseMove (const juce::MouseEvent& event)
{
    if (isGuidedMode())
        return;
    if (isSquareMode())
    {
        setMouseCursor (squareHandle().contains (event.position) ? juce::MouseCursor::BottomRightCornerResizeCursor
                        : squareOutline().contains (event.position) ? juce::MouseCursor::DraggingHandCursor
                        : gridArea.toFloat().contains (event.position) ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint (gridArea.expanded (8));
        return;
    }

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

    if (isGuidedMode())
        return; // its view draws itself

    if (isSquareMode())
    {
        paintSquareMode (g);
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
        const double cutoff = CalibrationPlayer::cutoffForRow (row, rows(), player.getHeights());
        const auto text = cutoff >= 10000.0 ? juce::String (juce::roundToInt (cutoff / 1000.0)) + "k"
                        : cutoff >= 1000.0 ? juce::String (cutoff / 1000.0, 1) + "k"
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
    header.removeFromLeft (4);
    squareModeButton.setBounds (header.removeFromLeft (64).withSizeKeepingCentre (64, 26));
    header.removeFromLeft (4);
    guidedModeButton.setBounds (header.removeFromLeft (72).withSizeKeepingCentre (72, 26));

    // How the heights are spaced, if there's room for it: its label goes first when there isn't much
    header.removeFromLeft (8);
    const int spacingWidth = 3 * 34, labelWidth = 48, pinWidth = 38;
    const bool fitsButtons = header.getWidth() >= spacingWidth + pinWidth, fitsLabel = header.getWidth() >= spacingWidth + pinWidth + labelWidth;
    spacingLabel.setBounds (fitsLabel ? header.removeFromLeft (labelWidth).withSizeKeepingCentre (labelWidth, 26) : juce::Rectangle<int>());
    auto spacingRow = fitsButtons ? header.removeFromLeft (spacingWidth).withSizeKeepingCentre (spacingWidth, 24) : juce::Rectangle<int>();
    for (auto& button : spacingButtons)
        button.setBounds (fitsButtons ? spacingRow.removeFromLeft (34) : juce::Rectangle<int>());
    pinButton.setBounds (fitsButtons ? header.removeFromLeft (pinWidth).withTrimmedLeft (4).withSizeKeepingCentre (pinWidth - 4, 24) : juce::Rectangle<int>());

    area.removeFromBottom (10);
    auto controls = area.removeFromRight (std::min (300, area.getWidth() / 3));
    controls.removeFromLeft (20);
    const std::vector<std::pair<juce::Slider*, juce::Label*>> rows { { &volumeSlider, &volumeLabel }, { &speedSlider, &speedLabel }, { &attackSlider, &attackLabel },
                                                                    { &releaseSlider, &releaseLabel }, { &depthSlider, &depthLabel }, { &repeatsSlider, &repeatsLabel },
                                                                    { &floorSlider, &floorLabel } };
    const int rowHeight = std::min (30, controls.getHeight() / ((int) rows.size() + 1)); // and one for what the depth works on
    for (auto [slider, label] : rows)
    {
        if (slider == &floorSlider)
        {
            // Dot | Row | Cycle, between the repeats and the floor
            auto row = controls.removeFromTop (rowHeight);
            depthUnitLabel.setBounds (row.removeFromLeft (64));
            row = row.withSizeKeepingCentre (row.getWidth(), std::min (24, rowHeight - 2)).withTrimmedRight (2);
            const int each = row.getWidth() / (int) depthUnitButtons.size();
            for (auto& button : depthUnitButtons)
                button.setBounds (row.removeFromLeft (each));
        }

        auto row = controls.removeFromTop (rowHeight);
        label->setBounds (row.removeFromLeft (64));
        slider->setBounds (row.withSizeKeepingCentre (row.getWidth(), std::min (28, rowHeight)));
    }

    // Spots mode: how many, the pan range, and which one's playing
    spotsArea = area;
    guidedView.setBounds (area.withTrimmedRight (10).withTrimmedBottom (2));
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

    // Drag mode has its square's size where the rows slider is
    for (size_t i = 0; i < squareSizeButtons.size(); ++i)
        squareSizeButtons[i].setBounds (rowsStrip.getX(), gridArea.getY() - 4 + (int) i * 30, rowsStrip.getWidth() - 2, 26);
    columnsSlider.setBounds (juce::Rectangle<int> (gridArea.getX() - 6, columnsStrip.getY(), gridArea.getWidth() + 12, 28));

    // Captions in the corner the two sliders leave free
    rowsCaption = juce::Rectangle<int> (rowsStrip.getX(), gridArea.getBottom() + 4, rowsStrip.getWidth(), 14);
    columnsCaption = juce::Rectangle<int> (rowsStrip.getX(), columnsStrip.getY(), gridArea.getX() - 12 - rowsStrip.getX(), 28);
}
