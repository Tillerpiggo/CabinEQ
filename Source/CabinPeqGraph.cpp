/*
  ==============================================================================

    CabinPeqGraph.cpp
    Created: 10 Oct 2024 4:30:44pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinPeqGraph.h"
#include "Format.h"

namespace
{
    const juce::Identifier idGraphRange { "graphRange" };
    const juce::Identifier idShowSpectrum { "showSpectrum" };

    // What the zoom buttons step through. Dragging or scrolling on the dB axis goes anywhere in between.
    const float displayRanges[] { 3.0f, 6.0f, 12.0f, 18.0f, 24.0f, 30.0f, 36.0f, 48.0f, 60.0f };
    constexpr float minDisplayRange = 3.0f, maxDisplayRange = 60.0f, defaultDisplayRange = 30.0f;
    constexpr float axisWidth = 44.0f; // the strip of dB labels you can drag to zoom

    // The smallest round step that gives at most six grid lines above 0 dB
    float gridStepFor (float range)
    {
        for (float step : { 1.0f, 2.0f, 3.0f, 6.0f, 12.0f })
            if (range / step <= 6.0f)
                return step;
        return 12.0f;
    }

    bool isCommandDown (const juce::ModifierKeys& mods)
    {
        return mods.isCommandDown() || mods.isCtrlDown();
    }
}

CabinPeqGraph::CabinPeqGraph (CabinEqAudioProcessor& p)
    : processor (p)
{
    setWantsKeyboardFocus (true);
    setOpaque (true);
    refresh();
    updateSpectrumTimer();
}

CabinPeqGraph::~CabinPeqGraph()
{
    processor.getAnalyzer().setActive (false);
}

//==============================================================================
void CabinPeqGraph::refresh()
{
    auto selected = processor.getSelectedProfile();
    bandProfile = selected.getBandProfile();

    // A different profile starts with nothing selected
    if (selected.getName() != profileName)
    {
        profileName = selected.getName();
        selectedIds.clear();
        focusedId = -1;
        hoverId = -1;
    }

    std::set<int> stillThere;
    for (auto id : selectedIds)
        if (bandProfile.getBandWithId (id).has_value())
            stillThere.insert (id);
    if (! bandProfile.getBandWithId (focusedId).has_value())
        focusedId = stillThere.empty() ? -1 : *stillThere.rbegin();
    selectedIds = stillThere;

    curve.setSampleRate (processor.getCurveSampleRate());
    curve.updateWithBands (bandProfile.getBands());
    curvePathsNeedRebuilding = true;
    repaint();

    if (onSelectionChanged)
        onSelectionChanged();
}

void CabinPeqGraph::setBypassed (bool shouldBeBypassed)
{
    if (isBypassed != shouldBeBypassed)
    {
        isBypassed = shouldBeBypassed;
        repaint();
    }
}

void CabinPeqGraph::focusBand (int bandId)
{
    if (bandProfile.getBandWithId (bandId).has_value())
        setSelection ({ bandId }, bandId);
}

void CabinPeqGraph::setSelection (std::set<int> ids, int newFocusedId)
{
    if (ids == selectedIds && newFocusedId == focusedId)
        return;

    selectedIds = std::move (ids);
    focusedId = newFocusedId;
    repaint();
    if (onSelectionChanged)
        onSelectionChanged();
}

void CabinPeqGraph::deleteSelectedBands()
{
    if (selectedIds.empty())
        return;

    beginEdit ("Delete bands");
    auto p = profile();
    for (auto id : selectedIds)
        p.removeBand (id);
    selectedIds.clear();
    focusedId = -1;
    refresh();
}

//==============================================================================
void CabinPeqGraph::timerCallback()
{
    if (processor.getAnalyzer().update())
        repaint (getPlotArea().toNearestInt());
}

void CabinPeqGraph::updateSpectrumTimer()
{
    const bool show = getShowSpectrum();
    processor.getAnalyzer().setActive (show);
    if (show)
        startTimerHz (30);
    else
        stopTimer();
    repaint();
}

//==============================================================================
juce::Rectangle<float> CabinPeqGraph::getPlotArea() const
{
    return getLocalBounds().toFloat().withTrimmedBottom (axisHeight);
}

float CabinPeqGraph::xForFrequency (float frequency) const
{
    auto plot = getPlotArea();
    return plot.getX() + plot.getWidth() * std::log (frequency / minFrequency) / std::log (maxFrequency / minFrequency);
}

float CabinPeqGraph::frequencyForX (float x) const
{
    auto plot = getPlotArea();
    return minFrequency * std::pow (maxFrequency / minFrequency, (x - plot.getX()) / plot.getWidth());
}

float CabinPeqGraph::yForDb (float db) const
{
    auto plot = getPlotArea().reduced (0.0f, 14.0f);
    const float range = getDisplayRange();
    return juce::jmap (db, range, -range, plot.getY(), plot.getBottom());
}

float CabinPeqGraph::dbForY (float y) const
{
    auto plot = getPlotArea().reduced (0.0f, 14.0f);
    const float range = getDisplayRange();
    return juce::jmap (y, plot.getY(), plot.getBottom(), range, -range);
}

juce::Point<float> CabinPeqGraph::handlePosition (const Band& band) const
{
    const float range = getDisplayRange();
    const float db = band.hasGain() ? band.ampl : curve.dbAtFrequencyForBand (band, band.freq);
    const float x = juce::jlimit (0.0f, (float) getWidth(), xForFrequency (band.freq));
    return { x, yForDb (juce::jlimit (-range, range, db)) };
}

//==============================================================================
std::optional<Band> CabinPeqGraph::bandAt (juce::Point<float> position) const
{
    // The topmost (last drawn) band wins, and the focused band is drawn last
    std::optional<Band> found;
    float bestDistance = handleRadius + 5.0f;
    for (const auto& band : bandProfile.getBands())
    {
        const float distance = handlePosition (band).getDistanceFrom (position);
        if (distance <= bestDistance || (band.id == focusedId && distance <= handleRadius + 5.0f))
        {
            bestDistance = distance;
            found = band;
        }
    }
    return found;
}

bool CabinPeqGraph::isNearZeroLine (juce::Point<float> position) const
{
    return getPlotArea().contains (position)
        && (int) bandProfile.getBands().size() < FilterChain::maxBands
        && std::abs (yForDb (0.0f) - position.y) < 8.0f;
}

std::vector<Band> CabinPeqGraph::getSelectedBands() const
{
    std::vector<Band> bands;
    for (const auto& band : bandProfile.getBands())
        if (selectedIds.count (band.id) > 0)
            bands.push_back (band);
    return bands;
}

int CabinPeqGraph::indexOfBand (int bandId) const
{
    const auto& bands = bandProfile.getBands();
    for (size_t i = 0; i < bands.size(); ++i)
        if (bands[i].id == bandId)
            return (int) i;
    return -1;
}

//==============================================================================
void CabinPeqGraph::beginEdit (const juce::String& name, bool coalesce)
{
    // Scrolling and nudging make lots of little edits; undo them together if they come close together
    const auto now = juce::Time::getMillisecondCounter();
    if (coalesce && name == lastCoalescedEditName && now - lastCoalescedEditTime < 800)
    {
        lastCoalescedEditTime = now;
        return;
    }

    processor.getUndoManager().beginNewTransaction (name);
    lastCoalescedEditName = coalesce ? name : juce::String();
    lastCoalescedEditTime = now;
}

int CabinPeqGraph::addBandAt (juce::Point<float> position, Band::Shape shape)
{
    Band band;
    band.shape = shape;
    band.freq = juce::jlimit (minFrequency, maxFrequency, frequencyForX (position.x));
    band.ampl = band.hasGain() ? juce::jlimit (Band::minGain, Band::maxGain, std::round (dbForY (position.y) * 10.0f) / 10.0f) : 0.0f;
    band.setQ (shape == Band::Shape::peak ? Band::defaultQ : Band::defaultCutQ);

    beginEdit ("Add band");
    const int id = profile().addBand (band);
    refresh();
    if (id >= 0)
        setSelection ({ id }, id);
    return id;
}

void CabinPeqGraph::updateBands (const std::vector<Band>& bands)
{
    auto p = profile();
    for (const auto& band : bands)
        p.updateBand (band);
    refresh();
}

void CabinPeqGraph::changeWidth (const std::vector<Band>& bands, float factor)
{
    std::vector<Band> changed;
    for (auto band : bands)
    {
        band.setQ (band.qFactor * factor);
        changed.push_back (band);
    }
    updateBands (changed);
}

void CabinPeqGraph::nudge (float octaves, float db)
{
    if (selectedIds.empty())
        return;

    beginEdit ("Nudge bands", true);
    std::vector<Band> changed;
    for (auto band : getSelectedBands())
    {
        band.freq = juce::jlimit (minFrequency, maxFrequency, band.freq * std::pow (2.0f, octaves));
        if (band.hasGain())
            band.ampl = juce::jlimit (Band::minGain, Band::maxGain, band.ampl + db);
        changed.push_back (band);
    }
    updateBands (changed);
}

//==============================================================================
float CabinPeqGraph::getDisplayRange() const
{
    return juce::jlimit (minDisplayRange, maxDisplayRange, (float) processor.parameters.state.getProperty (idGraphRange, defaultDisplayRange));
}

void CabinPeqGraph::setDisplayRange (float db)
{
    // Only what's shown: bands can still go beyond it (they sit at the edge)
    processor.parameters.state.setProperty (idGraphRange, juce::jlimit (minDisplayRange, maxDisplayRange, db), nullptr);
    curvePathsNeedRebuilding = true;
    repaint();
}

bool CabinPeqGraph::getShowSpectrum() const
{
    return processor.parameters.state.getProperty (idShowSpectrum, true);
}

void CabinPeqGraph::setShowSpectrum (bool shouldShow)
{
    processor.parameters.state.setProperty (idShowSpectrum, shouldShow, nullptr);
    updateSpectrumTimer();
}

//==============================================================================
void CabinPeqGraph::resized()
{
    curvePathsNeedRebuilding = true;
}

void CabinPeqGraph::paint (juce::Graphics& g)
{
    g.fillAll (Theme::graph);

    if (curvePathsNeedRebuilding)
        rebuildCurvePaths();

    drawGrid (g);
    if (getShowSpectrum())
        drawSpectrum (g);
    drawCurves (g);
    drawHandles (g);

    if (dragMode == DragMode::marquee)
    {
        g.setColour (Theme::accent.withAlpha (0.10f));
        g.fillRect (marquee);
        g.setColour (Theme::accent.withAlpha (0.6f));
        g.drawRect (marquee, 1.0f);
    }

    drawReadout (g);
    drawZoomControl (g);

    if (bandProfile.getBands().empty())
    {
        g.setColour (Theme::textFaint);
        g.setFont (Theme::font (13.0f));
        g.drawText ("Click the 0 dB line to add a band, then drag to shape it",
                    getPlotArea().withTrimmedTop (getPlotArea().getHeight() * 0.5f + 24.0f).withHeight (20.0f),
                    juce::Justification::centred);
    }
}

CabinPeqGraph::ZoomControl CabinPeqGraph::getZoomControl() const
{
    auto area = getPlotArea().removeFromTop (34.0f).removeFromRight (136.0f).reduced (8.0f, 6.0f);
    ZoomControl zoom;
    zoom.bounds = area;
    zoom.minus = area.removeFromLeft (26.0f);
    zoom.plus = area.removeFromRight (26.0f);
    zoom.label = area;
    return zoom;
}

void CabinPeqGraph::stepDisplayRange (int direction)
{
    // Zooming in shows fewer dB, so it steps down the list
    const float current = getDisplayRange();
    float next = current;
    if (direction < 0)
    {
        for (float range : displayRanges)
            if (range > current + 0.01f) { next = range; break; }
    }
    else
    {
        for (auto it = std::rbegin (displayRanges); it != std::rend (displayRanges); ++it)
            if (*it < current - 0.01f) { next = *it; break; }
    }
    setDisplayRange (next);
}

bool CabinPeqGraph::isOnAxis (juce::Point<float> position) const
{
    auto plot = getPlotArea();
    return position.x < plot.getX() + axisWidth && plot.contains (position);
}

void CabinPeqGraph::drawZoomControl (juce::Graphics& g)
{
    const auto zoom = getZoomControl();
    const bool hot = mouseIsOver && zoom.bounds.expanded (4.0f).contains (mousePosition);

    g.setColour (Theme::panel.withAlpha (hot ? 0.95f : 0.75f));
    g.fillRoundedRectangle (zoom.bounds, zoom.bounds.getHeight() * 0.5f);

    auto drawSign = [&] (juce::Rectangle<float> area, bool isPlus)
    {
        const bool over = mouseIsOver && area.contains (mousePosition);
        g.setColour (over ? Theme::text : Theme::textDim);
        const auto c = area.getCentre();
        g.drawLine (c.x - 4.0f, c.y, c.x + 4.0f, c.y, 1.5f);
        if (isPlus)
            g.drawLine (c.x, c.y - 4.0f, c.x, c.y + 4.0f, 1.5f);
    };
    // "+" shows more dB (zooms out), "-" fewer (zooms in)
    drawSign (zoom.minus, false);
    drawSign (zoom.plus, true);

    g.setColour (Theme::textDim);
    g.setFont (Theme::font (11.5f));
    g.drawText ("+/- " + juce::String (juce::roundToInt (getDisplayRange())) + " dB", zoom.label, juce::Justification::centred);
}

void CabinPeqGraph::drawGrid (juce::Graphics& g)
{
    auto plot = getPlotArea();

    // Frequencies: a line at every 1-9 of each decade, and labels at the usual spots
    for (float decade = 10.0f; decade <= 10000.0f; decade *= 10.0f)
    {
        for (int multiple = 1; multiple <= 9; ++multiple)
        {
            const float frequency = decade * (float) multiple;
            if (frequency < minFrequency || frequency > maxFrequency)
                continue;
            g.setColour (multiple == 1 ? Theme::gridLineMajor : Theme::gridLine);
            g.drawVerticalLine (juce::roundToInt (xForFrequency (frequency)), plot.getY(), plot.getBottom());
        }
    }

    g.setFont (Theme::font (11.0f));
    g.setColour (Theme::textFaint);
    for (float frequency : { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f })
    {
        const float x = xForFrequency (frequency);
        auto label = Format::frequencyShort (frequency);
        auto area = juce::Rectangle<float> (x - 20.0f, plot.getBottom() + 3.0f, 40.0f, axisHeight - 6.0f);
        if (frequency == minFrequency) area.setX (x + 3.0f);
        if (frequency == maxFrequency) area.setX (x - 43.0f);
        g.drawText (label, area, frequency == minFrequency ? juce::Justification::centredLeft
                                : frequency == maxFrequency ? juce::Justification::centredRight
                                                            : juce::Justification::centred);
    }

    // Gains
    const float range = getDisplayRange();
    const float step = gridStepFor (range);
    const float lowest = -std::floor (range / step + 0.001f) * step;
    for (float db = lowest; db <= range + 0.01f; db += step)
    {
        const float y = yForDb (db);
        g.setColour (std::abs (db) < 0.01f ? Theme::zeroLine : Theme::gridLine);
        g.drawHorizontalLine (juce::roundToInt (y), plot.getX(), plot.getRight());

        g.setColour (Theme::textFaint);
        auto text = std::abs (db) < 0.01f ? juce::String ("0") : (db > 0 ? "+" : "") + juce::String (juce::roundToInt (db));
        g.drawText (text, juce::Rectangle<float> (plot.getX() + 6.0f, y - 14.0f, 40.0f, 13.0f), juce::Justification::bottomLeft);
    }

    g.setColour (Theme::border);
    g.drawHorizontalLine ((int) plot.getBottom(), 0.0f, (float) getWidth());
}

void CabinPeqGraph::drawSpectrum (juce::Graphics& g)
{
    auto plot = getPlotArea();
    const auto& analyzer = processor.getAnalyzer();
    const double sampleRate = processor.getCurveSampleRate();

    // Tilt by 4.5 dB per octave around 1 kHz, so typical music looks roughly level
    constexpr float floorDb = -90.0f, ceilingDb = -12.0f;
    juce::Path path;
    path.startNewSubPath (plot.getX(), plot.getBottom());

    const float step = 2.0f;
    for (float x = plot.getX(); x <= plot.getRight(); x += step)
    {
        const float low = frequencyForX (x - step * 0.5f), high = frequencyForX (x + step * 0.5f);
        const float frequency = frequencyForX (x);
        float db = analyzer.getMaxMagnitudeDb (low, high, sampleRate);
        if (db > SpectrumAnalyzer::minDb + 1.0f) // leave silence at the floor
            db += 4.5f * std::log2 (frequency / 1000.0f);
        const float y = juce::jmap (juce::jlimit (floorDb, ceilingDb, db), floorDb, ceilingDb, plot.getBottom(), plot.getY() + plot.getHeight() * 0.1f);
        path.lineTo (x, y);
    }
    path.lineTo (plot.getRight(), plot.getBottom());
    path.closeSubPath();

    g.setGradientFill (juce::ColourGradient (Theme::spectrum.withAlpha (0.55f), 0.0f, plot.getY(),
                                             Theme::spectrum.withAlpha (0.15f), 0.0f, plot.getBottom(), false));
    g.fillPath (path);
}

juce::Path CabinPeqGraph::curvePathForChannel (int channel) const
{
    auto plot = getPlotArea();
    const float range = getDisplayRange();
    juce::Path path;
    for (float x = plot.getX(); x <= plot.getRight() + 1.0f; x += 1.5f)
    {
        const float frequency = frequencyForX (x);
        const float db = channel < 0 ? curve.dbAtFrequency (frequency) : curve.dbAtFrequencyForChannel (frequency, channel);
        const float y = yForDb (juce::jlimit (-range * 1.2f, range * 1.2f, db));
        if (x == plot.getX())
            path.startNewSubPath (x, y);
        else
            path.lineTo (x, y);
    }
    return path;
}

void CabinPeqGraph::rebuildCurvePaths()
{
    curvePathsNeedRebuilding = false;
    if (curve.hasChannelSpecificBands())
    {
        leftCurve = curvePathForChannel (0);
        rightCurve = curvePathForChannel (1);
        mainCurve.clear();
    }
    else
    {
        mainCurve = curvePathForChannel (-1);
        leftCurve.clear();
        rightCurve.clear();
    }
}

void CabinPeqGraph::drawCurves (juce::Graphics& g)
{
    auto plot = getPlotArea();
    const float zeroY = yForDb (0.0f);
    const float range = getDisplayRange();

    // The focused (or hovered) band's own shape, faintly
    const int highlighted = hoverId >= 0 ? hoverId : focusedId;
    if (auto band = bandProfile.getBandWithId (highlighted); band.has_value() && band->enabled)
    {
        juce::Path line;
        for (float x = plot.getX(); x <= plot.getRight() + 1.0f; x += 2.0f)
        {
            const float y = yForDb (juce::jlimit (-range * 1.2f, range * 1.2f, curve.dbAtFrequencyForBand (*band, frequencyForX (x))));
            if (x == plot.getX())
                line.startNewSubPath (x, y);
            else
                line.lineTo (x, y);
        }

        juce::Path shape (line);
        shape.lineTo (plot.getRight(), zeroY);
        shape.lineTo (plot.getX(), zeroY);
        shape.closeSubPath();

        auto colour = Theme::bandColour (indexOfBand (band->id));
        g.setColour (colour.withAlpha (0.16f));
        g.fillPath (shape);
    }

    const float thickness = 2.2f;
    if (! mainCurve.isEmpty())
    {
        const auto colour = isBypassed ? Theme::textFaint : Theme::accentBright;

        juce::Path fill (mainCurve);
        fill.lineTo (plot.getRight(), zeroY);
        fill.lineTo (plot.getX(), zeroY);
        fill.closeSubPath();
        g.setColour (colour.withAlpha (isBypassed ? 0.05f : 0.10f));
        g.fillPath (fill);

        g.setColour (colour);
        g.strokePath (mainCurve, juce::PathStrokeType (thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else
    {
        // Left and right differ, so draw both, with a key in the corner
        auto key = plot.withTrimmedLeft (plot.getWidth() - 120.0f).withHeight (22.0f).translated (-10.0f, 8.0f);
        g.setFont (Theme::font (11.0f));
        for (auto [path, colour, label] : { std::tuple { &leftCurve, Theme::leftChannel, "Left" }, std::tuple { &rightCurve, Theme::rightChannel, "Right" } })
        {
            const auto shown = isBypassed ? Theme::textFaint : colour;
            g.setColour (shown);
            g.strokePath (*path, juce::PathStrokeType (thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            auto entry = key.removeFromLeft (60.0f);
            g.fillRoundedRectangle (entry.removeFromLeft (12.0f).withSizeKeepingCentre (12.0f, 3.0f), 1.5f);
            g.setColour (Theme::textDim);
            g.drawText (label, entry.withTrimmedLeft (6.0f), juce::Justification::centredLeft);
        }
    }
}

void CabinPeqGraph::drawHandles (juce::Graphics& g)
{
    const auto& bands = bandProfile.getBands();

    auto drawHandle = [&] (const Band& band, int index)
    {
        const auto centre = handlePosition (band);
        const bool isSelected = selectedIds.count (band.id) > 0;
        const bool isHovered = band.id == hoverId;
        const bool isDragged = dragMode == DragMode::bands && isSelected;
        const float radius = handleRadius + (isDragged ? 1.5f : isHovered ? 1.0f : 0.0f);
        auto colour = Theme::bandColour (index);
        if (isBypassed)
            colour = colour.withSaturation (0.1f).withMultipliedBrightness (0.7f);

        auto circle = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);

        if (isSelected)
        {
            g.setColour (colour.withAlpha (0.25f));
            g.fillEllipse (circle.expanded (5.0f));
            g.setColour (Theme::text);
            g.drawEllipse (circle.expanded (2.5f), 1.5f);
        }

        if (band.enabled)
        {
            g.setColour (colour);
            g.fillEllipse (circle);
            g.setColour (Theme::graph.withAlpha (0.9f));
        }
        else
        {
            g.setColour (Theme::graph);
            g.fillEllipse (circle);
            g.setColour (colour.withAlpha (0.7f));
            g.drawEllipse (circle.reduced (0.75f), 1.5f);
        }

        g.setFont (Theme::font (10.5f, true));
        g.drawText (juce::String (index + 1), circle.translated (0.0f, -0.5f), juce::Justification::centred);

        // Which ear a one-sided band is for
        if (band.type != Band::Type::both)
        {
            auto badge = juce::Rectangle<float> (12.0f, 12.0f).withCentre (centre.translated (radius + 3.0f, -radius - 3.0f));
            g.setColour (band.type == Band::Type::left ? Theme::leftChannel : Theme::rightChannel);
            g.fillEllipse (badge);
            g.setColour (Theme::graph);
            g.setFont (Theme::font (8.5f, true));
            g.drawText (band.type == Band::Type::left ? "L" : "R", badge, juce::Justification::centred);
        }
    };

    for (size_t i = 0; i < bands.size(); ++i)
        if (bands[i].id != focusedId)
            drawHandle (bands[i], (int) i);

    const int focusedIndex = indexOfBand (focusedId);
    if (focusedIndex >= 0)
        drawHandle (bands[(size_t) focusedIndex], focusedIndex);

    // Where a click would add a band
    if (hoverId < 0 && hoverIsNearZeroLine && dragMode == DragMode::none && mouseIsOver)
    {
        const float db = 0.0f;
        auto ghost = juce::Rectangle<float> (handleRadius * 2.0f, handleRadius * 2.0f).withCentre ({ mousePosition.x, yForDb (db) });
        g.setColour (Theme::text.withAlpha (0.5f));
        g.drawEllipse (ghost, 1.2f);
        Icons::draw (g, Icons::plus(), ghost.reduced (3.0f), Theme::text.withAlpha (0.7f), 1.4f);
    }
}

void CabinPeqGraph::drawReadout (juce::Graphics& g)
{
    // A tag next to the band being hovered or dragged
    int shownId = hoverId;
    if (dragMode == DragMode::bands)
        shownId = focusedId;

    if (auto band = bandProfile.getBandWithId (shownId))
    {
        juce::String text = Band::shapeName (band->shape) + "  " + Format::frequency (band->freq);
        if (band->hasGain())
            text << "  " << Format::gain (band->ampl);
        text << "  Q " << Format::q (band->qFactor);
        if (! band->enabled)
            text << "  (off)";

        const auto font = Theme::font (12.0f);
        const float width = Theme::textWidth (font, text) + 16.0f;
        const auto centre = handlePosition (*band);
        auto tag = juce::Rectangle<float> (width, 22.0f).withCentre (centre.translated (0.0f, centre.y > getPlotArea().getCentreY() ? -30.0f : 30.0f));
        tag = tag.constrainedWithin (getPlotArea().reduced (4.0f));

        g.setColour (Theme::raised.withAlpha (0.95f));
        g.fillRoundedRectangle (tag, 4.0f);
        g.setColour (Theme::text);
        g.setFont (font);
        g.drawText (text, tag, juce::Justification::centred);
        return;
    }

    // Otherwise, the frequency under the mouse on the axis
    if (mouseIsOver && dragMode == DragMode::none && getPlotArea().contains (mousePosition))
    {
        auto plot = getPlotArea();
        auto text = Format::frequency (frequencyForX (mousePosition.x));
        const auto font = Theme::font (11.0f);
        const float width = Theme::textWidth (font, text) + 12.0f;
        auto tag = juce::Rectangle<float> (width, axisHeight - 6.0f).withCentre ({ mousePosition.x, plot.getBottom() + axisHeight * 0.5f });
        tag = tag.constrainedWithin (getLocalBounds().toFloat());

        g.setColour (Theme::raised);
        g.fillRoundedRectangle (tag, 3.0f);
        g.setColour (Theme::text);
        g.setFont (font);
        g.drawText (text, tag, juce::Justification::centred);
    }
}

//==============================================================================
void CabinPeqGraph::mouseMove (const juce::MouseEvent& event)
{
    mousePosition = event.position;
    mouseIsOver = true;

    auto band = bandAt (event.position);
    const int newHover = band.has_value() ? band->id : -1;
    const bool nearLine = ! band.has_value() && isNearZeroLine (event.position);

    if (newHover != hoverId || nearLine != hoverIsNearZeroLine)
    {
        hoverId = newHover;
        hoverIsNearZeroLine = nearLine;
    }

    setMouseCursor (hoverId >= 0 ? juce::MouseCursor::DraggingHandCursor
                    : getZoomControl().bounds.contains (event.position) ? juce::MouseCursor::PointingHandCursor
                    : isOnAxis (event.position) ? juce::MouseCursor::UpDownResizeCursor
                    : nearLine ? juce::MouseCursor::CrosshairCursor : juce::MouseCursor::NormalCursor);
    repaint();
}

void CabinPeqGraph::mouseExit (const juce::MouseEvent&)
{
    mouseIsOver = false;
    hoverId = -1;
    hoverIsNearZeroLine = false;
    repaint();
}

void CabinPeqGraph::mouseDown (const juce::MouseEvent& event)
{
    grabKeyboardFocus();
    mousePosition = event.position;

    // The second click of a double-click that just added a band keeps hold of that band
    const bool isRepeatClick = event.getNumberOfClicks() > 1;
    const bool repeatsAnAdd = isRepeatClick && bandAddedByLastClick >= 0 && bandProfile.getBandWithId (bandAddedByLastClick).has_value();
    if (! repeatsAnAdd)
        bandAddedByLastClick = -1;

    // The zoom buttons, and dragging the dB axis to zoom
    if (! event.mods.isPopupMenu())
    {
        const auto zoom = getZoomControl();
        if (zoom.minus.contains (event.position)) { stepDisplayRange (1); return; }
        if (zoom.plus.contains (event.position))  { stepDisplayRange (-1); return; }
        if (zoom.bounds.contains (event.position)) return;

        if (isOnAxis (event.position) && ! bandAt (event.position).has_value())
        {
            dragMode = DragMode::zoom;
            rangeAtDragStart = getDisplayRange();
            return;
        }
    }

    auto band = bandAt (event.position);

    if (event.mods.isPopupMenu())
    {
        if (band.has_value())
        {
            // Right-click deletes: the whole selection if the band's part of one, otherwise just it
            if (selectedIds.count (band->id) == 0)
                setSelection ({ band->id }, band->id);
            deleteSelectedBands();
            hoverId = -1;
        }
        else
        {
            showBackgroundMenu (event.position);
        }
        return;
    }

    if (repeatsAnAdd)
    {
        setSelection ({ bandAddedByLastClick }, bandAddedByLastClick);
    }
    else if (band.has_value())
    {
        if (event.mods.isShiftDown())
        {
            // Shift-drag changes the width, of the selection if the band's in it. Shift-click without
            // dragging adds the band to the selection or takes it out, which mouseUp does.
            shiftClickedId = band->id;
            dragMode = DragMode::bands;
            isChangingWidth = true;
            lastDragPosition = event.position;
            dragDistance = {};
            bandsAtDragStart = selectedIds.count (band->id) > 0 ? getSelectedBands() : std::vector<Band> { *band };
            hasBegunDragEdit = false;
            return;
        }

        if (selectedIds.count (band->id) == 0)
            setSelection ({ band->id }, band->id);
        else
            setSelection (selectedIds, band->id);
    }
    else if (hoverIsNearZeroLine || isNearZeroLine (event.position))
    {
        // Add a band on the 0 dB line, and keep dragging it
        bandAddedByLastClick = addBandAt ({ event.position.x, yForDb (0.0f) }, Band::Shape::peak);
        if (bandAddedByLastClick < 0)
            return;
    }
    else
    {
        dragMode = DragMode::marquee;
        marquee = { event.position, event.position };
        selectionBeforeMarquee = event.mods.isShiftDown() ? selectedIds : std::set<int>();
        if (! event.mods.isShiftDown())
            setSelection ({}, -1);
        return;
    }

    dragMode = DragMode::bands;
    isChangingWidth = event.mods.isAltDown();
    lastDragPosition = event.position;
    dragDistance = {};
    bandsAtDragStart = getSelectedBands();
    hasBegunDragEdit = bandAddedByLastClick >= 0; // adding a band already started its undo step
    hoverIsNearZeroLine = false;
    repaint();
}

void CabinPeqGraph::mouseDrag (const juce::MouseEvent& event)
{
    mousePosition = event.position;

    if (dragMode == DragMode::zoom)
    {
        // Drag up to zoom in (fewer dB), down to zoom out
        setDisplayRange (rangeAtDragStart * std::exp ((float) event.getDistanceFromDragStartY() / 150.0f));
        return;
    }

    if (dragMode == DragMode::marquee)
    {
        marquee = juce::Rectangle<float> (event.mouseDownPosition, event.position);
        auto ids = selectionBeforeMarquee;
        for (const auto& band : bandProfile.getBands())
            if (marquee.contains (handlePosition (band)))
                ids.insert (band.id);
        setSelection (ids, ids.empty() ? -1 : (ids.count (focusedId) > 0 ? focusedId : *ids.rbegin()));
        repaint();
        return;
    }

    if (dragMode != DragMode::bands || bandsAtDragStart.empty())
        return;

    // A shift-click only becomes a width drag once the mouse really moves
    if (shiftClickedId >= 0 && event.getDistanceFromDragStart() < 3)
        return;
    shiftClickedId = -1;

    if (! hasBegunDragEdit)
    {
        beginEdit (isChangingWidth ? "Change width" : "Move bands");
        hasBegunDragEdit = true;
    }

    // Shift (or Alt) changes the width instead, and can be pressed or let go mid-drag. Carry on
    // from wherever the bands are when it changes, so they don't jump.
    const bool wantsWidth = event.mods.isShiftDown() || event.mods.isAltDown();
    if (wantsWidth != isChangingWidth)
    {
        isChangingWidth = wantsWidth;
        std::vector<Band> current;
        for (const auto& band : bandsAtDragStart)
            if (auto now = bandProfile.getBandWithId (band.id))
                current.push_back (*now);
        bandsAtDragStart = current;
        dragDistance = {};
    }

    // Accumulate movement, so Cmd for fine control can come and go mid-drag
    const float fineness = isCommandDown (event.mods) ? 0.15f : 1.0f;
    dragDistance += (event.position - lastDragPosition) * fineness;
    lastDragPosition = event.position;

    if (isChangingWidth)
    {
        // Up makes the band wider (more bandwidth, lower Q)
        changeWidth (bandsAtDragStart, std::exp (dragDistance.y / 120.0f));
        return;
    }

    const auto distance = dragDistance;

    const auto plot = getPlotArea();
    const float octavesPerPixel = std::log2 (maxFrequency / minFrequency) / plot.getWidth();
    const float dbPerPixel = (dbForY (0.0f) - dbForY (1.0f));

    std::vector<Band> moved;
    for (auto band : bandsAtDragStart)
    {
        band.freq = juce::jlimit (minFrequency, maxFrequency, band.freq * std::pow (2.0f, distance.x * octavesPerPixel));
        if (band.hasGain())
            band.ampl = juce::jlimit (Band::minGain, Band::maxGain, band.ampl + distance.y * -dbPerPixel);
        moved.push_back (band);
    }
    updateBands (moved);
}

void CabinPeqGraph::mouseUp (const juce::MouseEvent& event)
{
    if (shiftClickedId >= 0)
    {
        auto ids = selectedIds;
        if (ids.count (shiftClickedId) > 0)
            ids.erase (shiftClickedId);
        else
            ids.insert (shiftClickedId);
        setSelection (ids, ids.count (shiftClickedId) > 0 ? shiftClickedId : (ids.empty() ? -1 : *ids.rbegin()));
        shiftClickedId = -1;
    }

    dragMode = DragMode::none;
    bandsAtDragStart.clear();
    mouseMove (event);
}

void CabinPeqGraph::mouseDoubleClick (const juce::MouseEvent& event)
{
    // Ignore it if the first click added a band; the second click is already dragging it
    if (event.mods.isPopupMenu() || bandAddedByLastClick >= 0 || getZoomControl().bounds.contains (event.position))
        return;

    if (isOnAxis (event.position) && ! bandAt (event.position).has_value())
    {
        setDisplayRange (defaultDisplayRange);
        return;
    }

    auto band = bandAt (event.position);
    if (band.has_value())
    {
        beginEdit (band->enabled ? "Turn band off" : "Turn band on");
        auto toggled = *band;
        toggled.enabled = ! toggled.enabled;
        updateBands ({ toggled });
    }
    else if (getPlotArea().contains (event.position))
    {
        dragMode = DragMode::none;
        addBandAt (event.position, Band::Shape::peak);
    }
}

void CabinPeqGraph::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    // Scrolling on the dB axis zooms
    if (isOnAxis (event.position) || getZoomControl().bounds.contains (event.position))
    {
        const float scroll = wheel.deltaY * (wheel.isReversed ? -1.0f : 1.0f);
        if (scroll != 0.0f)
            setDisplayRange (getDisplayRange() * std::exp (-scroll * 0.6f));
        return;
    }

    std::vector<Band> targets;
    if (auto band = bandAt (event.position))
        targets = selectedIds.count (band->id) > 0 ? getSelectedBands() : std::vector<Band> { *band };
    else if (! selectedIds.empty())
        targets = getSelectedBands();

    const float delta = (std::abs (wheel.deltaY) > std::abs (wheel.deltaX) ? wheel.deltaY : wheel.deltaX) * (wheel.isReversed ? -1.0f : 1.0f);
    if (targets.empty() || delta == 0.0f)
        return;

    beginEdit ("Change width", true);
    changeWidth (targets, std::exp (delta * (event.mods.isShiftDown() ? 0.15f : 0.8f)));
}

bool CabinPeqGraph::keyPressed (const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    const float fine = mods.isShiftDown() ? 4.0f : 1.0f;

    if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
    {
        deleteSelectedBands();
        return true;
    }
    if ((key.getKeyCode() == 'A' || key.getKeyCode() == 'a') && isCommandDown (mods) && ! mods.isShiftDown())
    {
        std::set<int> all;
        for (const auto& band : bandProfile.getBands())
            all.insert (band.id);
        setSelection (all, focusedId >= 0 ? focusedId : (all.empty() ? -1 : *all.begin()));
        return true;
    }
    if (key == juce::KeyPress::escapeKey && ! selectedIds.empty())
    {
        setSelection ({}, -1);
        return true;
    }
    if (! selectedIds.empty() && ! isCommandDown (mods))
    {
        if (key.getKeyCode() == juce::KeyPress::leftKey)  { nudge (-fine / 24.0f, 0.0f); return true; }
        if (key.getKeyCode() == juce::KeyPress::rightKey) { nudge (fine / 24.0f, 0.0f); return true; }
        if (key.getKeyCode() == juce::KeyPress::upKey)    { nudge (0.0f, 0.5f * fine); return true; }
        if (key.getKeyCode() == juce::KeyPress::downKey)  { nudge (0.0f, -0.5f * fine); return true; }
    }
    return false;
}

//==============================================================================
void CabinPeqGraph::showBackgroundMenu (juce::Point<float> position)
{
    juce::Component::SafePointer<CabinPeqGraph> safeThis (this);
    juce::PopupMenu menu;

    const bool canAdd = (int) bandProfile.getBands().size() < FilterChain::maxBands && getPlotArea().contains (position);
    juce::PopupMenu addMenu;
    for (auto shape : { Band::Shape::peak, Band::Shape::lowShelf, Band::Shape::highShelf, Band::Shape::lowCut, Band::Shape::highCut })
        addMenu.addItem (Band::shapeName (shape), canAdd, false, [safeThis, position, shape]
        {
            if (safeThis != nullptr)
                safeThis->addBandAt (position, shape);
        });
    menu.addSubMenu ("Add band at " + Format::frequency (frequencyForX (position.x)), addMenu, canAdd);

    menu.addSeparator();
    juce::PopupMenu rangeMenu;
    for (float range : displayRanges)
        rangeMenu.addItem ("+/- " + juce::String ((int) range) + " dB", true, std::abs (range - getDisplayRange()) < 0.1f,
                           [safeThis, range] { if (safeThis != nullptr) safeThis->setDisplayRange (range); });
    menu.addSubMenu ("Display range", rangeMenu);
    menu.addItem ("Show output spectrum", true, getShowSpectrum(),
                  [safeThis] { if (safeThis != nullptr) safeThis->setShowSpectrum (! safeThis->getShowSpectrum()); });

    menu.addSeparator();
    const bool hasBands = ! bandProfile.getBands().empty();
    menu.addItem ("Select all bands", hasBands, false, [safeThis]
    {
        if (safeThis != nullptr)
            safeThis->keyPressed (juce::KeyPress ('A', juce::ModifierKeys::commandModifier, 0));
    });
    menu.addItem (juce::PopupMenu::Item ("Delete all bands").setEnabled (hasBands).setColour (Theme::danger).setAction ([safeThis]
    {
        if (safeThis == nullptr)
            return;
        safeThis->beginEdit ("Delete all bands");
        safeThis->profile().setBands ({});
        safeThis->refresh();
    }));

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this)
                                                  .withTargetScreenArea ({ juce::Desktop::getMousePosition(), juce::Desktop::getMousePosition() }));
}
