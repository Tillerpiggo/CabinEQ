/*
  ==============================================================================

    GuidedSearch.h

    Experimental: a guided way to calibrate. Each turn it gives you a plane to
    explore with a pad, plays a fixed frame of bursts while you do, and keeps what
    you choose only if it holds up against the curve you started the turn from on
    two more frames.

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

    After a choice that holds up:
        if it touched the pad's edge, the same plane again, recentred;
        otherwise an adaptive plane: across is that movement (continue it or undo
        it), up is the least-used direction that isn't nearly parallel to it,
        with its part along the movement taken out.
    After one that doesn't, or no improvement: the next structured plane.
    After two adaptive turns running: one structured turn.

    A sweep is every structured plane on every frame; adaptive turns are extra.
    A sweep with nothing kept is followed by an audit on frames at in-between
    positions. If that finds nothing either, the range halves; when it's under
    the smallest range, the vertices double (5, 9, 17), keeping the curve.

    This class is only the controller. It knows nothing of audio or the profile:
    it's given callbacks to read the vertices, preview and commit a curve, play a
    frame, and refine the vertices, and it's told what happens (the pad moved, a
    frame finished playing, what you answered).

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <numeric>

class GuidedSearch
{
public:
    using Vector = std::vector<float>;

    //==============================================================================
    /// Twelve bursts around one spot of a 3 x 3 x 3 block of pan, height and level (each -1, 0 or 1; an audit's
    /// are in between). Three lines through (a, b): across the pans at height a and level b, up the heights at pan a
    /// and level b, and through the levels at pan a and height b. Each line goes low, middle, high, middle.
    struct Frame
    {
        float a = 0.0f, b = 0.0f;
        bool isAudit = false;

        struct Burst { float pan, height, level; };

        std::array<Burst, 12> bursts() const
        {
            std::array<Burst, 12> all {};
            size_t next = 0;
            for (int line = 0; line < 3; ++line)
                for (float t : { -1.0f, 0.0f, 1.0f, 0.0f })
                    all[next++] = line == 0 ? Burst { t, a, b } : line == 1 ? Burst { a, t, b } : Burst { a, b, t };
            return all;
        }

        bool operator== (const Frame& other) const { return a == other.a && b == other.b && isAudit == other.isAudit; }
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
        int validationFrames = 2;
        int adaptiveTurnsBeforeStructured = 2;
    };

    struct Callbacks
    {
        std::function<Vector()> readVertices;                    // left gains, then right gains, as the curve is now
        std::function<void (const Vector&)> preview;             // hear this curve; it's not kept yet
        std::function<void (const Vector&)> commit;              // keep this one
        std::function<void (const Vector&)> revert;              // back to this one, the turn's start
        std::function<Vector (int vertexCount)> refine;          // make the curve this many vertices, keeping its shape
        std::function<void (const Frame&)> playFrame;            // from its first burst, round and round
        std::function<juce::String (int vertex, int vertexCount)> vertexName; // "632 Hz", for naming directions
        std::function<void()> turnStarted;
        std::function<void (const juce::var&)> log;
    };

    enum class Phase { idle, exploring, settling, deciding, validatingBefore, validatingAfter, finished };
    enum class Response { better, same, worse };

    //==============================================================================
    GuidedSearch (Config configToUse, Callbacks callbacksToUse)
        : config (std::move (configToUse)), callbacks (std::move (callbacksToUse))
    {
        for (float a : { -1.0f, 0.0f, 1.0f })
            for (float b : { -1.0f, 0.0f, 1.0f })
                frames.push_back ({ a, b, false });
        for (float a : { -0.5f, 0.5f })
            for (float b : { -0.5f, 0.5f })
                auditFrames.push_back ({ a, b, true });
    }

    /// Starts from the curve as it is, at the coarsest vertices and the widest range
    void begin()
    {
        levelIndex = 0;
        range = config.startRangeDb;
        directions.clear();
        turnNumber = 0;
        callbacks.refine (config.vertexCounts[0]); // the curve as it is, through the coarsest vertices
        setVertexCount (config.vertexCounts[0], {});
        log ("begin", {});
        startSweep();
        startStructuredTurn();
    }

    //==============================================================================
    // What's happening, for whatever's showing it
    Phase getPhase() const               { return phase; }
    const Plane& getPlane() const        { return plane; }
    const Frame& getFrame() const        { return playingFrame; }
    float getRangeDb() const             { return range; }
    int getVertexCount() const           { return numVertices; }
    juce::Point<float> getPad() const    { return pad; }
    int getTurnNumber() const            { return turnNumber; }
    bool isAuditing() const              { return stage == Stage::audit; }
    int getValidationIndex() const       { return validationIndex; }
    int getNumStructuredPlanes() const   { return (int) structuredPlanes.size(); }
    int getSweepSize() const             { return (int) (structuredPlanes.size() * frames.size()); }
    int getSweepDone() const             { return sweepDone; }
    const Vector& getStart() const       { return start; }
    Vector getCandidate() const          { return previewAt (pad); }
    bool hasMoved() const                { return std::abs (pad.x) > 1.0e-4f || std::abs (pad.y) > 1.0e-4f; }

    /// Whether you can say yet that the choice is better: it's had its one complete frame, and it's a change
    bool canChoose() const               { return phase == Phase::deciding && hasMoved(); }
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
        if (phase != Phase::exploring && phase != Phase::settling && phase != Phase::deciding)
            return;
        if (phase != Phase::exploring)
        {
            phase = Phase::exploring;
            play (turnFrame);
        }
        pad = { juce::jlimit (-1.0f, 1.0f, at.x), juce::jlimit (-1.0f, 1.0f, at.y) };
        callbacks.preview (previewAt (pad));
    }

    /// Let go: one complete frame plays with the choice, standing still, before it can be called better
    void padReleased()
    {
        if (phase != Phase::exploring)
            return;
        phase = Phase::settling;
        play (turnFrame);
    }

    /// The frame that's playing has played right through
    void framePlayed()
    {
        if (phase == Phase::settling)
        {
            phase = Phase::deciding;
        }
        else if (phase == Phase::validatingBefore)
        {
            phase = Phase::validatingAfter;
            callbacks.preview (candidate);
            play (playingFrame);
        }
        else if (phase == Phase::validatingAfter)
        {
            heardAfter = true;
            phase = Phase::validatingBefore;
            callbacks.preview (start);
            play (playingFrame);
        }
    }

    /// "It's better" goes on to check it on other frames; "it's not" drops it and moves to the next structured plane
    void choose (bool isBetter)
    {
        if (phase != Phase::exploring && phase != Phase::settling && phase != Phase::deciding)
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
        startValidation();
    }

    /// Against the starting curve on this frame, the change is...
    void respond (Response response)
    {
        if (! canRespond())
            return;
        responses.push_back (response);
        if (response == Response::worse)
        {
            finishTurn (false); // kept only if it's worse on neither
        }
        else if (++validationIndex < config.validationFrames)
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

    //==============================================================================
    // For tests, and for anyone curious
    struct Direction
    {
        Vector vector;
        juce::String name;
        int uses = 0;
        int order = -1; // where it first came up in the queue of structured planes; later ones follow on
    };

    /// The least-used direction that isn't nearly parallel to `along`. Of those used equally little, the one that
    /// came first in the original queue. -1 if there isn't one.
    static int leastUsedIndependent (const Vector& along, const std::vector<Direction>& pool, float nearlyParallel)
    {
        int best = -1;
        for (int i = 0; i < (int) pool.size(); ++i)
        {
            const auto& candidate = pool[(size_t) i];
            if (std::abs (cosine (along, candidate.vector)) > nearlyParallel)
                continue;
            if (best < 0 || candidate.uses < pool[(size_t) best].uses
                || (candidate.uses == pool[(size_t) best].uses && candidate.order < pool[(size_t) best].order))
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

    /// Puts the turn back to how it started: whatever was being tried is dropped, and the pad's in the middle again.
    /// For when you leave partway through one.
    void resetTurn()
    {
        if (phase == Phase::idle || phase == Phase::finished)
            return;
        callbacks.revert (start);
        pad = {};
        heardAfter = false;
        responses.clear();
        validationIndex = 0;
        phase = Phase::exploring;
        if (callbacks.turnStarted)
            callbacks.turnStarted();
        play (turnFrame);
    }
    const std::vector<Direction>& getDirections() const { return directions; }
    const std::vector<Plane>& getStructuredPlanes() const { return structuredPlanes; }

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
    enum class Stage { sweep, audit };

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
        planeDirections.clear();
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
    // Turns

    void startSweep()
    {
        stage = Stage::sweep;
        covered.assign (structuredPlanes.size() * frames.size(), false);
        sweepDone = 0;
        keptInSweep = false;
    }

    /// The next structured plane, on a frame it's not had this sweep (or, in an audit, the next audit frame)
    void startStructuredTurn()
    {
        if (stage == Stage::sweep && sweepDone >= getSweepSize())
            endSweep();
        else if (stage == Stage::audit && auditTurnsLeft <= 0)
            endAudit();
        if (phase == Phase::finished)
            return;

        adaptiveRun = 0;
        const auto planeIndex = (size_t) planeCursor;
        planeCursor = (planeCursor + 1) % (int) structuredPlanes.size();
        ++directions[(size_t) planeDirections[planeIndex].first].uses;
        ++directions[(size_t) planeDirections[planeIndex].second].uses;

        Frame frame;
        if (stage == Stage::audit)
        {
            frame = auditFrames[(size_t) (auditCursor++ % (int) auditFrames.size())];
            --auditTurnsLeft;
        }
        else
        {
            // The first frame from where we are that this plane hasn't had
            for (size_t step = 0; step < frames.size(); ++step)
            {
                const size_t frameIndex = ((size_t) frameCursor + step) % frames.size();
                if (! covered[planeIndex * frames.size() + frameIndex])
                {
                    covered[planeIndex * frames.size() + frameIndex] = true;
                    ++sweepDone;
                    frame = frames[frameIndex];
                    frameCursor = (int) ((frameIndex + 1) % frames.size());
                    break;
                }
            }
        }
        startTurn (structuredPlanes[planeIndex], frame);
    }

    void startTurn (const Plane& planeToExplore, const Frame& frame)
    {
        ++turnNumber;
        plane = planeToExplore;
        turnFrame = frame;
        start = callbacks.readVertices();
        start.resize ((size_t) numVertices * 2, 0.0f);
        pad = {};
        heardAfter = false;
        responses.clear();
        phase = Phase::exploring;
        if (callbacks.turnStarted)
            callbacks.turnStarted();
        play (turnFrame);
    }

    void play (const Frame& frame)
    {
        playingFrame = frame;
        callbacks.playFrame (frame);
    }

    /// The frame after the turn's (then the one after that), from the same set
    void startValidation()
    {
        const auto& set = turnFrame.isAudit ? auditFrames : frames;
        size_t at = 0;
        for (size_t i = 0; i < set.size(); ++i)
            if (set[i] == turnFrame)
                at = i;
        heardAfter = false;
        phase = Phase::validatingBefore;
        callbacks.preview (start);
        play (set[(at + 1 + (size_t) validationIndex) % set.size()]);
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
            (stage == Stage::audit ? keptInAudit : keptInSweep) = true;

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
            // There may be further to go: the same plane, at the same range, from where it's got to
            if (plane.isAdaptive)
                ++adaptiveRun;
            startTurn (plane, turnFrame);
            return;
        }

        if (kept && adaptiveAllowed)
        {
            if (auto adaptive = adaptivePlaneFrom (movement))
            {
                ++adaptiveRun;
                startTurn (*adaptive, nextFrame());
                return;
            }
        }
        startStructuredTurn();
    }

    /// For a turn that isn't one of the sweep's: just the next frame along
    Frame nextFrame()
    {
        if (stage == Stage::audit)
            return auditFrames[(size_t) (auditCursor++ % (int) auditFrames.size())];
        const auto frame = frames[(size_t) frameCursor];
        frameCursor = (frameCursor + 1) % (int) frames.size();
        return frame;
    }

    //==============================================================================
    // Sweeps, ranges and refining

    void endSweep()
    {
        if (keptInSweep)
        {
            log ("sweep", { { "kept", true } });
            startSweep(); // something improved: the same range again
            return;
        }

        // Nothing kept. Before narrowing the range, look at the positions in between.
        log ("sweep", { { "kept", false } });
        stage = Stage::audit;
        auditTurnsLeft = (int) structuredPlanes.size();
        keptInAudit = false;
    }

    void endAudit()
    {
        log ("audit", { { "kept", keptInAudit } });
        if (! keptInAudit)
        {
            range *= 0.5f;
            if (range < config.minRangeDb - 1.0e-4f)
            {
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
            else
            {
                log ("range", {});
            }
        }
        startSweep();
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

    void log (const juce::String& event, std::initializer_list<std::pair<const char*, juce::var>> more)
    {
        if (! callbacks.log)
            return;
        auto* object = new juce::DynamicObject();
        object->setProperty ("event", event);
        object->setProperty ("turn", turnNumber);
        object->setProperty ("vertices", numVertices);
        object->setProperty ("rangeDb", range);
        for (const auto& [key, value] : more)
            object->setProperty (key, value);
        callbacks.log (juce::var (object));
    }

    void logTurn (bool kept, const Vector& chosen)
    {
        if (! callbacks.log)
            return;
        juce::Array<juce::var> answered;
        for (auto response : responses)
            answered.add (response == Response::better ? "better" : response == Response::same ? "same" : "worse");

        auto* frame = new juce::DynamicObject();
        frame->setProperty ("a", turnFrame.a);
        frame->setProperty ("b", turnFrame.b);
        frame->setProperty ("audit", turnFrame.isAudit);

        auto* object = new juce::DynamicObject();
        object->setProperty ("event", "turn");
        object->setProperty ("turn", turnNumber);
        object->setProperty ("vertices", numVertices);
        object->setProperty ("rangeDb", range);
        object->setProperty ("adaptive", plane.isAdaptive);
        object->setProperty ("structuredPlane", plane.structuredIndex);
        object->setProperty ("uName", plane.uName);
        object->setProperty ("vName", plane.vName);
        object->setProperty ("u", toVar (plane.u));
        object->setProperty ("v", toVar (plane.v));
        object->setProperty ("frame", juce::var (frame));
        object->setProperty ("start", toVar (start));
        object->setProperty ("x", pad.x);
        object->setProperty ("y", pad.y);
        object->setProperty ("chosen", toVar (chosen));
        object->setProperty ("validation", answered);
        object->setProperty ("kept", kept);
        callbacks.log (juce::var (object));
    }

    //==============================================================================
    Config config;
    Callbacks callbacks;

    std::vector<Frame> frames, auditFrames;
    std::vector<Direction> directions;
    std::vector<Plane> structuredPlanes;
    std::vector<std::pair<int, int>> planeDirections; // each structured plane's two directions

    int numVertices = 0, levelIndex = 0;
    float range = 30.0f;

    Stage stage = Stage::sweep;
    std::vector<bool> covered; // plane by frame, this sweep
    int sweepDone = 0, planeCursor = 0, frameCursor = 0;
    int auditTurnsLeft = 0, auditCursor = 0;
    bool keptInSweep = false, keptInAudit = false;
    int adaptiveRun = 0;
    int nextDirectionOrder = 0;

    // The turn
    Phase phase = Phase::idle;
    int turnNumber = 0;
    Plane plane;
    Frame turnFrame, playingFrame;
    Vector start, candidate;
    juce::Point<float> pad;
    std::vector<Response> responses;
    int validationIndex = 0;
    bool heardAfter = false;
};
