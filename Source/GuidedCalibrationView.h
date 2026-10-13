/*
  ==============================================================================

    GuidedCalibrationView.h

    Experimental. The calibration panel's "Guided" mode: each turn, a whole
    arrangement of sounds, what to listen for in it, and a pad to explore the
    plane GuidedSearch gives you. This is what joins the search to the app:

      the vertices   are the curve's own points, from 20 Hz to 20 kHz, bunched
                     towards the top by the high-frequency bias: a vertex a
                     fraction t of the way along is at 1 - (1 - t)^bias of the
                     way up, in octaves. 1 is even; it's chosen before a search
                     starts, and stays. Doubling the vertices keeps the old ones.
                     A vertex's gain in each ear is its point's gain, plus or
                     minus half its difference between the ears.
      a preview      writes those points, so you hear it and see it on the graph.
                     A turn is one undo step; a choice that isn't kept is undone.
      an arrangement is a box of sounds for the calibration player: its corners
                     (and, later, a layer between two of its sides), each played
                     2, 4 or 8 times before the next, in reading order. Pan
                     is from left to right, height is the low cut (the grid's
                     three rows', and between them), level is from the floor up
                     to the volume.
      the markers    are the box as it should sound, far smaller than near, with
                     the sound that's playing lit and named. They're the aim, not
                     a measurement.
      the log        is a line of JSON for everything tried, in
                     ~/Library/Application Support/CabinEQ/Guided calibration

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqAudioProcessor.h"
#include "CurveShapePanel.h"
#include "GuidedSearch.h"
#include "Format.h"

class GuidedCalibrationView : public juce::Component,
                              private juce::Timer
{
public:
    explicit GuidedCalibrationView (CabinEqAudioProcessor& p)
        : processor (p), player (p.getCalibration())
    {
        pad.neutral = { 0.5f, 0.5f };
        pad.setTooltip ("Drag to try a change to the curve; the middle is no change. Let go to hear it settle.");
        pad.onChange = [this] (juce::Point<float> at)
        {
            if (search != nullptr)
                search->padMoved ({ at.x * 2.0f - 1.0f, at.y * 2.0f - 1.0f });
            refresh();
        };
        pad.onGestureEnd = [this]
        {
            if (search != nullptr)
                search->padReleased();
            refresh();
        };
        pad.onReset = [this]
        {
            if (search != nullptr)
            {
                search->padMoved ({});
                search->padReleased();
            }
            refresh();
        };
        addAndMakeVisible (pad);

        for (auto* button : { &firstButton, &secondButton, &thirdButton, &hearStartButton, &restartButton, &logButton })
            addAndMakeVisible (*button);
        firstButton.setColour (juce::TextButton::buttonColourId, Theme::accent.withAlpha (0.35f));
        firstButton.onClick = [this] { answer (0); };
        secondButton.onClick = [this] { answer (1); };
        thirdButton.onClick = [this] { answer (2); };

        hearStartButton.setClickingTogglesState (true);
        hearStartButton.setTooltip ("Hear the setting this turn started from, without losing where the dot is. Click again for your change.");
        hearStartButton.onClick = [this]
        {
            if (search != nullptr)
                search->setHearingStart (hearStartButton.getToggleState());
            refresh();
        };

        restartButton.setTooltip ("Stop this search. You can then choose the high-frequency bias again, and Play starts a new one from the curve as it is.");
        restartButton.onClick = [this] { startOver(); };
        logButton.setTooltip ("Show the file that everything tried is written to");
        logButton.onClick = [this] { (logFile.existsAsFile() ? logFile : logFolder()).revealToUser(); };

        // How many times each sound plays before the next
        repetitions = juce::jlimit (2, 8, (int) processor.parameters.state.getProperty (idRepetitions, 4));
        for (size_t i = 0; i < repetitionButtons.size(); ++i)
        {
            const int count = 2 << i; // 2, 4, 8
            auto& button = repetitionButtons[i];
            button.setButtonText ("x" + juce::String (count));
            button.setTooltip ("Each sound plays " + juce::String (count) + " times before the next");
            button.setConnectedEdges ((i > 0 ? juce::Button::ConnectedOnLeft : 0) | (i + 1 < repetitionButtons.size() ? juce::Button::ConnectedOnRight : 0));
            button.onClick = [this, count] { setRepetitions (count); };
            addAndMakeVisible (button);
        }

        // How much the vertices bunch towards the top: chosen before a search starts
        bias = juce::jlimit (1.0f, 2.0f, (float) (double) processor.parameters.state.getProperty (idBias, 1.5));
        const float biases[] { 1.0f, 1.5f, 2.0f };
        const char* biasNames[] { "1", "1.5", "2" };
        const char* biasTips[] { "Even: the points are evenly spaced in octaves", "More of the points towards the top", "Most of the points towards the top" };
        for (size_t i = 0; i < biasButtons.size(); ++i)
        {
            auto& button = biasButtons[i];
            button.setButtonText (biasNames[i]);
            button.setTooltip (biasTips[i]);
            button.setConnectedEdges ((i > 0 ? juce::Button::ConnectedOnLeft : 0) | (i + 1 < biasButtons.size() ? juce::Button::ConnectedOnRight : 0));
            button.onClick = [this, value = biases[i]] { setBias (value); };
            addAndMakeVisible (button);
        }
        refresh();
    }

    ~GuidedCalibrationView() override { stopTimer(); }

    std::function<void()> onPlayingChanged;
    static inline bool shouldWriteLog = true; // tests turn it off, so they leave nothing behind

    /// Starts a search if there isn't one going for this profile, and plays
    void start()
    {
        if (search == nullptr || search->getPhase() == GuidedSearch::Phase::idle || profileName != processor.getProfiles().getSelectedProfileName())
            beginSearch();
        else
            playArrangement (search->getArrangement());

        player.setMode (CalibrationPlayer::Mode::sequence);
        player.setPlaying (true);
        startTimerHz (30);
        refresh();
        if (onPlayingChanged)
            onPlayingChanged();
    }

    void pause()
    {
        stopTimer();
        if (player.isPlaying() && player.getMode() == CalibrationPlayer::Mode::sequence)
            player.setPlaying (false);
        refresh();
        if (onPlayingChanged)
            onPlayingChanged();
    }

    /// For leaving partway through a turn (another mode, or the panel's hidden): what was being tried isn't
    /// kept, so the curve goes back to how the turn found it. The search itself carries on from there next time.
    void leave()
    {
        if (search != nullptr)
            search->resetTurn();
        pause();
    }

    /// Ends the search, dropping whatever was being tried. What it's already kept stays in the curve.
    void startOver()
    {
        if (search != nullptr)
            search->abandon();
        search.reset();
        pause();
    }

    /// See what the player's up to: whether the arrangement has played right through, and which sound it's on.
    /// (The timer does this thirty times a second.)
    void poll()
    {
        if (search == nullptr)
            return;

        // A different profile: this search was the other one's
        if (profileName != processor.getProfiles().getSelectedProfileName())
        {
            search.reset();
            pause();
            return;
        }

        // The floor slider's moved, or the heights are spaced or pinned another way: the levels and the low cuts come from those
        if (player.getFloorDb() != playingFloor || player.getHeights() != playingSpacing)
            playArrangement (search->getArrangement());

        // It's played right through once the sound after its last has started
        if (player.getSequenceBurstsPlayed() > nextSequenceEnd)
        {
            nextSequenceEnd += sequenceLength;
            search->framePlayed();
        }
        refresh();
    }

    //==============================================================================
    // The choices that aren't the search's own

    int getRepetitions() const { return repetitions; }
    void setRepetitions (int count)
    {
        repetitions = count <= 2 ? 2 : count <= 4 ? 4 : 8;
        processor.parameters.state.setProperty (idRepetitions, repetitions, nullptr);
        if (search != nullptr && isRunning())
            playArrangement (search->getArrangement()); // the same sounds, more or fewer times each
        refresh();
    }

    /// How much the vertices bunch towards the top. A search keeps the one it started with.
    float getBias() const { return bias; }
    void setBias (float newBias)
    {
        bias = juce::jlimit (1.0f, 2.0f, newBias);
        processor.parameters.state.setProperty (idBias, bias, nullptr);
        refresh();
        repaint();
    }

    /// A fraction t of the way along the vertices is 1 - (1 - t)^bias of the way from 20 Hz to 20 kHz, in octaves
    static float vertexFrequency (int vertex, int count, float highFrequencyBias)
    {
        const float t = (float) vertex / (float) std::max (1, count - 1);
        const float warped = 1.0f - std::pow (1.0f - t, highFrequencyBias);
        return 20.0f * std::pow (1000.0f, warped);
    }

    const GuidedSearch* getSearch() const { return search.get(); }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        paintMarkers (g);

        auto text = textArea();
        auto titleRow = text.removeFromTop (18);
        g.setFont (Theme::font (11.0f, true));
        g.setColour (Theme::accentBright);
        const int badgeWidth = juce::roundToInt (Theme::textWidth (Theme::font (11.0f, true), badge)) + 4;
        g.drawText (badge, titleRow.removeFromRight (badgeWidth), juce::Justification::centredRight);
        g.setColour (Theme::text);
        g.setFont (Theme::font (13.0f, true));
        g.drawFittedText (title, titleRow, juce::Justification::centredLeft, 1);

        text.removeFromTop (3);
        g.setFont (Theme::font (12.0f));
        g.setColour (Theme::text);
        g.drawFittedText (instruction, text.removeFromTop (instructionLines * 15), juce::Justification::topLeft, instructionLines, 0.9f);

        if (search == nullptr)
        {
            // Before a search: how much the points bunch towards the top, beside its three buttons
            g.setColour (Theme::textDim);
            g.drawText ("High-frequency bias", biasRow.withWidth (114), juce::Justification::centredLeft);
            text.removeFromTop (biasRow.getBottom() - text.getY() + 2);
        }

        text.removeFromTop (3);
        g.setColour (Theme::textDim);
        g.setFont (Theme::font (11.5f));
        g.drawFittedText (prompt, text.removeFromTop (promptLines * 14 + 1), juce::Justification::topLeft, promptLines, 0.9f);

        g.setFont (Theme::font (10.5f));
        g.setColour (Theme::textFaint);
        text.removeFromTop (2);
        if (axesText.isNotEmpty())
            g.drawFittedText (axesText, text.removeFromTop (13), juce::Justification::centredLeft, 1, 0.8f);
        g.drawFittedText (statusText, text.removeFromTop (13), juce::Justification::centredLeft, 1, 0.8f);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto buttons = area.removeFromBottom (28);
        area.removeFromBottom (6);

        const int side = juce::jmin (area.getHeight(), 178, std::max (110, area.getWidth() / 3));
        pad.setBounds (area.removeFromLeft (side).withSizeKeepingCentre (side, side));
        area.removeFromLeft (10);

        // The markers, with how many times each plays under them
        auto markers = area.removeFromLeft (118);
        auto repetitionRow = markers.removeFromBottom (22);
        const int each = repetitionRow.getWidth() / (int) repetitionButtons.size();
        for (auto& button : repetitionButtons)
            button.setBounds (repetitionRow.removeFromLeft (each));
        markerArea = markers.withTrimmedBottom (4);
        area.removeFromLeft (12);
        textBounds = area;

        // The bias, on the starting screen, sits in the text under what it's all about
        biasRow = textBounds.withTrimmedTop (18 + 3 + 3 * 15 + 6).removeFromTop (22);
        auto biasButtonsArea = biasRow.withTrimmedLeft (116);
        for (auto& button : biasButtons)
            button.setBounds (biasButtonsArea.removeFromLeft (32));

        logButton.setBounds (buttons.removeFromRight (40));
        buttons.removeFromRight (4);
        restartButton.setBounds (buttons.removeFromRight (78));
        buttons.removeFromRight (10);
        for (auto* button : { &firstButton, &secondButton, &thirdButton })
        {
            button->setBounds (buttons.removeFromLeft (72));
            buttons.removeFromLeft (4);
        }
        buttons.removeFromLeft (6);
        hearStartButton.setBounds (buttons.removeFromLeft (std::min (92, std::max (0, buttons.getWidth()))));
    }

private:
    using Phase = GuidedSearch::Phase;
    using Dimension = GuidedSearch::Dimension;
    using Position = GuidedSearch::Position;

    static inline const juce::Identifier idRepetitions { "guidedRepetitions" }, idBias { "guidedHighBias" }; // on the state root, not undoable

    juce::Rectangle<int> textArea() const { return textBounds; }
    bool isRunning() const { return isTimerRunning(); }
    void timerCallback() override { poll(); }

    //==============================================================================
    // The search's callbacks

    void beginSearch()
    {
        profileName = processor.getProfiles().getSelectedProfileName();
        searchBias = bias; // this search's, from here on
        startLog();

        GuidedSearch::Callbacks callbacks;
        callbacks.readVertices = [this] { return readVertices(); };
        callbacks.preview = [this] (const GuidedSearch::Vector& curve) { writeVertices (curve); };
        callbacks.commit = [this] (const GuidedSearch::Vector& curve) { writeVertices (curve); };
        callbacks.revert = [this] (const GuidedSearch::Vector&) { processor.getUndoManager().undoCurrentTransactionOnly(); };
        callbacks.refine = [this] (int count) { return makeVertices (count); };
        callbacks.playArrangement = [this] (const GuidedSearch::Arrangement& arrangement) { playArrangement (arrangement); };
        callbacks.vertexName = [this] (int vertex, int count) { return Format::frequency (vertexFrequency (vertex, count, searchBias)); };
        callbacks.turnStarted = [this] { processor.getUndoManager().beginNewTransaction ("Guided calibration"); };
        callbacks.log = [this] (const juce::var& line) { writeLog (line); };

        search = std::make_unique<GuidedSearch> (GuidedSearch::Config(), std::move (callbacks));
        search->begin();
    }

    /// The curve's points, lowest first, if they're the vertices already
    std::vector<CurvePoint> vertexPoints (int count) const
    {
        auto points = processor.getSelectedProfile().getBandProfile().getPoints();
        std::sort (points.begin(), points.end(), [] (const CurvePoint& a, const CurvePoint& b) { return a.freq < b.freq; });
        if ((int) points.size() != count)
            return {};
        for (int i = 0; i < count; ++i)
            if (std::abs (std::log2 (points[(size_t) i].freq / vertexFrequency (i, count, searchBias))) > 0.01f || points[(size_t) i].freqSplit != 0.0f)
                return {};
        return points;
    }

    /// Redraws the curve through this many vertices, each ear's as close as they come to how it was
    GuidedSearch::Vector makeVertices (int count)
    {
        auto profile = processor.getSelectedProfile();
        if (vertexPoints (count).empty())
        {
            const auto before = profile.getBandProfile();
            std::vector<CurvePoint> points;
            for (int i = 0; i < count; ++i)
            {
                const float frequency = vertexFrequency (i, count, searchBias);
                const float left = before.sharedDbAt (frequency, 0), right = before.sharedDbAt (frequency, 1);
                points.push_back ({ i, frequency, 0.5f * (left + right), 0.0f, left - right });
            }
            processor.getUndoManager().beginNewTransaction ("Guided calibration: " + juce::String (count) + " points");
            profile.setPoints (points);
        }
        if (! profile.getCurveOn())
            profile.setCurveOn (true);
        processor.setEditingCurve (true);
        return readVertices (count);
    }

    GuidedSearch::Vector readVertices() { return readVertices (search != nullptr && search->getVertexCount() > 0 ? search->getVertexCount() : 5); }

    GuidedSearch::Vector readVertices (int count)
    {
        auto points = vertexPoints (count);
        if (points.empty())
        {
            makeVertices (count); // something else has changed the points: back to vertices
            points = vertexPoints (count);
        }
        GuidedSearch::Vector curve ((size_t) count * 2, 0.0f);
        for (size_t i = 0; i < points.size() && i < (size_t) count; ++i)
        {
            curve[i] = points[i].gain + 0.5f * points[i].gainSplit;
            curve[(size_t) count + i] = points[i].gain - 0.5f * points[i].gainSplit;
        }
        return curve;
    }

    void writeVertices (const GuidedSearch::Vector& curve)
    {
        const int count = (int) curve.size() / 2;
        auto points = vertexPoints (count);
        if (points.empty())
            return;
        auto profile = processor.getSelectedProfile();
        for (size_t i = 0; i < points.size(); ++i)
        {
            const float left = curve[i], right = curve[(size_t) count + i];
            points[i].gain = juce::jlimit (CurvePoint::minGain, CurvePoint::maxGain, 0.5f * (left + right));
            points[i].gainSplit = juce::jlimit (-CurvePoint::maxGainSplit, CurvePoint::maxGainSplit, left - right);
            profile.updatePoint (points[i]);
        }
    }

    /// Each sound as many times as is set, then the next. Pan from -1 to 1 is left to right; height is the
    /// grid's three rows' low cuts, and between; level is from the floor (-1) up to the volume (1).
    void playArrangement (const GuidedSearch::Arrangement& arrangement)
    {
        std::vector<CalibrationPlayer::SequenceBurst> bursts;
        const float floor = player.getFloorDb();
        const auto spacing = player.getHeights();
        for (const auto& position : arrangement.bursts (repetitions))
            bursts.push_back ({ 0.5f * (position.pan + 1.0f), CalibrationPlayer::cutoffForHeight (position.height, spacing), floor * 0.5f * (1.0f - position.level) });
        sequenceLength = (int) bursts.size();
        nextSequenceEnd = sequenceLength;
        playingRepetitions = repetitions;
        playingFloor = floor;
        playingSpacing = spacing;
        player.setSequence (bursts);
    }

    /// Which of the arrangement's sounds is playing, or -1
    int activeMarker() const
    {
        if (search == nullptr || ! player.isPlaying() || player.getMode() != CalibrationPlayer::Mode::sequence)
            return -1;
        const int burst = player.getCurrentPosition();
        return burst < 0 ? -1 : (burst / std::max (1, playingRepetitions)) % std::max (1, search->getArrangement().size());
    }

    //==============================================================================
    // What you answer with

    void answer (int button)
    {
        if (search == nullptr)
            return;
        const auto phase = search->getPhase();
        if (phase == Phase::validatingBefore || phase == Phase::validatingAfter)
            search->respond (button == 0 ? GuidedSearch::Response::better : button == 1 ? GuidedSearch::Response::same : GuidedSearch::Response::worse);
        else if (phase != Phase::finished)
            search->choose (button == 0);
        refresh();
    }

    /// The two sides a layer's between
    static juce::String sidesOf (Dimension dimension)
    {
        return dimension == Dimension::width ? "left and right" : dimension == Dimension::height ? "low and high" : "far and near";
    }

    static juce::String positionName (const Position& position)
    {
        auto name = [] (float at, const char* low, const char* between, const char* high) { return at <= -0.99f ? low : at >= 0.99f ? high : between; };
        return juce::String (name (position.height, "Low", "Mid", "High")) + ", " + name (position.pan, "left", "centre", "right")
               + ", " + name (position.level, "far", "halfway", "near");
    }

    void refresh()
    {
        const auto phase = search != nullptr ? search->getPhase() : Phase::idle;
        const bool validating = phase == Phase::validatingBefore || phase == Phase::validatingAfter;
        const bool running = search != nullptr && phase != Phase::idle && phase != Phase::finished;
        const bool settingUp = search == nullptr;

        juce::String newTitle, newBadge, newInstruction, newPrompt, newAxes, newStatus;
        int lines = 3, newPromptLines = 2;
        if (running)
        {
            const auto& plane = search->getPlane();
            const auto& arrangement = search->getArrangement();
            const auto middle = arrangement.middleDimension();
            const bool isOrderCheck = search->getValidationIndex() == 0;

            newTitle = validating ? "Check " + juce::String (search->getValidationIndex() + 1) + " of " + juce::String (search->getNumChecks())
                                        + (isOrderCheck ? ": another order" : ": more positions")
                                  : juce::String ("The whole arrangement");
            newInstruction = "Make the whole arrangement larger and more consistent. Listen for compression, collapse, or distortion anywhere in the arrangement.";
            lines = 4;
            newPromptLines = 3;

            if (search->isHearingStart())
            {
                newBadge = "STARTING SETTING";
                newPrompt = "This is how the turn started. Click Hear start again for your change.";
            }
            else if (phase == Phase::introducing)
            {
                newBadge = "LISTEN FIRST";
                newPrompt = (middle.has_value() ? "Twelve sounds now: the box, and a layer between " + sidesOf (*middle) + ". "
                                                : juce::String ("The corners of a box: its near side read like a page, then its far side. "))
                          + "The pad unlocks once they've played through.";
            }
            else if (phase == Phase::exploring)
            {
                newBadge = "YOUR TURN";
                newPrompt = "Drag the dot until the whole arrangement is as good as you can get it, then let go.";
            }
            else if (phase == Phase::settling)
            {
                newBadge = "SETTLING";
                newPrompt = "Listening to your choice, once through...";
            }
            else if (phase == Phase::deciding)
            {
                newBadge = search->hasMoved() ? "BETTER?" : "NO CHANGE";
                newPrompt = search->hasMoved() ? "Is the whole arrangement better than with the starting setting?"
                                               : "That's no change. Drag the dot, or skip this one.";
            }
            else if (phase == Phase::validatingBefore)
            {
                newBadge = "BEFORE";
                newPrompt = (isOrderCheck || ! middle.has_value() ? juce::String ("The same sounds, down each side's columns this time, to check your change. ")
                                                                  : "Now with a layer between " + sidesOf (*middle) + ", to check your change. ")
                          + "First, the starting setting...";
            }
            else
            {
                newBadge = "AFTER";
                newPrompt = "Now with your change. Is the whole arrangement better, the same, or worse?";
            }

            newAxes = "Across: " + plane.uName + "   Up: " + plane.vName;
            newStatus = "Turn " + juce::String (search->getTurnNumber())
                      + "  |  +/-" + juce::String (search->getRangeDb(), search->getRangeDb() < 10.0f ? 2 : 0) + " dB"
                      + "  |  " + juce::String (search->getVertexCount()) + " points"
                      + "  |  pass " + juce::String (search->getPassDone()) + " of " + juce::String (search->getPassSize())
                      + "  |  " + juce::String (arrangement.size()) + " sounds"
                      + (plane.isAdaptive ? "  |  adaptive" : "");
        }
        else if (phase == Phase::finished)
        {
            newTitle = "Finished";
            newInstruction = "There's nothing left to try at the finest setting. What was kept is in your curve.";
        }
        else
        {
            newTitle = "Guided calibration (experimental)";
            newInstruction = "Each turn plays a box of sounds (left and right, low and high, far and near) and gives you a pad that changes the curve.";
            juce::StringArray frequencies;
            for (int i = 0; i < 5; ++i)
                frequencies.add (Format::frequencyShort (vertexFrequency (i, 5, bias)));
            newPrompt = "5 points to start: " + frequencies.joinIntoString (", ") + " Hz. Play redraws your curve through them and begins.";
        }

        firstButton.setButtonText ("Better");
        secondButton.setButtonText (validating ? "Same" : phase == Phase::deciding && search->hasMoved() ? "No better" : "Skip");
        thirdButton.setButtonText ("Worse");
        firstButton.setVisible (running);
        secondButton.setVisible (running);
        thirdButton.setVisible (validating);
        firstButton.setEnabled (running && (validating ? search->canRespond() : search->canChoose()));
        secondButton.setEnabled (! validating || search->canRespond());
        thirdButton.setEnabled (validating && search->canRespond());
        hearStartButton.setVisible (running);
        hearStartButton.setEnabled (running && search->canExplore());
        hearStartButton.setToggleState (running && search->isHearingStart(), juce::dontSendNotification);
        restartButton.setVisible (! settingUp);
        logButton.setVisible (! settingUp);

        for (size_t i = 0; i < repetitionButtons.size(); ++i)
            repetitionButtons[i].setToggleState (repetitions == (2 << i), juce::dontSendNotification);
        const float biases[] { 1.0f, 1.5f, 2.0f };
        for (size_t i = 0; i < biasButtons.size(); ++i)
        {
            biasButtons[i].setVisible (settingUp);
            biasButtons[i].setToggleState (std::abs (bias - biases[i]) < 0.01f, juce::dontSendNotification);
        }

        // The pad's yours while you're exploring; while an arrangement's being introduced or checked, it isn't
        const bool padIsLive = running && search->canExplore();
        pad.setEnabled (padIsLive);
        pad.setAlpha (padIsLive ? 1.0f : 0.45f);
        if (running)
            pad.setPosition ({ 0.5f * (search->getPad().x + 1.0f), 0.5f * (search->getPad().y + 1.0f) });

        const int marker = activeMarker();
        const auto positionsNow = running ? search->getArrangement().positions() : std::vector<Position>();
        if (newTitle != title || newBadge != badge || newInstruction != instruction || newPrompt != prompt || newAxes != axesText
            || newStatus != statusText || lines != instructionLines || newPromptLines != promptLines || marker != shownMarker || positionsNow != shownPositions)
        {
            title = newTitle;
            badge = newBadge;
            instruction = newInstruction;
            prompt = newPrompt;
            axesText = newAxes;
            statusText = newStatus;
            instructionLines = lines;
            promptLines = newPromptLines;
            shownMarker = marker;
            shownPositions = positionsNow;
            repaint();
        }
    }

    //==============================================================================
    // The markers: the box as it should sound, with the sound that's playing lit and named

    void paintMarkers (juce::Graphics& g)
    {
        const auto area = markerArea.toFloat();
        g.setColour (Theme::graph);
        g.fillRoundedRectangle (area, 6.0f);
        g.setColour (Theme::border);
        g.drawRoundedRectangle (area.reduced (0.5f), 6.0f, 1.0f);

        const bool isLit = shownMarker >= 0 && shownMarker < (int) shownPositions.size();
        g.setFont (Theme::font (isLit ? 10.5f : 9.5f));
        g.setColour (isLit ? Theme::text : Theme::textFaint);
        g.drawText (isLit ? positionName (shownPositions[(size_t) shownMarker]) : juce::String ("what to aim for"),
                    area.reduced (4.0f, 3.0f), juce::Justification::centredBottom);

        if (shownPositions.empty())
        {
            g.setFont (Theme::font (9.5f));
            g.setColour (Theme::textFaint);
            g.drawFittedText ("The dots show where each sound should seem to be", area.reduced (10.0f, 18.0f).toNearestInt(), juce::Justification::centred, 3);
            return;
        }

        // The box from in front and a little to one side: what's far is smaller, and up to the right
        const auto inner = area.reduced (6.0f).withTrimmedBottom (14.0f);
        const float side = std::min (inner.getWidth(), inner.getHeight());
        const auto box = juce::Rectangle<float> (side, side).withCentre (inner.getCentre());
        const float nearHalf = side * 0.29f, farHalf = side * 0.18f, inset = 9.0f;
        const juce::Point<float> nearCentre { box.getX() + nearHalf + inset, box.getBottom() - nearHalf - inset };
        const juce::Point<float> farCentre { box.getRight() - farHalf - inset, box.getY() + farHalf + inset };
        auto nearness = [] (const Position& position) { return 0.5f * (position.level + 1.0f); };
        auto place = [&] (const Position& position)
        {
            const auto centre = farCentre + (nearCentre - farCentre) * nearness (position);
            const float half = farHalf + (nearHalf - farHalf) * nearness (position);
            return juce::Point<float> { centre.x + position.pan * half, centre.y - position.height * half };
        };

        // Its twelve edges: each corner to the ones a step along each dimension
        auto corner = [] (int index) { return Position { (index & 1) != 0 ? 1.0f : -1.0f, (index & 2) != 0 ? 1.0f : -1.0f, (index & 4) != 0 ? 1.0f : -1.0f }; };
        g.setColour (Theme::zeroLine);
        for (int index = 0; index < 8; ++index)
            for (int bit = 1; bit < 8; bit <<= 1)
                if ((index & bit) == 0)
                    g.drawLine ({ place (corner (index)), place (corner (index | bit)) }, 1.0f);

        // The far ones first, so the near ones are in front; the one that's playing last of all
        std::vector<int> drawOrder (shownPositions.size());
        std::iota (drawOrder.begin(), drawOrder.end(), 0);
        std::stable_sort (drawOrder.begin(), drawOrder.end(), [this] (int a, int b)
        {
            const bool aLit = a == shownMarker, bLit = b == shownMarker;
            return aLit != bLit ? bLit : shownPositions[(size_t) a].level < shownPositions[(size_t) b].level;
        });
        for (int index : drawOrder)
        {
            const auto& position = shownPositions[(size_t) index];
            const float radius = 3.2f + 2.8f * nearness (position);
            const auto dot = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (place (position));
            const bool lit = index == shownMarker;
            if (lit)
            {
                g.setColour (Theme::accent.withAlpha (0.35f));
                g.fillEllipse (dot.expanded (4.5f));
            }
            g.setColour (lit ? Theme::accentBright : Theme::raisedHover);
            g.fillEllipse (dot);
        }
    }

    //==============================================================================
    // The log

    static juce::File logFolder()
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
#if JUCE_MAC
                   .getChildFile ("Application Support")
#endif
                   .getChildFile ("CabinEQ").getChildFile ("Guided calibration");
    }

    void startLog()
    {
        logFile = logFolder().getChildFile ("Guided " + juce::Time::getCurrentTime().formatted ("%Y-%m-%d %H.%M.%S") + ".jsonl");
        auto* session = new juce::DynamicObject();
        session->setProperty ("event", "session");
        session->setProperty ("profile", profileName);
        session->setProperty ("time", juce::Time::getCurrentTime().toISO8601 (true));
        session->setProperty ("highFrequencyBias", searchBias);
        session->setProperty ("repetitions", repetitions);
        session->setProperty ("notes", "gains are left ear then right, in dB; a vertex t of the way along is 1 - (1 - t)^bias of the way from 20 Hz to 20 kHz in octaves; "
                                       "an arrangement's positions are pan, height and level from -1 to 1, in the order they're played");
        writeLog (juce::var (session));
    }

    void writeLog (const juce::var& line)
    {
        if (! shouldWriteLog || logFile == juce::File() || ! logFile.getParentDirectory().createDirectory())
            return;
        logFile.appendText (juce::JSON::toString (line, true) + "\n");
    }

    //==============================================================================
    CabinEqAudioProcessor& processor;
    CalibrationPlayer& player;
    std::unique_ptr<GuidedSearch> search;
    juce::String profileName;
    juce::File logFile;

    int repetitions = 4;       // how many times each sound plays before the next
    float bias = 1.5f;         // how much the vertices bunch towards the top, for the next search
    float searchBias = 1.5f;   // and for the one that's going

    // What's playing
    int sequenceLength = 32, nextSequenceEnd = 32, playingRepetitions = 4;
    float playingFloor = 0.0f;   // what its levels were worked out from
    CalibrationPlayer::Heights playingSpacing; // and its low cuts

    XYPad pad;
    juce::TextButton firstButton, secondButton, thirdButton, hearStartButton { "Hear start" }, restartButton { "Start over" }, logButton { "Log" };
    std::array<juce::TextButton, 3> repetitionButtons, biasButtons;
    juce::Rectangle<int> markerArea, textBounds, biasRow;
    juce::String title, badge, instruction, prompt, axesText, statusText;
    int instructionLines = 3, promptLines = 2, shownMarker = -1;
    std::vector<Position> shownPositions; // the arrangement that's showing, in the order it plays

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GuidedCalibrationView)
};
