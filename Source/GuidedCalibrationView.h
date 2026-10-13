/*
  ==============================================================================

    GuidedCalibrationView.h

    Experimental. The calibration panel's "Guided" mode: each turn, one line of
    three sounds, what to listen for in it, and a pad to explore the plane
    GuidedSearch gives you. This is what joins the search to the app:

      the vertices   are the curve's own points, from 20 Hz to 20 kHz, bunched
                     towards the top by the high-frequency bias: a vertex a
                     fraction t of the way along is at 1 - (1 - t)^bias of the
                     way up, in octaves. 1 is even; it's chosen before a search
                     starts, and stays. Doubling the vertices keeps the old ones.
                     A vertex's gain in each ear is its point's gain, plus or
                     minus half its difference between the ears.
      a preview      writes those points, so you hear it and see it on the graph.
                     A turn is one undo step; a choice that isn't kept is undone.
      a trial        is three sounds for the calibration player: A, B, C, B, each
                     repeated 2, 4 or 8 times, round and round. Pan is from left
                     to right, height is the low cut (the grid's three rows', and
                     between them), level is from the floor up to the volume.
      the markers    are three dots laid out as the line should sound, with the
                     one that's playing lit. They're the aim, not a measurement.
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

        // How many times each of the three sounds plays before the next
        repetitions = juce::jlimit (2, 8, (int) processor.parameters.state.getProperty (idRepetitions, 4));
        for (size_t i = 0; i < repetitionButtons.size(); ++i)
        {
            const int count = 2 << i; // 2, 4, 8
            auto& button = repetitionButtons[i];
            button.setButtonText ("x" + juce::String (count));
            button.setTooltip ("Each of the three sounds plays " + juce::String (count) + " times before the next");
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
            playTrial (search->getTrial());

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

    /// See what the player's up to: whether the line has played right through, and which of the three is sounding.
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

        // The floor slider's moved: the levels are measured from it
        if (player.getFloorDb() != trialFloor)
            playTrial (search->getTrial());

        // The line's played right through once the sound after its last has started
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
            playTrial (search->getTrial()); // the same sounds, more or fewer times each
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

        if (goalText.isNotEmpty())
        {
            text.removeFromTop (3);
            g.setColour (Theme::accentBright);
            g.drawFittedText (goalText, text.removeFromTop (30), juce::Justification::topLeft, 2, 0.9f);
        }
        else if (search == nullptr)
        {
            // Before a search: how much the points bunch towards the top, beside its three buttons
            g.setColour (Theme::textDim);
            g.drawText ("High-frequency bias", biasRow.withWidth (114), juce::Justification::centredLeft);
            text.removeFromTop (biasRow.getBottom() - text.getY() + 2);
        }

        text.removeFromTop (3);
        g.setColour (Theme::textDim);
        g.setFont (Theme::font (11.5f));
        g.drawFittedText (prompt, text.removeFromTop (29), juce::Justification::topLeft, 2, 0.9f);

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
        auto markers = area.removeFromLeft (104);
        auto repetitionRow = markers.removeFromBottom (22);
        const int each = repetitionRow.getWidth() / (int) repetitionButtons.size();
        for (auto& button : repetitionButtons)
            button.setBounds (repetitionRow.removeFromLeft (each));
        markerArea = markers.withTrimmedBottom (4);
        area.removeFromLeft (12);
        textBounds = area;

        // The bias, on the starting screen, sits in the text where the goal would be
        biasRow = textBounds.withTrimmedTop (18 + 3 + 3 * 15 + 6).removeFromTop (22);
        auto biasButtonsArea = biasRow.withTrimmedLeft (118);
        for (auto& button : biasButtons)
            button.setBounds (biasButtonsArea.removeFromLeft (38));

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
    using Kind = GuidedSearch::Kind;

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
        callbacks.playTrial = [this] (const GuidedSearch::Trial& trial) { playTrial (trial); };
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

    /// A, B, C, B, each as many times as is set. Pan from -1 to 1 is left to right; height is the grid's three
    /// rows' low cuts, and between; level is from the floor (-1) up to the volume (1).
    void playTrial (const GuidedSearch::Trial& trial)
    {
        std::vector<CalibrationPlayer::SequenceBurst> bursts;
        const float floor = player.getFloorDb();
        for (const auto& position : trial.bursts (repetitions))
            bursts.push_back ({ 0.5f * (position.pan + 1.0f), CalibrationPlayer::cutoffForHeight (position.height), floor * 0.5f * (1.0f - position.level) });
        sequenceLength = (int) bursts.size();
        nextSequenceEnd = sequenceLength;
        playingRepetitions = repetitions;
        trialFloor = floor;
        player.setSequence (bursts);
    }

    /// Which of the three is sounding, or -1
    int activeMarker() const
    {
        if (search == nullptr || ! player.isPlaying() || player.getMode() != CalibrationPlayer::Mode::sequence)
            return -1;
        const int burst = player.getCurrentPosition();
        return burst < 0 ? -1 : GuidedSearch::Trial::positionAtStep (burst / std::max (1, playingRepetitions));
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

    static juce::String kindName (Kind kind) { return kind == Kind::height ? "Height" : kind == Kind::width ? "Width" : "Depth"; }

    static juce::String instructionFor (Kind kind)
    {
        return kind == Kind::height ? "Make these sounds form a clearly separated vertical line. Keep them at the same depth and left/right position."
             : kind == Kind::width  ? "Make these sounds form a clearly separated horizontal line. Keep their height and depth consistent."
                                    : "Make these sounds form a clearly separated front-to-back line. Keep their height and left/right position consistent.";
    }

    void refresh()
    {
        const auto phase = search != nullptr ? search->getPhase() : Phase::idle;
        const bool validating = phase == Phase::validatingBefore || phase == Phase::validatingAfter;
        const bool running = search != nullptr && phase != Phase::idle && phase != Phase::finished;
        const bool settingUp = search == nullptr;

        juce::String newTitle, newBadge, newInstruction, newGoal, newPrompt, newAxes, newStatus;
        int lines = 3;
        if (running)
        {
            const auto& plane = search->getPlane();
            const auto kind = search->getTrial().kind;
            const auto check = "Check " + juce::String (search->getValidationIndex() + 1) + " of " + juce::String (search->getNumChecks());

            newTitle = (validating ? check + ": " : juce::String()) + kindName (kind) + " trial";
            newInstruction = instructionFor (kind);
            newGoal = search->getGoal() == GuidedSearch::Goal::straighten
                          ? "Straighten: reduce bending or sideways drift without collapsing the separation."
                          : "Expand: increase separation without introducing noticeable bending or drift.";

            if (search->isHearingStart())
            {
                newBadge = "STARTING SETTING";
                newPrompt = "This is how the turn started. Click Hear start again for your change.";
            }
            else if (phase == Phase::introducing)
            {
                newBadge = "LISTEN FIRST";
                newPrompt = "Get to know the three sounds. The pad unlocks once they've played through.";
            }
            else if (phase == Phase::exploring)
            {
                newBadge = "YOUR TURN";
                newPrompt = "Drag the dot until the line is as good as you can get it, then let go.";
            }
            else if (phase == Phase::settling)
            {
                newBadge = "SETTLING";
                newPrompt = "Listening to your choice, once through...";
            }
            else if (phase == Phase::deciding)
            {
                newBadge = search->hasMoved() ? "BETTER?" : "NO CHANGE";
                newPrompt = search->hasMoved() ? "Is that better than the starting setting, for this goal?"
                                               : "That's no change. Drag the dot, or skip this one.";
            }
            else
            {
                newBadge = phase == Phase::validatingBefore ? "BEFORE" : "AFTER";
                newPrompt = phase == Phase::validatingBefore ? "A different line, to check your change. First, the starting setting..."
                                                             : "Now with your change. For the same goal, is this line better, the same, or worse?";
            }

            newAxes = "Across: " + plane.uName + "   Up: " + plane.vName;
            newStatus = "Turn " + juce::String (search->getTurnNumber())
                      + "  |  +/-" + juce::String (search->getRangeDb(), search->getRangeDb() < 10.0f ? 2 : 0) + " dB"
                      + "  |  " + juce::String (search->getVertexCount()) + " points"
                      + "  |  pass " + juce::String (search->getPassDone()) + " of " + juce::String (search->getPassSize())
                      + (search->getCoverageLevel() > 0 ? "  |  finer heights " + juce::String (search->getCoverageLevel()) : juce::String())
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
            newInstruction = "Each turn plays a line of three sounds, tells you what to listen for, and gives you a pad that changes the curve.";
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

        // The pad's yours while you're exploring; while a line's being introduced or checked, it isn't
        const bool padIsLive = running && search->canExplore();
        pad.setEnabled (padIsLive);
        pad.setAlpha (padIsLive ? 1.0f : 0.45f);
        if (running)
            pad.setPosition ({ 0.5f * (search->getPad().x + 1.0f), 0.5f * (search->getPad().y + 1.0f) });

        const int marker = activeMarker();
        const auto kindNow = running ? (int) search->getTrial().kind : -1;
        if (newTitle != title || newBadge != badge || newInstruction != instruction || newGoal != goalText || newPrompt != prompt
            || newAxes != axesText || newStatus != statusText || lines != instructionLines || marker != shownMarker || kindNow != shownKind)
        {
            title = newTitle;
            badge = newBadge;
            instruction = newInstruction;
            goalText = newGoal;
            prompt = newPrompt;
            axesText = newAxes;
            statusText = newStatus;
            instructionLines = lines;
            shownMarker = marker;
            shownKind = kindNow;
            repaint();
        }
    }

    //==============================================================================
    // The markers: three dots laid out as the line should sound, with the one that's playing lit

    void paintMarkers (juce::Graphics& g)
    {
        const auto area = markerArea.toFloat();
        g.setColour (Theme::graph);
        g.fillRoundedRectangle (area, 6.0f);
        g.setColour (Theme::border);
        g.drawRoundedRectangle (area.reduced (0.5f), 6.0f, 1.0f);

        g.setFont (Theme::font (9.5f));
        g.setColour (Theme::textFaint);
        g.drawText ("what to aim for", area.reduced (4.0f, 3.0f), juce::Justification::centredBottom);

        if (shownKind < 0)
        {
            g.drawFittedText ("Three markers show how each line should sound", area.reduced (10.0f, 18.0f).toNearestInt(), juce::Justification::centred, 3);
            return;
        }

        // Where the three go, and what they're called: A is the low end of the line, C the high end
        const auto kind = (Kind) shownKind;
        const auto inner = area.reduced (16.0f, 16.0f).withTrimmedBottom (12.0f);
        const auto centre = inner.getCentre();
        std::array<juce::Point<float>, 3> at;
        std::array<float, 3> radius { 6.5f, 6.5f, 6.5f };
        std::array<const char*, 3> names { "", "", "" };
        if (kind == Kind::height)
        {
            at = { juce::Point<float> { inner.getX() + 14.0f, inner.getBottom() - 6.0f }, { inner.getX() + 14.0f, centre.y }, { inner.getX() + 14.0f, inner.getY() + 6.0f } };
            names = { "Low", "Middle", "High" };
        }
        else if (kind == Kind::width)
        {
            at = { juce::Point<float> { area.getX() + 18.0f, centre.y }, { centre.x, centre.y }, { area.getRight() - 18.0f, centre.y } };
            names = { "Left", "Middle", "Right" };
        }
        else
        {
            // Front to back: the far one small and up the page, the near one big and down it
            at = { juce::Point<float> { inner.getX() + 22.0f, inner.getY() + 6.0f }, { inner.getX() + 16.0f, centre.y }, { inner.getX() + 10.0f, inner.getBottom() - 8.0f } };
            radius = { 4.0f, 6.0f, 8.5f };
            names = { "Far", "Middle", "Near" };
        }

        g.setColour (Theme::zeroLine);
        g.drawLine ({ at[0], at[2] }, 1.2f);

        for (size_t i = 0; i < 3; ++i)
        {
            const bool lit = (int) i == shownMarker;
            auto dot = juce::Rectangle<float> (radius[i] * 2.0f, radius[i] * 2.0f).withCentre (at[i]);
            if (lit)
            {
                g.setColour (Theme::accent.withAlpha (0.35f));
                g.fillEllipse (dot.expanded (4.5f));
            }
            g.setColour (lit ? Theme::accentBright : Theme::raisedHover);
            g.fillEllipse (dot);

            g.setColour (lit ? Theme::text : Theme::textDim);
            g.setFont (Theme::font (10.5f, lit));
            if (kind == Kind::width) // the ends' names under them, the middle's over it, so they don't run together
                g.drawText (names[i], juce::Rectangle<float> (60.0f, 14.0f).withCentre ({ at[i].x, at[i].y + (i == 1 ? -18.0f : 18.0f) })
                                          .constrainedWithin (area.reduced (2.0f)), juce::Justification::centred);
            else
                g.drawText (names[i], juce::Rectangle<float> (at[i].x + radius[i] + 7.0f, at[i].y - 7.0f, 60.0f, 14.0f), juce::Justification::centredLeft);
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
                                       "a trial's positions are pan, height and level from -1 to 1");
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

    int repetitions = 4;       // how many times each of the three sounds plays
    float bias = 1.5f;         // how much the vertices bunch towards the top, for the next search
    float searchBias = 1.5f;   // and for the one that's going

    // What's playing
    int sequenceLength = 16, nextSequenceEnd = 16, playingRepetitions = 4;
    float trialFloor = 0.0f;   // what its levels were worked out from

    XYPad pad;
    juce::TextButton firstButton, secondButton, thirdButton, hearStartButton { "Hear start" }, restartButton { "Start over" }, logButton { "Log" };
    std::array<juce::TextButton, 3> repetitionButtons, biasButtons;
    juce::Rectangle<int> markerArea, textBounds, biasRow;
    juce::String title, badge, instruction, goalText, prompt, axesText, statusText;
    int instructionLines = 3, shownMarker = -1, shownKind = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GuidedCalibrationView)
};
