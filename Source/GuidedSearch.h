/*
  ==============================================================================

    GuidedSearch.h

    Experimental: a guided way to calibrate. Each turn it gives you a plane to
    explore with a pad, plays one line of three sounds while you do, and keeps what
    you choose only if it holds up against the curve you started the turn from on
    the other two kinds of line.

    The curve is a vector: each vertex's gain in the left ear, then each one's in
    the right. A plane is two directions in that vector, u and v, and the pad at
    (x, y), each from -1 to 1, previews

        start + range * (x * u + y * v)

    always from the turn's frozen starting curve. Directions have a largest entry
    of 1, so the range is how far one axis moves its biggest vertex.

    Structured planes, built from each vertex's "both ears" (C) and "left up,
    right down" (D) directions, from the highest vertex to the lowest in each group:

        (C i, C i+1)   neighbouring regions together
        (C i, D i)     a region's level, and its difference between the ears
        (D i, D i+1)   neighbouring ear differences

    A turn is one trial: a line of three sounds, A, B and C, played A, B, C, B
    round and round. A height trial's three differ in height (their low cut), a
    width trial's in pan, a depth trial's in level; the other two are held. The
    first time a kind of trial comes up, it plays once through before the pad
    does anything, so you know what you're listening to. Each turn has a goal,
    which the whole pass shares: to straighten the line, or to spread it out.

    After a choice that holds up:
        if it touched the pad's edge, the same plane and trial again, recentred;
        otherwise an adaptive plane: across is that movement (continue it or undo
        it), up is the least-used direction that isn't nearly parallel to it,
        with its part along the movement taken out.
    After one that doesn't, or no improvement: the next structured plane.
    After two adaptive turns running: one structured turn.

    A pass is every structured plane on every kind of trial; adaptive turns are
    extra. Passes come in pairs, one to straighten and one to spread. After a pair
    with nothing kept, the height trials get finer: their three heights were the
    whole span, then each half of it is added, then each quarter (and the heights
    held in width and depth trials come from the same set). Only when they're as
    fine as they go does the range halve; when it's under the smallest range, the
    vertices double (5, 9, 17), keeping the curve.

    This class is only the controller. It knows nothing of audio or the profile:
    it's given callbacks to read the vertices, preview and commit a curve, play a
    trial, and refine the vertices, and it's told what happens (the pad moved, the
    line finished playing, what you answered).

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <numeric>
#include <set>

class GuidedSearch
{
public:
    using Vector = std::vector<float>;

    enum class Kind { height = 0, width = 1, depth = 2 };
    enum class Goal { straighten = 0, expand = 1 };

    //==============================================================================
    /// A line of three sounds, each a pan, a height and a level from -1 to 1. One of the three differs along the
    /// line; the other two are held.
    struct Trial
    {
        struct Position
        {
            float pan = 0.0f, height = 0.0f, level = 0.0f;
            bool operator== (const Position& other) const { return pan == other.pan && height == other.height && level == other.level; }
        };

        Kind kind = Kind::height;
        std::array<Position, 3> positions {}; // A, the low end; B, the middle; C, the high end

        /// Which of the three plays at each of the four steps: A, B, C, B
        static constexpr int numSteps = 4;
        static int positionAtStep (int step) { return step % numSteps == 3 ? 1 : step % numSteps; }

        /// A, B, C, B, each this many times
        std::vector<Position> bursts (int repetitions) const
        {
            std::vector<Position> all;
            for (int step = 0; step < numSteps; ++step)
                for (int repeat = 0; repeat < repetitions; ++repeat)
                    all.push_back (positions[(size_t) positionAtStep (step)]);
            return all;
        }

        bool operator== (const Trial& other) const { return kind == other.kind && positions == other.positions; }
    };

    struct Plane
    {
        Vector u, v;
        juce::String uName, vName;
        bool isAdaptive = false;
        int structuredIndex = -1; // which of the structured planes, if it's one
    };

    struct Config
    {
        float startRangeDb = 30.0f;
        float minRangeDb = 3.75f;         // ranges go 30, 15, 7.5, 3.75; under this, the vertices are refined instead
        float rangeAfterRefiningDb = 15.0f;
        std::vector<int> vertexCounts { 5, 9, 17 };
        float edge = 0.98f;               // a choice this far out on either axis touched the pad's edge
        float nearlyParallel = 0.95f;     // cosine similarity above this is no use as a second axis
        int adaptiveTurnsBeforeStructured = 2;

        /// The heights a height trial's three sounds are at, each level of coverage adding finer ones to the last
        std::vector<std::vector<std::array<float, 3>>> heightTriples {
            { { -1.0f, 0.0f, 1.0f } },
            { { -1.0f, -0.5f, 0.0f }, { 0.0f, 0.5f, 1.0f } },
            { { -1.0f, -0.75f, -0.5f }, { -0.5f, -0.25f, 0.0f }, { 0.0f, 0.25f, 0.5f }, { 0.5f, 0.75f, 1.0f } }
        };
    };

    struct Callbacks
    {
        std::function<Vector()> readVertices;                    // left gains, then right gains, as the curve is now
        std::function<void (const Vector&)> preview;             // hear this curve; it's not kept yet
        std::function<void (const Vector&)> commit;              // keep this one
        std::function<void (const Vector&)> revert;              // back to this one, the turn's start
        std::function<Vector (int vertexCount)> refine;          // make the curve this many vertices, keeping its shape
        std::function<void (const Trial&)> playTrial;            // from its first sound, round and round
        std::function<juce::String (int vertex, int vertexCount)> vertexName; // "632 Hz", for naming directions
        std::function<void()> turnStarted;
        std::function<void (const juce::var&)> log;
    };

    /// introducing: a kind of trial's first time, playing once through before the pad does anything
    enum class Phase { idle, introducing, exploring, settling, deciding, validatingBefore, validatingAfter, finished };
    enum class Response { better, same, worse };

    //==============================================================================
    GuidedSearch (Config configToUse, Callbacks callbacksToUse)
        : config (std::move (configToUse)), callbacks (std::move (callbacksToUse))
    {
        // Where the held pan and level come from, turn by turn: the middle first, then round the corners and edges
        spots = { { 0.0f, 0.0f }, { -1.0f, -1.0f }, { 1.0f, 1.0f }, { -1.0f, 1.0f }, { 1.0f, -1.0f },
                  { 0.0f, -1.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f }, { 1.0f, 0.0f } };
    }

    /// Starts from the curve as it is, at the coarsest vertices, the widest range and the plainest trials
    void begin()
    {
        levelIndex = 0;
        range = config.startRangeDb;
        directions.clear();
        turnNumber = 0;
        introduced = {};
        goal = Goal::straighten;
        keptInRound = false;
        planeCursor = kindCursor = spotCursor = tripleCursor = heldHeightCursor = 0;
        callbacks.refine (config.vertexCounts[0]); // the curve as it is, through the coarsest vertices
        setVertexCount (config.vertexCounts[0], {});
        setCoverageLevel (0);
        log ("begin", {});
        startPass();
        startStructuredTurn();
    }

    //==============================================================================
    // What's happening, for whatever's showing it
    Phase getPhase() const               { return phase; }
    const Plane& getPlane() const        { return plane; }
    const Trial& getTrial() const        { return playingTrial; } // what's playing: the turn's own, or a check's
    const Trial& getTurnTrial() const    { return turnTrial; }
    Goal getGoal() const                 { return turnGoal; }     // the same through a turn's checks
    float getRangeDb() const             { return range; }
    int getVertexCount() const           { return numVertices; }
    juce::Point<float> getPad() const    { return pad; }
    int getTurnNumber() const            { return turnNumber; }
    int getValidationIndex() const       { return validationIndex; }
    int getNumChecks() const             { return 2; }
    int getNumStructuredPlanes() const   { return (int) structuredPlanes.size(); }
    int getPassSize() const              { return (int) structuredPlanes.size() * 3; }
    int getPassDone() const              { return passDone; }
    int getCoverageLevel() const         { return coverageLevel; }
    const std::vector<std::array<float, 3>>& getHeightTriples() const { return activeTriples; }
    const std::vector<float>& getHeldHeights() const { return heldHeights; }
    const Vector& getStart() const       { return start; }
    Vector getCandidate() const          { return previewAt (pad); }
    bool hasMoved() const                { return std::abs (pad.x) > 1.0e-4f || std::abs (pad.y) > 1.0e-4f; }
    bool isHearingStart() const          { return hearingStart; }

    /// Whether the pad does anything: not while a trial's being introduced, or a choice is being checked
    bool canExplore() const              { return phase == Phase::exploring || phase == Phase::settling || phase == Phase::deciding; }
    /// Whether you can say yet that the choice is better: it's had its one complete time through, and it's a change
    bool canChoose() const               { return phase == Phase::deciding && hasMoved() && ! hearingStart; }
    /// Whether you can answer yet: the changed curve has at least started playing against the starting one
    bool canRespond() const              { return phase == Phase::validatingAfter || (phase == Phase::validatingBefore && heardAfter); }

    Vector previewAt (juce::Point<float> at) const
    {
        Vector curve = start;
        for (size_t i = 0; i < curve.size(); ++i)
            curve[i] += range * (at.x * plane.u[i] + at.y * plane.v[i]);
        return curve;
    }

    //==============================================================================
    // What happens

    /// The pad's being dragged: x and y from -1 to 1. Grabbing it again after letting go carries on the same turn.
    void padMoved (juce::Point<float> at)
    {
        if (! canExplore())
            return;
        if (phase != Phase::exploring)
        {
            phase = Phase::exploring;
            play (turnTrial);
        }
        hearingStart = false; // dragging is for hearing the change
        pad = { juce::jlimit (-1.0f, 1.0f, at.x), juce::jlimit (-1.0f, 1.0f, at.y) };
        callbacks.preview (previewAt (pad));
    }

    /// Let go: the line plays once right through with the choice, standing still, before it can be called better
    void padReleased()
    {
        if (phase != Phase::exploring)
            return;
        phase = Phase::settling;
        play (turnTrial);
    }

    /// Hear the curve the turn started from, without losing where the pad is. Turning it off again gives the
    /// choice its whole time through once more.
    void setHearingStart (bool shouldHearStart)
    {
        if (! canExplore() || hearingStart == shouldHearStart)
            return;
        hearingStart = shouldHearStart;
        callbacks.preview (hearingStart ? start : previewAt (pad));
        if (! hearingStart && phase != Phase::exploring)
        {
            phase = Phase::settling;
            play (turnTrial);
        }
    }

    /// The line that's playing has played right through
    void framePlayed()
    {
        if (phase == Phase::introducing)
        {
            introduced[(size_t) turnTrial.kind] = true;
            phase = Phase::exploring;
        }
        else if (phase == Phase::settling)
        {
            if (! hearingStart)
                phase = Phase::deciding;
        }
        else if (phase == Phase::validatingBefore)
        {
            introduced[(size_t) playingTrial.kind] = true; // heard with the curve standing still
            phase = Phase::validatingAfter;
            callbacks.preview (candidate);
            play (playingTrial);
        }
        else if (phase == Phase::validatingAfter)
        {
            heardAfter = true;
            phase = Phase::validatingBefore;
            callbacks.preview (start);
            play (playingTrial);
        }
    }

    /// "It's better" goes on to check it on the other two kinds of trial; "it's not" (or skipping the turn) drops
    /// it and moves to the next structured plane
    void choose (bool isBetter)
    {
        if (phase != Phase::introducing && ! canExplore())
            return;
        if (! isBetter)
        {
            finishTurn (false);
            return;
        }
        if (! canChoose())
            return;

        candidate = previewAt (pad);
        responses.clear();
        validationIndex = 0;

        // The other two kinds, each its own trial, with the same sounds for its before and its after
        validationTrials.clear();
        for (int step = 1; step <= 2; ++step)
            validationTrials.push_back (makeTrial ((Kind) (((int) turnTrial.kind + step) % 3)));
        startValidation();
    }

    /// Against the starting curve on this trial, the change is...
    void respond (Response response)
    {
        if (! canRespond())
            return;
        responses.push_back (response);
        if (response == Response::worse)
        {
            finishTurn (false); // kept only if it's worse on neither
        }
        else if (++validationIndex < (int) validationTrials.size())
        {
            startValidation();
        }
        else
        {
            finishTurn (true);
        }
    }

    /// Stops where it is, dropping whatever was being tried and hadn't been kept
    void abandon()
    {
        if (phase != Phase::idle && phase != Phase::finished)
            callbacks.revert (start);
        phase = Phase::idle;
    }

    /// Puts the turn back to how it started: whatever was being tried is dropped, and the pad's in the middle again.
    /// For when you leave partway through one.
    void resetTurn()
    {
        if (phase == Phase::idle || phase == Phase::finished)
            return;
        callbacks.revert (start);
        beginTurn();
    }

    //==============================================================================
    // For tests, and for anyone curious
    struct Direction
    {
        Vector vector;
        juce::String name;
        int uses = 0;
        int order = -1; // where it first came up in the queue of structured planes; later ones follow on
    };
    const std::vector<Direction>& getDirections() const { return directions; }
    const std::vector<Plane>& getStructuredPlanes() const { return structuredPlanes; }

    /// The least-used direction that isn't nearly parallel to `along`. Of those used equally little, the one that
    /// came first in the original queue. -1 if there isn't one.
    static int leastUsedIndependent (const Vector& along, const std::vector<Direction>& pool, float nearlyParallel)
    {
        int best = -1;
        for (int i = 0; i < (int) pool.size(); ++i)
        {
            const auto& candidateDirection = pool[(size_t) i];
            if (std::abs (cosine (along, candidateDirection.vector)) > nearlyParallel)
                continue;
            if (best < 0 || candidateDirection.uses < pool[(size_t) best].uses
                || (candidateDirection.uses == pool[(size_t) best].uses && candidateDirection.order < pool[(size_t) best].order))
                best = i;
        }
        return best;
    }

    /// A direction with its part along another taken out, and its largest entry made 1. Empty if nothing's left of it.
    static Vector apartFrom (const Vector& direction, const Vector& along)
    {
        Vector apart = direction;
        const float alongSquared = dot (along, along);
        const float part = alongSquared > 0.0f ? dot (along, direction) / alongSquared : 0.0f;
        for (size_t i = 0; i < apart.size(); ++i)
            apart[i] -= along[i] * part;
        const float size = largest (apart);
        if (size < 1.0e-5f)
            return {};
        for (auto& value : apart)
            value /= size;
        return apart;
    }

    static float dot (const Vector& a, const Vector& b)
    {
        float sum = 0.0f;
        for (size_t i = 0; i < a.size(); ++i)
            sum += a[i] * b[i];
        return sum;
    }

    static float largest (const Vector& a)
    {
        float most = 0.0f;
        for (float value : a)
            most = std::max (most, std::abs (value));
        return most;
    }

    static float cosine (const Vector& a, const Vector& b)
    {
        const float lengths = std::sqrt (dot (a, a) * dot (b, b));
        return lengths > 0.0f ? dot (a, b) / lengths : 0.0f;
    }

private:
    //==============================================================================
    // Vertices, directions and planes

    /// Sets up the directions and structured planes for this many vertices. Directions from coarser vertices are
    /// kept, as the coordinated changes to the finer ones that they now are.
    void setVertexCount (int count, const std::vector<Direction>& earlier)
    {
        const int before = numVertices;
        numVertices = count;
        const auto n = (size_t) count;

        directions.clear();
        auto name = [this, count] (int vertex) { return callbacks.vertexName ? callbacks.vertexName (vertex, count) : "vertex " + juce::String (vertex + 1); };

        // Each vertex in both ears (C), then each as left up and right down (D)
        for (int i = 0; i < count; ++i)
        {
            Vector both (2 * n, 0.0f);
            both[(size_t) i] = both[n + (size_t) i] = 1.0f;
            directions.push_back ({ both, "both ears at " + name (i) });
        }
        for (int i = 0; i < count; ++i)
        {
            Vector apart (2 * n, 0.0f);
            apart[(size_t) i] = 1.0f;
            apart[n + (size_t) i] = -1.0f;
            directions.push_back ({ apart, "left over right at " + name (i) });
        }

        // The planes, from the highest vertex down in each group
        structuredPlanes.clear();
        planeDirections.clear();
        auto addPlane = [this] (int uIndex, int vIndex)
        {
            Plane p;
            p.u = directions[(size_t) uIndex].vector;
            p.v = directions[(size_t) vIndex].vector;
            p.uName = directions[(size_t) uIndex].name;
            p.vName = directions[(size_t) vIndex].name;
            p.structuredIndex = (int) structuredPlanes.size();
            structuredPlanes.push_back (p);
            planeDirections.push_back ({ uIndex, vIndex });
        };
        for (int i = count - 2; i >= 0; --i) addPlane (i, i + 1);                 // (C i, C i+1)
        for (int i = count - 1; i >= 0; --i) addPlane (i, count + i);             // (C i, D i)
        for (int i = count - 2; i >= 0; --i) addPlane (count + i, count + i + 1); // (D i, D i+1)

        // The order the directions first come up in that queue: it settles which of two equally used ones is next
        nextDirectionOrder = 0;
        for (const auto& [first, second] : planeDirections)
            for (int index : { first, second })
                if (directions[(size_t) index].order < 0)
                    directions[(size_t) index].order = nextDirectionOrder++;

        // Earlier, broader directions: each new vertex between two old ones takes their average
        for (const auto& old : earlier)
        {
            if (before <= 1 || (int) old.vector.size() != 2 * before)
                continue;
            Vector finer (2 * n, 0.0f);
            for (int ear = 0; ear < 2; ++ear)
                for (int i = 0; i < count; ++i)
                {
                    const float at = (float) i * (float) (before - 1) / (float) (count - 1);
                    const int below = std::min ((int) at, before - 2);
                    const float along = at - (float) below;
                    const size_t from = (size_t) (ear * before + below);
                    finer[(size_t) ear * n + (size_t) i] = old.vector[from] * (1.0f - along) + old.vector[from + 1] * along;
                }
            addDirection (finer, old.name, old.uses);
        }
        planeCursor = 0;
    }

    /// Adds a direction unless there's one nearly parallel to it already, which is counted as used instead
    void addDirection (const Vector& vector, const juce::String& name, int uses)
    {
        for (auto& existing : directions)
        {
            if (std::abs (cosine (existing.vector, vector)) > config.nearlyParallel)
            {
                existing.uses = std::max (existing.uses, uses);
                return;
            }
        }
        directions.push_back ({ vector, name, uses, nextDirectionOrder++ });
    }

    /// Across: the movement that held up. Up: the least-used direction that isn't nearly parallel to it, with
    /// its part along the movement taken out. Nothing if there's no such direction.
    std::optional<Plane> adaptivePlaneFrom (const Vector& movement)
    {
        const float size = largest (movement);
        if (size < 1.0e-5f)
            return std::nullopt;

        Plane p;
        p.isAdaptive = true;
        p.u = movement;
        for (auto& value : p.u)
            value /= size;
        p.uName = "your last change (left undoes it)";

        const int index = leastUsedIndependent (p.u, directions, config.nearlyParallel);
        if (index < 0)
            return std::nullopt;
        auto& other = directions[(size_t) index];
        p.v = apartFrom (other.vector, p.u);
        if (p.v.empty())
            return std::nullopt;
        p.vName = other.name + ", apart from that change";
        ++other.uses;
        addDirection (p.u, "an earlier change", 1); // a movement that held up is a direction worth having
        return p;
    }

    //==============================================================================
    // Trials

    /// The height trials' heights up to this level of coverage, coarsest first, so the whole span stays in turn
    /// with the finer ones; and every height among them, for width and depth trials to hold
    void setCoverageLevel (int level)
    {
        coverageLevel = juce::jlimit (0, (int) config.heightTriples.size() - 1, level);
        activeTriples.clear();
        std::set<float> heights;
        for (int i = 0; i <= coverageLevel; ++i)
            for (const auto& triple : config.heightTriples[(size_t) i])
            {
                activeTriples.push_back (triple);
                heights.insert (triple.begin(), triple.end());
            }
        heldHeights.assign (heights.begin(), heights.end());
    }

    /// The next trial of a kind: its line across the whole span (or, for heights, the next three heights in
    /// turn), with what it holds still taken from the next spot and the next height in turn
    Trial makeTrial (Kind kind)
    {
        const auto spot = spots[(size_t) spotCursor];
        spotCursor = (spotCursor + 1) % (int) spots.size();
        const std::array<float, 3> span { -1.0f, 0.0f, 1.0f };

        Trial trial;
        trial.kind = kind;
        if (kind == Kind::height)
        {
            const auto heights = activeTriples[(size_t) tripleCursor % activeTriples.size()];
            ++tripleCursor;
            for (size_t i = 0; i < 3; ++i)
                trial.positions[i] = { spot.first, heights[i], spot.second };
        }
        else
        {
            const float height = heldHeights[(size_t) heldHeightCursor % heldHeights.size()];
            ++heldHeightCursor;
            for (size_t i = 0; i < 3; ++i)
                trial.positions[i] = kind == Kind::width ? Trial::Position { span[i], height, spot.second }
                                                         : Trial::Position { spot.first, height, span[i] };
        }
        return trial;
    }

    //==============================================================================
    // Turns

    void startPass()
    {
        covered.assign (structuredPlanes.size() * 3, false);
        passDone = 0;
        keptInPass = false;
    }

    /// The next structured plane, on a kind of trial it's not had this pass
    void startStructuredTurn()
    {
        if (passDone >= getPassSize())
            endPass();
        if (phase == Phase::finished)
            return;

        adaptiveRun = 0;
        const auto planeIndex = (size_t) planeCursor;
        planeCursor = (planeCursor + 1) % (int) structuredPlanes.size();
        ++directions[(size_t) planeDirections[planeIndex].first].uses;
        ++directions[(size_t) planeDirections[planeIndex].second].uses;

        // The first kind from where we are that this plane hasn't had
        Kind kind = Kind::height;
        for (int step = 0; step < 3; ++step)
        {
            const int candidateKind = (kindCursor + step) % 3;
            if (! covered[planeIndex * 3 + (size_t) candidateKind])
            {
                covered[planeIndex * 3 + (size_t) candidateKind] = true;
                ++passDone;
                kind = (Kind) candidateKind;
                kindCursor = (candidateKind + 1) % 3;
                break;
            }
        }
        startTurn (structuredPlanes[planeIndex], makeTrial (kind));
    }

    void startTurn (const Plane& planeToExplore, const Trial& trial)
    {
        ++turnNumber;
        plane = planeToExplore;
        turnTrial = trial;
        turnGoal = goal;
        start = callbacks.readVertices();
        start.resize ((size_t) numVertices * 2, 0.0f);
        beginTurn();
    }

    /// The turn from its beginning: the pad in the middle, and if this kind of trial is new, once through first
    void beginTurn()
    {
        pad = {};
        heardAfter = false;
        hearingStart = false;
        responses.clear();
        validationTrials.clear();
        validationIndex = 0;
        phase = introduced[(size_t) turnTrial.kind] ? Phase::exploring : Phase::introducing;
        if (callbacks.turnStarted)
            callbacks.turnStarted();
        play (turnTrial);
    }

    void play (const Trial& trial)
    {
        playingTrial = trial;
        callbacks.playTrial (trial);
    }

    void startValidation()
    {
        heardAfter = false;
        phase = Phase::validatingBefore;
        callbacks.preview (start);
        play (validationTrials[(size_t) validationIndex]);
    }

    void finishTurn (bool kept)
    {
        const auto finalPad = pad;
        const Vector chosen = previewAt (finalPad);
        logTurn (kept, chosen);

        Vector movement;
        if (kept)
        {
            callbacks.commit (chosen);
            keptInPass = true;

            // What actually changed: the curve may not have been able to go all the way there
            movement = callbacks.readVertices();
            movement.resize (start.size(), 0.0f);
            for (size_t i = 0; i < movement.size(); ++i)
                movement[i] -= start[i];
        }
        else
        {
            callbacks.revert (start);
        }

        // After two adaptive turns running, the next is a structured one, whatever happened
        const bool adaptiveAllowed = adaptiveRun < config.adaptiveTurnsBeforeStructured;
        const bool touchedEdge = std::max (std::abs (finalPad.x), std::abs (finalPad.y)) >= config.edge;

        if (kept && touchedEdge && (! plane.isAdaptive || adaptiveAllowed))
        {
            // There may be further to go: the same plane and the same sounds, at the same range, from where it's got to
            if (plane.isAdaptive)
                ++adaptiveRun;
            startTurn (plane, turnTrial);
            return;
        }

        if (kept && adaptiveAllowed)
        {
            if (auto adaptive = adaptivePlaneFrom (movement))
            {
                ++adaptiveRun;
                startTurn (*adaptive, makeTrial (turnTrial.kind)); // the kind that showed the improvement
                return;
            }
        }
        startStructuredTurn();
    }

    //==============================================================================
    // Passes, coverage, ranges and refining

    /// Passes come in pairs: one to straighten the lines, then one to spread them. A pair with nothing kept
    /// means there's nothing more to find like this.
    void endPass()
    {
        log ("pass", { { "kept", keptInPass }, { "goal", goal == Goal::straighten ? "straighten" : "expand" } });
        keptInRound = keptInRound || keptInPass;

        if (goal == Goal::straighten)
        {
            goal = Goal::expand;
        }
        else
        {
            goal = Goal::straighten;
            if (! keptInRound)
                lookCloserOrNarrow();
            keptInRound = false;
        }
        if (phase != Phase::finished)
            startPass();
    }

    /// Finer height trials first. Only when those are as fine as they go, a smaller range; and under the smallest
    /// range, more vertices.
    void lookCloserOrNarrow()
    {
        if (coverageLevel + 1 < (int) config.heightTriples.size())
        {
            setCoverageLevel (coverageLevel + 1);
            log ("coverage", { { "level", coverageLevel } });
            return;
        }

        range *= 0.5f;
        if (range >= config.minRangeDb - 1.0e-4f)
        {
            log ("range", {});
            return;
        }

        // That was the smallest range: more vertices, if there are more to have
        if (levelIndex + 1 >= (int) config.vertexCounts.size())
        {
            phase = Phase::finished;
            log ("finished", {});
            return;
        }
        ++levelIndex;
        const auto earlier = directions;
        const int count = config.vertexCounts[(size_t) levelIndex];
        callbacks.refine (count);
        setVertexCount (count, earlier);
        range = config.rangeAfterRefiningDb;
        log ("refined", { { "vertices", count } });
    }

    //==============================================================================
    // The log: enough to see afterwards what was tried, and what was answered

    static juce::var toVar (const Vector& vector)
    {
        juce::Array<juce::var> values;
        for (float value : vector)
            values.add (std::round (value * 1000.0f) / 1000.0f);
        return values;
    }

    static juce::var toVar (const Trial& trial)
    {
        juce::Array<juce::var> positions;
        for (const auto& position : trial.positions)
        {
            auto* object = new juce::DynamicObject();
            object->setProperty ("pan", position.pan);
            object->setProperty ("height", position.height);
            object->setProperty ("level", position.level);
            positions.add (juce::var (object));
        }
        auto* object = new juce::DynamicObject();
        object->setProperty ("kind", trial.kind == Kind::height ? "height" : trial.kind == Kind::width ? "width" : "depth");
        object->setProperty ("positions", positions);
        return juce::var (object);
    }

    void log (const juce::String& event, std::initializer_list<std::pair<const char*, juce::var>> more)
    {
        if (! callbacks.log)
            return;
        auto* object = new juce::DynamicObject();
        object->setProperty ("event", event);
        object->setProperty ("turn", turnNumber);
        object->setProperty ("vertices", numVertices);
        object->setProperty ("rangeDb", range);
        object->setProperty ("coverage", coverageLevel);
        for (const auto& [key, value] : more)
            object->setProperty (key, value);
        callbacks.log (juce::var (object));
    }

    void logTurn (bool kept, const Vector& chosen)
    {
        if (! callbacks.log)
            return;
        juce::Array<juce::var> answered, checks;
        for (auto response : responses)
            answered.add (response == Response::better ? "better" : response == Response::same ? "same" : "worse");
        for (const auto& trial : validationTrials)
            checks.add (toVar (trial));

        auto* object = new juce::DynamicObject();
        object->setProperty ("event", "turn");
        object->setProperty ("turn", turnNumber);
        object->setProperty ("vertices", numVertices);
        object->setProperty ("rangeDb", range);
        object->setProperty ("coverage", coverageLevel);
        object->setProperty ("goal", turnGoal == Goal::straighten ? "straighten" : "expand");
        object->setProperty ("adaptive", plane.isAdaptive);
        object->setProperty ("structuredPlane", plane.structuredIndex);
        object->setProperty ("uName", plane.uName);
        object->setProperty ("vName", plane.vName);
        object->setProperty ("u", toVar (plane.u));
        object->setProperty ("v", toVar (plane.v));
        object->setProperty ("trial", toVar (turnTrial));
        object->setProperty ("start", toVar (start));
        object->setProperty ("x", pad.x);
        object->setProperty ("y", pad.y);
        object->setProperty ("chosen", toVar (chosen));
        object->setProperty ("checks", checks);
        object->setProperty ("validation", answered);
        object->setProperty ("kept", kept);
        callbacks.log (juce::var (object));
    }

    //==============================================================================
    Config config;
    Callbacks callbacks;

    std::vector<Direction> directions;
    std::vector<Plane> structuredPlanes;
    std::vector<std::pair<int, int>> planeDirections; // each structured plane's two directions

    int numVertices = 0, levelIndex = 0;
    float range = 30.0f;

    // Trials: what's held still comes round in turn
    std::vector<std::pair<float, float>> spots;        // a held pan and a held level
    std::vector<std::array<float, 3>> activeTriples;   // height trials' three heights
    std::vector<float> heldHeights;                    // the heights width and depth trials hold
    int coverageLevel = 0;
    int spotCursor = 0, tripleCursor = 0, heldHeightCursor = 0, kindCursor = 0;
    std::array<bool, 3> introduced {};                 // which kinds of trial have been heard once through

    // Passes
    Goal goal = Goal::straighten;
    std::vector<bool> covered; // plane by kind, this pass
    int passDone = 0, planeCursor = 0;
    bool keptInPass = false, keptInRound = false;
    int adaptiveRun = 0;
    int nextDirectionOrder = 0;

    // The turn
    Phase phase = Phase::idle;
    int turnNumber = 0;
    Plane plane;
    Trial turnTrial, playingTrial;
    Goal turnGoal = Goal::straighten;
    Vector start, candidate;
    juce::Point<float> pad;
    std::vector<Trial> validationTrials;
    std::vector<Response> responses;
    int validationIndex = 0;
    bool heardAfter = false, hearingStart = false;
};
