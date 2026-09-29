/*
  ==============================================================================

    BandInspector.cpp

  ==============================================================================
*/

#include "BandInspector.h"
#include "Format.h"
#include "CabinPeqGraph.h"

BandInspector::BandInspector (CabinEqAudioProcessor& p)
    : processor (p)
{
    int shapeId = 1;
    for (auto shape : { Band::Shape::peak, Band::Shape::lowShelf, Band::Shape::highShelf, Band::Shape::lowCut, Band::Shape::highCut })
        shapeBox.addItem (Band::shapeName (shape), shapeId++);
    shapeBox.onChange = [this]
    {
        const auto shape = static_cast<Band::Shape> (shapeBox.getSelectedId() - 1);
        edit ("Change shape", [shape] (Band& band)
        {
            const bool wasCut = ! band.hasGain();
            band.shape = shape;
            if (wasCut != ! band.hasGain())
                band.setQ (band.hasGain() && shape == Band::Shape::peak ? Band::defaultQ : Band::defaultCutQ);
        });
    };

    channelBox.addItem ("Both ears", 1);
    channelBox.addItem ("Left ear", 2);
    channelBox.addItem ("Right ear", 3);
    channelBox.onChange = [this]
    {
        const auto type = static_cast<Band::Type> (channelBox.getSelectedId() - 1);
        edit ("Change channel", [type] (Band& band) { band.type = type; });
    };

    frequencyField.format = [] (double v) { return Format::frequency (v); };
    frequencyField.parse = Format::parseFrequency;
    gainField.format = [] (double v) { return Format::gain (v); };
    qField.format = [] (double v) { return Format::q (v); };

    for (auto* field : { &frequencyField, &gainField, &qField })
    {
        field->onGestureStart = [this] { processor.getUndoManager().beginNewTransaction ("Edit band"); };
    }
    frequencyField.onValueChange = [this] (double v)
    {
        if (isCurveMode())
            editPoint ([this, v] (CurvePoint& point)
            {
                // An ear's point stays at the same level on screen as it moves along the shared curve
                const float shownDb = point.gain + layerBaseDb (point.freq);
                point.freq = juce::jlimit (CurvePoint::minFreq, CurvePoint::maxFreq, (float) v);
                point.gain = juce::jlimit (CurvePoint::minGain, CurvePoint::maxGain, shownDb - layerBaseDb (point.freq));
            });
        else
            edit ({}, [v] (Band& band) { band.freq = (float) v; });
    };
    gainField.onValueChange = [this] (double v)
    {
        if (isCurveMode())
            editPoint ([this, v] (CurvePoint& point)
            {
                point.gain = juce::jlimit (CurvePoint::minGain, CurvePoint::maxGain, (float) v - layerBaseDb (point.freq));
            });
        else
            edit ({}, [v] (Band& band) { band.ampl = (float) v; });
    };
    qField.onValueChange = [this] (double v) { edit ({}, [v] (Band& band) { band.setQ ((float) v); }); };

    frequencyField.setTooltip ("Drag, scroll or double-click to type. Shift for fine control.");
    gainField.setTooltip ("Drag, scroll or double-click to type. Alt-click resets to 0 dB.");
    qField.setTooltip ("Higher Q is narrower. Drag, scroll or double-click to type.");

    enabledToggle.onClick = [this]
    {
        const bool on = enabledToggle.getToggleState();
        edit (on ? "Turn band on" : "Turn band off", [on] (Band& band) { band.enabled = on; });
    };

    // Curve mode: both ears share a curve, and when split, each ear can be tweaked on top of it
    splitToggle.setTooltip ("Tweak each ear separately, on top of the curve both ears get. Both ears keep the same timing (matched phase), "
                            "for about 11 ms of delay while split.");
    splitToggle.onClick = [this]
    {
        const bool split = splitToggle.getToggleState();
        processor.getUndoManager().beginNewTransaction (split ? "Split ears" : "Join ears");
        processor.getSelectedProfile().setCurveSplit (split);
        if (split)
            processor.parameters.state.setProperty (CabinPeqGraph::idCurveLayer, (int) BandProfile::both, nullptr);
        updateControls();
        if (onEdited)
            onEdited();
    };
    bothLayerButton.setTooltip ("Edit the curve both ears get (B)");
    leftEarButton.setTooltip ("Tweak the left ear, on top of both (L)");
    rightEarButton.setTooltip ("Tweak the right ear, on top of both (R)");
    bothLayerButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    leftEarButton.setConnectedEdges (juce::Button::ConnectedOnRight | juce::Button::ConnectedOnLeft);
    rightEarButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    bothLayerButton.setColour (juce::TextButton::buttonOnColourId, Theme::accent);
    leftEarButton.setColour (juce::TextButton::buttonOnColourId, Theme::leftChannel);
    rightEarButton.setColour (juce::TextButton::buttonOnColourId, Theme::rightChannel);
    for (auto* button : { &bothLayerButton, &leftEarButton, &rightEarButton })
        button->setColour (juce::TextButton::textColourOnId, Theme::graph);
    bothLayerButton.onClick = [this] { setLayer (BandProfile::both); };
    leftEarButton.onClick = [this] { setLayer (BandProfile::leftTweak); };
    rightEarButton.onClick = [this] { setLayer (BandProfile::rightTweak); };

    deleteButton.setColour (juce::TextButton::textColourOffId, Theme::danger);
    deleteButton.onClick = [this] { if (onDeleteClicked) onDeleteClicked(); };

    for (auto* child : std::initializer_list<juce::Component*> { &shapeBox, &frequencyField, &gainField, &qField, &channelBox, &enabledToggle, &deleteButton,
                                                                 &splitToggle, &bothLayerButton, &leftEarButton, &rightEarButton })
        addChildComponent (child);
}

