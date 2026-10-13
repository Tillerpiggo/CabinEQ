/*
  ==============================================================================

    GuidedCalibrationView.h

    Experimental. The calibration panel's "Guided" mode: a pad to explore the plane
    GuidedSearch gives you each turn, and the buttons to answer it with. This is
    what joins the search to the app:

      the vertices   are the curve's own points, evenly spaced in octaves from
                     20 Hz to 20 kHz. A vertex's gain in each ear is its point's
                     gain, plus or minus half its difference between the ears.
      a preview      writes those points, so you hear it and see it on the graph.
                     A turn is one undo step; a choice that isn't kept is undone.
      a frame        is twelve bursts for the calibration player to go round:
                     pan from left to right, height as the grid's three rows'
                     low cuts, and level from the floor up to the volume.
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

        for (auto* button : { &firstButton, &secondButton, &thirdButton, &restartButton, &logButton })
            addAndMakeVisible (*button);
        firstButton.setColour (juce::TextButton::buttonColourId, Theme::accent.withAlpha (0.35f));
        firstButton.onClick = [this] { answer (0); };
        secondButton.onClick = [this] { answer (1); };
        thirdButton.onClick = [this] { answer (2); };
        restartButton.setTooltip ("Start the search again from the curve as it is now: 5 points, and the widest range");
        restartButton.onClick = [this] { restart(); };
        logButton.setTooltip ("Show the file that everything tried is written to");
        logButton.onClick = [this] { (logFile.existsAsFile() ? logFile : logFolder()).revealToUser(); };
        refresh();
    }

    ~GuidedCalibrationView() override { stopTimer(); }

    std::function<void()> onPlayingChanged;
    static inline bool shouldWriteLog = true; // tests turn it off, so they leave nothing behind

    /// Starts the search if there isn't one going for this profile, and plays
    void start()
    {
        if (search == nullptr || search->getPhase() == GuidedSearch::Phase::idle || profileName != processor.getProfiles().getSelectedProfileName())
            beginSearch();
        else
            playFrame (search->getFrame());

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

    void restart()
    {
        // Whatever was being tried and not kept goes
        if (search != nullptr)
            search->abandon();
        search.reset();
        start();
    }

    //==============================================================================
    static float vertexFrequency (int vertex, int count)
    {
        return 20.0f * std::pow (1000.0f, (float) vertex / (float) std::max (1, count - 1));
    }

    void paint (juce::Graphics& g) override
    {
        auto text = textArea();
        g.setColour (Theme::text);
        g.setFont (Theme::font (13.0f, true));
        g.drawFittedText (headline, text.removeFromTop (36), juce::Justification::topLeft, 2);

        g.setFont (Theme::font (12.0f));
        g.setColour (Theme::textDim);
        text.removeFromTop (2);
        g.drawFittedText (acrossText, text.removeFromTop (17), juce::Justification::centredLeft, 1);
        g.drawFittedText (upText, text.removeFromTop (17), juce::Justification::centredLeft, 1);

        g.setFont (Theme::font (11.0f));
        g.setColour (Theme::textFaint);
        text.removeFromTop (4);
        g.drawFittedText (statusText, text.removeFromTop (30), juce::Justification::topLeft, 2);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        const int side = std::min (area.getHeight() - 6, std::max (120, area.getWidth() / 3));
        pad.setBounds (area.removeFromLeft (side).withSizeKeepingCentre (side, side));

        auto buttons = textArea().removeFromBottom (28);
        logButton.setBounds (buttons.removeFromRight (40));
        buttons.removeFromRight (4);
        restartButton.setBounds (buttons.removeFromRight (62));
        buttons.removeFromRight (10);
        const int each = std::min (84, (buttons.getWidth() - 8) / 3);
        for (auto* button : { &firstButton, &secondButton, &thirdButton })
        {
            button->setBounds (buttons.removeFromLeft (each));
            buttons.removeFromLeft (4);
        }
    }

private:
    using Phase = GuidedSearch::Phase;

    juce::Rectangle<int> textArea() const
    {
        return getLocalBounds().withTrimmedLeft (pad.getRight() + 16).withTrimmedRight (4).reduced (0, 4);
    }

    //==============================================================================
    // The search's callbacks

    void beginSearch()
    {
        profileName = processor.getProfiles().getSelectedProfileName();
        startLog();

        GuidedSearch::Callbacks callbacks;
        callbacks.readVertices = [this] { return readVertices(); };
        callbacks.preview = [this] (const GuidedSearch::Vector& curve) { writeVertices (curve); };
        callbacks.commit = [this] (const GuidedSearch::Vector& curve) { writeVertices (curve); };
        callbacks.revert = [this] (const GuidedSearch::Vector&) { processor.getUndoManager().undoCurrentTransactionOnly(); };
        callbacks.refine = [this] (int count) { return makeVertices (count); };
        callbacks.playFrame = [this] (const GuidedSearch::Frame& frame) { playFrame (frame); };
        callbacks.vertexName = [] (int vertex, int count) { return Format::frequency (vertexFrequency (vertex, count)); };
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
            if (std::abs (std::log2 (points[(size_t) i].freq / vertexFrequency (i, count))) > 0.01f || points[(size_t) i].freqSplit != 0.0f)
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
                const float frequency = vertexFrequency (i, count);
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

    /// Pan from -1 to 1 is left to right; height is the grid's three rows' low cuts, and between; level is from
    /// the floor (-1) up to the volume (1)
    void playFrame (const GuidedSearch::Frame& frame)
    {
        std::vector<CalibrationPlayer::SequenceBurst> bursts;
        const float floor = player.getFloorDb();
        for (const auto& burst : frame.bursts())
            bursts.push_back ({ 0.5f * (burst.pan + 1.0f), CalibrationPlayer::cutoffForHeight (burst.height), floor * 0.5f * (1.0f - burst.level) });
        frameLength = (int) bursts.size();
        nextFrameEnd = frameLength;
        frameFloor = floor;
        player.setSequence (bursts);
    }

    void timerCallback() override
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

        // The floor slider's moved: the frame's levels are measured from it
        if (player.getFloorDb() != frameFloor)
            playFrame (search->getFrame());

        // The frame's played right through once the burst after its last has started
        if (player.getSequenceBurstsPlayed() > nextFrameEnd)
        {
            nextFrameEnd += frameLength;
            search->framePlayed();
        }
        refresh();
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
        else if (phase == Phase::finished)
            restart();
        else
            search->choose (button == 0);
        refresh();
    }

    void refresh()
    {
        const auto phase = search != nullptr ? search->getPhase() : Phase::idle;
        const bool validating = phase == Phase::validatingBefore || phase == Phase::validatingAfter;
        const bool running = search != nullptr && phase != Phase::idle && phase != Phase::finished;

        juce::String newHeadline, across, up, status;
        if (! running)
        {
            newHeadline = phase == Phase::finished ? "Finished: there's nothing left to try at the finest setting."
                                                   : "Guided calibration (experimental). Press Play to start.";
            status = "It redraws your curve through 5 points, then gives you one change at a time to try by ear.";
        }
        else
        {
            const auto& plane = search->getPlane();
            const int check = search->getValidationIndex() + 1;
            newHeadline = phase == Phase::exploring ? "Drag the dot to where it sounds best, then let go."
                        : phase == Phase::settling ? "Listening to your choice..."
                        : phase == Phase::deciding ? (search->hasMoved() ? "Is that better than where you started?"
                                                                         : "That's no change. Drag the dot, or skip this one.")
                        : phase == Phase::validatingBefore ? "Check " + juce::String (check) + " of 2. Playing: BEFORE"
                                                           : "Check " + juce::String (check) + " of 2. Playing: AFTER. How is it, against before?";
            across = "Across: " + plane.uName;
            up = "Up: " + plane.vName;

            const auto& frame = search->getFrame();
            status = "Turn " + juce::String (search->getTurnNumber())
                   + "  |  +/-" + juce::String (search->getRangeDb(), search->getRangeDb() < 10.0f ? 2 : 0) + " dB"
                   + "  |  " + juce::String (search->getVertexCount()) + " points"
                   + "  |  " + (search->isAuditing() ? juce::String ("audit") : "sweep " + juce::String (search->getSweepDone()) + " of " + juce::String (search->getSweepSize()))
                   + (plane.isAdaptive ? "  |  adaptive" : "")
                   + "  |  frame " + (frame.isAudit ? juce::String ("between") : juce::String ((int) ((frame.a + 1.0f) * 3.0f + frame.b + 2.0f)) + " of 9");
        }

        firstButton.setButtonText (phase == Phase::finished ? "Again" : "Better");
        secondButton.setButtonText (validating ? "Same" : phase == Phase::deciding && search->hasMoved() ? "No better" : "Skip");
        thirdButton.setButtonText ("Worse");
        firstButton.setVisible (running || phase == Phase::finished);
        secondButton.setVisible (running);
        thirdButton.setVisible (validating);
        firstButton.setEnabled (phase == Phase::finished || (validating ? search->canRespond() : running && search->canChoose()));
        secondButton.setEnabled (! validating || search->canRespond());
        thirdButton.setEnabled (validating && search->canRespond());
        restartButton.setVisible (search != nullptr);

        // The pad's yours while you're exploring; during the checks it shows what you chose
        pad.setEnabled (running && ! validating);
        pad.setAlpha (running && ! validating ? 1.0f : 0.5f);
        if (running)
            pad.setPosition ({ 0.5f * (search->getPad().x + 1.0f), 0.5f * (search->getPad().y + 1.0f) });

        if (newHeadline != headline || across != acrossText || up != upText || status != statusText)
        {
            headline = newHeadline;
            acrossText = across;
            upText = up;
            statusText = status;
            repaint();
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
        session->setProperty ("vertexSpacing", "even in octaves from 20 Hz to 20 kHz; gains are left ear then right, in dB");
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
    int frameLength = 12, nextFrameEnd = 12;
    float frameFloor = 0.0f; // what the playing frame's levels were worked out from

    XYPad pad;
    juce::TextButton firstButton, secondButton, thirdButton, restartButton { "Restart" }, logButton { "Log" };
    juce::String headline, acrossText, upText, statusText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GuidedCalibrationView)
};
