/*
  ==============================================================================

    BandInspector.cpp

  ==============================================================================
*/

#include "BandInspector.h"
#include "Format.h"

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
    frequencyField.onValueChange = [this] (double v) { edit ({}, [v] (Band& band) { band.freq = (float) v; }); };
    gainField.onValueChange = [this] (double v) { edit ({}, [v] (Band& band) { band.ampl = (float) v; }); };
    qField.onValueChange = [this] (double v) { edit ({}, [v] (Band& band) { band.setQ ((float) v); }); };

    frequencyField.setTooltip ("Drag, scroll or double-click to type. Shift for fine control.");
    gainField.setTooltip ("Drag, scroll or double-click to type. Alt-click resets to 0 dB.");
    qField.setTooltip ("Higher Q is narrower. Drag, scroll or double-click to type.");

    enabledToggle.onClick = [this]
    {
        const bool on = enabledToggle.getToggleState();
        edit (on ? "Turn band on" : "Turn band off", [on] (Band& band) { band.enabled = on; });
    };

    deleteButton.setColour (juce::TextButton::textColourOffId, Theme::danger);
    deleteButton.onClick = [this] { if (onDeleteClicked) onDeleteClicked(); };

    for (auto* child : std::initializer_list<juce::Component*> { &shapeBox, &frequencyField, &gainField, &qField, &channelBox, &enabledToggle, &deleteButton })
        addChildComponent (child);
}

void BandInspector::showBand (int newBandId, int newBandNumber, int newNumSelected)
{
    bandId = newBandId;
    bandNumber = newBandNumber;
    numSelected = newNumSelected;
    updateControls();
    resized();
    repaint();
}

std::optional<Band> BandInspector::currentBand() const
{
    if (bandId < 0)
        return std::nullopt;
    return processor.getSelectedProfile().getBand (bandId);
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
    auto band = currentBand();
    const bool hasBand = band.has_value();

    for (auto* child : getChildren())
        child->setVisible (hasBand);

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

    if (! currentBand().has_value())
    {
        g.setColour (Theme::textFaint);
        g.setFont (Theme::font (13.0f));
        g.drawText ("Click a band to edit it here. Scroll over a band to change its width, and right-click for more.",
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