void BandInspector::showBand (int newBandId, int newBandNumber, int newNumSelected)
{
    // A value that's been typed but not entered belongs to the band it was typed for
    if (newBandId != bandId)
        for (auto* field : { &frequencyField, &gainField, &qField })
            field->commitEditing();

    bandId = newBandId;
    bandNumber = newBandNumber;
    numSelected = newNumSelected;
    updateControls();
    resized();
    repaint();
}

std::optional<Band> BandInspector::currentBand() const
{
    if (bandId < 0 || isCurveMode())
        return std::nullopt;
    return processor.getSelectedProfile().getBand (bandId);
}

bool BandInspector::isCurveMode() const
{
    return processor.getSelectedProfile().getMode() == BandProfile::Mode::curve;
}

std::optional<CurvePoint> BandInspector::currentPoint() const
{
    if (bandId < 0 || ! isCurveMode())
        return std::nullopt;
    return processor.getSelectedProfile().getPoint (bandId, layer());
}

bool BandInspector::isSplit() const
{
    return isCurveMode() && processor.getSelectedProfile().isCurveSplit();
}

int BandInspector::layer() const
{
    return isSplit() ? juce::jlimit (0, 2, (int) processor.parameters.state.getProperty (CabinPeqGraph::idCurveLayer, 0)) : (int) BandProfile::both;
}

float BandInspector::layerBaseDb (float frequency) const
{
    return layer() == BandProfile::both ? 0.0f : processor.getSelectedBandProfile().curveDbAt (frequency, -2);
}

void BandInspector::setLayer (int newLayer)
{
    processor.parameters.state.setProperty (CabinPeqGraph::idCurveLayer, newLayer, nullptr);
    updateControls();
    if (onEdited)
        onEdited();
}

juce::Rectangle<int> BandInspector::curveControlsArea() const
{
    return getLocalBounds().reduced (16, 0).removeFromLeft (isSplit() ? 104 + 52 + 2 * 36 + 20 : 104 + 20);
}

void BandInspector::editPoint (std::function<void (CurvePoint&)> change)
{
    auto point = currentPoint();
    if (! point.has_value())
        return;

    change (*point);
    processor.getSelectedProfile().updatePoint (*point, layer());
    updateControls();
    if (onEdited)
        onEdited();
}

void BandInspector::edit (const juce::String& name, std::function<void (Band&)> change)
{
    auto band = currentBand();
    if (! band.has_value())
        return;

    if (name.isNotEmpty())
        processor.getUndoManager().beginNewTransaction (name);

    change (*band);
    processor.getSelectedProfile().updateBand (*band);
    updateControls();
    if (onEdited)
        onEdited();
}

void BandInspector::updateControls()
{
    // A curve's point has just a frequency and a gain, and the curve can be split between the ears
    if (isCurveMode())
    {
        const auto point = currentPoint();
        const bool split = isSplit();
        for (auto* child : getChildren())
            child->setVisible (child == &splitToggle
                               || ((child == &bothLayerButton || child == &leftEarButton || child == &rightEarButton) && split)
                               || ((child == &frequencyField || child == &gainField || child == &deleteButton) && point.has_value()));
        splitToggle.setToggleState (split, juce::dontSendNotification);
        bothLayerButton.setToggleState (layer() == BandProfile::both, juce::dontSendNotification);
        leftEarButton.setToggleState (layer() == BandProfile::leftTweak, juce::dontSendNotification);
        rightEarButton.setToggleState (layer() == BandProfile::rightTweak, juce::dontSendNotification);
        if (point.has_value())
        {
            frequencyField.setValue (point->freq);
            gainField.setValue (point->gain + layerBaseDb (point->freq)); // the level you see: for an ear, both plus its tweak
            gainField.setEnabled (true);
            deleteButton.setButtonText (numSelected > 1 ? "Delete " + juce::String (numSelected) : "Delete");
            for (auto* field : { &frequencyField, &gainField })
                field->setAccentColour (layer() == BandProfile::leftTweak ? Theme::leftChannel
                                        : layer() == BandProfile::rightTweak ? Theme::rightChannel : Theme::accentBright);
        }
        resized();
        repaint();
        return;
    }

    auto band = currentBand();
    const bool hasBand = band.has_value();

    for (auto* child : getChildren())
        child->setVisible (hasBand && child != &splitToggle && child != &bothLayerButton && child != &leftEarButton && child != &rightEarButton);

    if (! hasBand)
        return;

    shapeBox.setSelectedId (static_cast<int> (band->shape) + 1, juce::dontSendNotification);
    channelBox.setSelectedId (static_cast<int> (band->type) + 1, juce::dontSendNotification);
    frequencyField.setValue (band->freq);
    gainField.setValue (band->ampl);
    gainField.setEnabled (band->hasGain());
    qField.setValue (band->qFactor);
    enabledToggle.setToggleState (band->enabled, juce::dontSendNotification);
    deleteButton.setButtonText (numSelected > 1 ? "Delete " + juce::String (numSelected) : "Delete");

    const auto colour = Theme::bandColour (bandNumber - 1);
    for (auto* field : { &frequencyField, &gainField, &qField })
        field->setAccentColour (colour);
}

void BandInspector::paint (juce::Graphics& g)
{
    g.fillAll (Theme::panel);
    g.setColour (Theme::border);
    g.drawHorizontalLine (0, 0.0f, (float) getWidth());

    auto area = getLocalBounds().reduced (16, 0);

    if (isCurveMode())
    {
        area.setLeft (curveControlsArea().getRight());
        const auto point = currentPoint();
        g.setColour (point.has_value() ? Theme::textDim : Theme::textFaint);
        g.setFont (Theme::font (point.has_value() ? 11.0f : 13.0f));
        if (! point.has_value())
            g.drawText (! isSplit() ? juce::String ("Drag from the 0 dB line to add a point. Drag empty space to select several and drag them together; right-click deletes.")
                        : layer() == BandProfile::both ? juce::String ("Editing the curve both ears get. L and R tweak one ear on top of it (B, L and R keys switch).")
                        : juce::String ("Tweaking the ") + (layer() == BandProfile::leftTweak ? "left" : "right")
                              + " ear on top of both. Drag from the dashed line to add a point; the other ear is faint.",
                        area, juce::Justification::centredLeft, true);
        else
            g.drawText (numSelected > 1 ? juce::String (numSelected) + " points" : "Point",
                        area.removeFromLeft (70), juce::Justification::centredLeft);
        return;
    }

    if (! currentBand().has_value())
    {
        g.setColour (Theme::textFaint);
        g.setFont (Theme::font (13.0f));
        g.drawText ("Click a band to edit it here. Shift-drag a band to change its width, and right-click to delete it. Scroll to zoom.",
                    area, juce::Justification::centredLeft, true);
        return;
    }

    // The band's number, in its colour
    auto chip = area.removeFromLeft (28).toFloat().withSizeKeepingCentre (26.0f, 26.0f);
    g.setColour (Theme::bandColour (bandNumber - 1));
    g.fillEllipse (chip);
    g.setColour (Theme::graph);
    g.setFont (Theme::font (13.0f, true));
    g.drawText (juce::String (bandNumber), chip, juce::Justification::centred);

    if (numSelected > 1)
    {
        g.setColour (Theme::textDim);
        g.setFont (Theme::font (11.0f));
        g.drawText (juce::String (numSelected) + " selected", area.removeFromLeft (80).withTrimmedLeft (8), juce::Justification::centredLeft);
    }
}

void BandInspector::resized()
{
    auto area = getLocalBounds().reduced (16, 12);
    if (isCurveMode())
    {
        auto controls = curveControlsArea().reduced (0, 12);
        splitToggle.setBounds (controls.removeFromLeft (104).withSizeKeepingCentre (104, 30));
        bothLayerButton.setBounds (controls.removeFromLeft (52).withSizeKeepingCentre (52, 28));
        leftEarButton.setBounds (controls.removeFromLeft (36).withSizeKeepingCentre (36, 28));
        rightEarButton.setBounds (controls.removeFromLeft (36).withSizeKeepingCentre (36, 28));
        area.setLeft (curveControlsArea().getRight());
        area.removeFromLeft (70);
        deleteButton.setBounds (area.removeFromRight (84).withSizeKeepingCentre (84, 30));
        frequencyField.setBounds (area.removeFromLeft (116).withSizeKeepingCentre (116, 40));
        area.removeFromLeft (8);
        gainField.setBounds (area.removeFromLeft (104).withSizeKeepingCentre (104, 40));
        return;
    }

    area.removeFromLeft (numSelected > 1 ? 36 + 80 : 36);
    deleteButton.setBounds (area.removeFromRight (84).withSizeKeepingCentre (84, 30));
    area.removeFromRight (12);

    // Squeeze the controls to fit a narrow window
    const int widths[] { 116, 104, 92, 76, 112, 64 };
    constexpr int gap = 8;
    const int needed = 564 + gap * 6;
    auto toggleArea = area.removeFromRight (64); // the switch and its label don't shrink
    enabledToggle.setBounds (toggleArea.withSizeKeepingCentre (64, 30));
    const float scale = juce::jlimit (0.6f, 1.0f, (float) area.getWidth() / (float) (needed - 64 - gap));

    int index = 0;
    for (auto* component : std::initializer_list<juce::Component*> { &shapeBox, &frequencyField, &gainField, &qField, &channelBox })
    {
        const int width = juce::roundToInt ((float) widths[index++] * scale);
        const int height = component == &frequencyField || component == &gainField || component == &qField ? 40 : 30;
        component->setBounds (area.removeFromLeft (width).withSizeKeepingCentre (width, height));
        area.removeFromLeft (juce::roundToInt (gap * scale));
    }
}
