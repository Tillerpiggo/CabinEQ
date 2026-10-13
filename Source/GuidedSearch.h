/*
  ==============================================================================

    GuidedSearch.h

    Experimental: a guided way to calibrate. Each turn it gives you a plane to
    explore with a pad, plays a whole arrangement of sounds while you do (left
    and right, low and high, far and near, all at once), and keeps what you
    choose only if it holds up against the curve you started the turn from when
    the arrangement's played in another order, and with more positions in it.

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

    What you judge is never one dimension by itself. An arrangement is a box of
    sounds: two layers across (pan), two up (the low cut) and two deep (level), so
    its eight corners; and it's played by going from each to a neighbour, one
    step along one dimension at a time, round and round. One dimension changes at
    every other step, one within each half of the round and one between the
    halves, and which is which goes round from turn to turn, so that none of them
    always has the easiest comparisons. The order stays as it is through a turn.
    The first time an arrangement comes up, it plays once through before the pad
    does anything.

    After a choice that holds up:
        if it touched the pad's edge, the same plane and arrangement again, recentred;
        otherwise an adaptive plane: across is that movement (continue it or undo
        it), up is the least-used direction that isn't nearly parallel to it,
        with its part along the movement taken out.
    After one that doesn't, or no improvement: the next structured plane.
    After two adaptive turns running: one structured turn.

    A pass is every structured plane once; adaptive turns are extra. After a pass
    with nothing kept, the arrangement gets a layer between two of its sides:
    three layers along one dimension, which goes round from turn to turn, and so
    twelve sounds. First the layer's halfway; after another such pass it's nearer
    one side or the other as well. The corners are always there. Only when
    that's as fine as it goes does the range halve; when it's under the smallest
    range, the vertices double (5, 9, 17), keeping the curve.

    This class is only the controller. It knows nothing of audio or the profile:
    it's given callbacks to read the vertices, preview and commit a curve, play an
    arrangement, and refine the vertices, and it's told what happens (the pad
    moved, the arrangement finished playing, what you answered).

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <numeric>

class GuidedSearch
{
public:
    using Vector = std::vector<float>;

    enum class Dimension { width = 0, height = 1, depth = 2 };

    /// Where a sound is: its pan, its height (how high its low cut is) and its level, each from -1 to 1
    struct Position
    {
        float pan = 0.0f, height = 0.0f, level = 0.0f;

        float& along (Dimension dimension)      { return dimension == Dimension::width ? pan : dimension == Dimension::height ? height : level; }
        float along (Dimension dimension) const { return dimension == Dimension::width ? pan : dimension == Dimension::height ? height : level; }
        bool operator== (const Position& other) const { return pan == other.pan && height == other.height && level == other.level; }
    };

    //==============================================================================
    /// A box of sounds, and the order they're played in
    struct Arrangement
    {
        /// Where each dimension's layers are, lowest first: its two ends, or those with one between
        std::array<std::vector<float>, 3> layers { { { -1.0f, 1.0f }, { -1.0f, 1.0f }, { -1.0f, 1.0f } } };

        /// The dimension that changes at every other step (the only one that can have a layer between its ends),
        /// the one that changes within each half of the round, and the one that changes between the halves
        std::array<Dimension, 3> order { Dimension::width, Dimension::height, Dimension::depth };

        const std::vector<float>& layersOf (Dimension dimension) const { return layers[(size_t) dimension]; }
        int size() const { return (int) (layers[0].size() * layers[1].size() * layers[2].size()); }

        /// The dimension with a layer between its ends, if there is one
        std::optional<Dimension> middleDimension() const
        {
            for (auto dimension : { Dimension::width, Dimension::height, Dimension::depth })
                if (layersOf (dimension).size() > 2)
                    return dimension;
            return std::nullopt;
        }

        /// Every sound once, each a neighbour of the last (and the first of the last): one step along one dimension.
        /// Out along the first dimension at the second's low end and back at its high end, at the third's low end;
        /// then the same backwards at the third's high end.
        std::vector<Position> positions() const
        {
            const auto& first = layersOf (order[0]);
            const auto& second = layersOf (order[1]);
            const auto& third = layersOf (order[2]);
            jassert (second.size() == 2 && third.size() == 2);

            std::vector<std::pair<size_t, size_t>> ring;
            for (size_t i = 0; i < first.size(); ++i)
                ring.push_back ({ i, 0 });
            for (size_t i = first.size(); i-- > 0;)
                ring.push_back ({ i, 1 });

            std::vector<Position> all;
            for (size_t half = 0; half < 2; ++half)
                for (size_t step = 0; step < ring.size(); ++step)
                {
                    const auto [i, j] = ring[half == 0 ? step : ring.size() - 1 - step];
                    Position position;
                    position.along (order[0]) = first[i];
                    position.along (order[1]) = second[j];
                    position.along (order[2]) = third[half];
                    all.push_back (position);
                }
            return all;
        }

        /// Each sound this many times before the next
        std::vector<Position> bursts (int repetitions) const
        {
            std::vector<Position> all;
            for (const auto& position : positions())
                all.insert (all.end(), (size_t) std::max (1, repetitions), position);
            return all;
        }

        /// The same sounds, played in another order
        Arrangement reordered() const
        {
            auto other = *this;
            if (middleDimension().has_value())
                std::swap (other.order[1], other.order[2]); // the one with three layers has to stay first
            else
                other.order = { order[1], order[2], order[0] };
            return other;
        }

        bool operator== (const Arrangement& other) const { return layers == other.layers && order == other.order; }
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

        /// Where the layer between a dimension's two ends can be, for each level of coverage: none to begin with
        /// (just the corners), then halfway, then nearer one end or the other as well
        std::vector<std::vector<float>> middleLayers { {}, { 0.0f }, { 0.0f, -0.5f, 0.5f } };
    };

    struct Callbacks
    {
        std::function<Vector()> readVertices;                    // left gains, then right gains, as the curve is now
        std::function<void (const Vector&)> preview;             // hear this curve; it's not kept yet
        std::function<void (const Vector&)> commit;              // keep this one
        std::function<void (const Vector&)> revert;              // back to this one, the turn's start
        std::function<Vector (int vertexCount)> refine;          // make the curve this many vertices, keeping its shape
        std::function<void (const Arrangement&)> playArrangement; // from its first sound, round and round
        std::function<juce::String (int vertex, int vertexCount)> vertexName; // "632 Hz", for naming directions
        std::function<void()> turnStarted;
        std::function<void (const juce::var&)> log;
    };

    /// introducing: an arrangement's first time, playing once through before the pad does anything
    enum class Phase { idle, introducing, exploring, settling, deciding, validatingBefore, validatingAfter, finished };
    enum class Response { better, same, worse };

    //==============================================================================
    GuidedSearch (Config configToUse, Callbacks callbacksToUse)
        : config (std::move (configToUse)), callbacks (std::move (callbacksToUse))
    {
    }

    /// Starts from the curve as it is, at the coarsest vertices, the widest range and just the corners
    void begin()
    {
        levelIndex = 0;
        range = config.startRangeDb;
        directions.clear();
        turnNumber = 0;
        introduced = {};
        coverageLevel = 0;
        planeCursor = arrangementCursor = checkCursor = 0;
        callbacks.refine (config.vertexCounts[0]); // the curve as it is, through the coarsest vertices
        setVertexCount (config.vertexCounts[0], {});
        log ("begin", {});
        startPass();
        startStructuredTurn();
    }

    //==============================================================================
    // What's happening, for whatever's showing it
    Phase getPhase() const               { return phase; }
    const Plane& getPlane() const        { return plane; }
    const Arrangement& getArrangement() const     { return playing; } // what's playing: the turn's own, or a check's
    const Arrangement& getTurnArrangement() const { return turnArrangement; }
    float getRangeDb() const             { return range; }
    int getVertexCount() const           { return numVertices; }
    juce::Point<float> getPad() const    { return pad; }
    int getTurnNumber() const            { return turnNumber; }
    int getValidationIndex() const       { return validationIndex; }
    int getNumChecks() const             { return 2; }
    int getNumStructuredPlanes() const   { return (int) structuredPlanes.size(); }
    int getPassSize() const              { return (int) structuredPlanes.size(); }
    int getPassDone() const              { return passDone; }
    int getCoverageLevel() const         { return coverageLevel; }
    int getNumCoverageLevels() const     { return (int) config.middleLayers.size(); }
    const Vector& getStart() const       { return start; }
    Vector getCandidate() const          { return previewAt (pad); }
    bool hasMoved() const                { return std::abs (pad.x) > 1.0e-4f || std::abs (pad.y) > 1.0e-4f; }
    bool isHearingStart() const          { return hearingStart; }

    /// Whether the pad does anything: not while an arrangement's being introduced, or a choice is being checked
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
            play (turnArrangement);
        }
        hearingStart = false; // dragging is for hearing the change
        pad = { juce::jlimit (-1.0f, 1.0f, at.x), juce::jlimit (-1.0f, 1.0f, at.y) };
        callbacks.preview (previewAt (pad));
    }

    /// Let go: the arrangement plays once right through with the choice, standing still, before it can be called better
    void padReleased()
    {
        if (phase != Phase::exploring)
            return;
        phase = Phase::settling;
        play (turnArrangement);
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
            play (turnArrangement);
        }
    }

    /// The arrangement that's playing has played right through
    void framePlayed()
    {
        if (phase == Phase::introducing)
        {
            setIntroduced (turnArrangement);
            phase = Phase::exploring;
        }
        else if (phase == Phase::settling)
        {
            if (! hearingStart)
                phase = Phase::deciding;
        }
        else if (phase == Phase::validatingBefore)
        {
            setIntroduced (playing); // heard with the curve standing still
            phase = Phase::validatingAfter;
            callbacks.preview (candidate);
            play (playing);
        }
        else if (phase == Phase::validatingAfter)
        {
            heardAfter = true;
            phase = Phase::validatingBefore;
            callbacks.preview (start);
            play (playing);
        }
    }

    /// "It's better" goes on to check it: on the same sounds in another order, and then on more positions.
    /// "It's not" (or skipping the turn) drops it and moves to the next structured plane.
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

        // Both are the whole arrangement again, never a part of it, each with the same sounds for its before and after
        checks = { turnArrangement.reordered(), withMorePositions (turnArrangement) };
        startValidation();
    }

    /// Against the starting curve on this check, the change is...
    void respond (Response response)
    {
        if (! canRespond())
            return;
        responses.push_back (response);
        if (response == Response::worse)
        {
            finishTurn (false); // kept only if it's worse on neither
        }
        else if (++validationIndex < (int) checks.size())
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
    // Arrangements

    static Dimension dimensionAt (int index) { return (Dimension) (((index % 3) + 3) % 3); }

    /// The box with a layer between the ends of one dimension, which then has to be the one that changes most
    static Arrangement withMiddleLayer (Dimension dimension, float at, bool otherWayRound)
    {
        Arrangement arrangement;
        arrangement.layers[(size_t) dimension] = { -1.0f, at, 1.0f };
        arrangement.order = { dimension, dimensionAt ((int) dimension + (otherWayRound ? 2 : 1)), dimensionAt ((int) dimension + (otherWayRound ? 1 : 2)) };
        return arrangement;
    }

    /// The next arrangement to explore with. Just the corners, to begin with, each dimension taking its turn as
    /// the one that changes most; then with a layer between, each dimension taking its turn to have it, and the
    /// places it can be coming round after that.
    Arrangement nextArrangement()
    {
        const int turn = arrangementCursor++;
        const auto& middles = config.middleLayers[(size_t) coverageLevel];
        if (middles.empty())
        {
            Arrangement corners;
            corners.order = { dimensionAt (turn), dimensionAt (turn + 1), dimensionAt (turn + 2) };
            return corners;
        }
        const int round = turn / 3;
        return withMiddleLayer (dimensionAt (turn), middles[(size_t) round % middles.size()], round % 2 == 1);
    }

    /// For checking a choice on positions it wasn't made with: a layer between the ends of a dimension that
    /// hasn't got one, the dimensions (and the places the layer can be) taking it in turn
    Arrangement withMorePositions (const Arrangement& arrangement)
    {
        std::vector<Dimension> candidates;
        for (auto dimension : { Dimension::width, Dimension::height, Dimension::depth })
            if (arrangement.middleDimension() != dimension)
                candidates.push_back (dimension);

        const int turn = checkCursor++;
        const auto& middles = config.middleLayers[(size_t) std::max (coverageLevel, firstLevelWithMiddles())];
        const float at = middles.empty() ? 0.0f : middles[(size_t) (turn / (int) candidates.size()) % middles.size()];
        return withMiddleLayer (candidates[(size_t) turn % candidates.size()], at, false);
    }

    int firstLevelWithMiddles() const
    {
        for (int level = 0; level < (int) config.middleLayers.size(); ++level)
            if (! config.middleLayers[(size_t) level].empty())
                return level;
        return 0;
    }

    /// Which you've heard once through: just the corners, and with a layer between
    bool isIntroduced (const Arrangement& arrangement) const { return introduced[arrangement.middleDimension().has_value() ? 1 : 0]; }
    void setIntroduced (const Arrangement& arrangement)      { introduced[arrangement.middleDimension().has_value() ? 1 : 0] = true; }

    //==============================================================================
    // Turns

    void startPass()
    {
        passDone = 0;
        keptInPass = false;
    }

    /// The next structured plane, with the next arrangement
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
        ++passDone;
        startTurn (structuredPlanes[planeIndex], nextArrangement());
    }

    void startTurn (const Plane& planeToExplore, const Arrangement& arrangement)
    {
        ++turnNumber;
        plane = planeToExplore;
        turnArrangement = arrangement;
        start = callbacks.readVertices();
        start.resize ((size_t) numVertices * 2, 0.0f);
        beginTurn();
    }

    /// The turn from its beginning: the pad in the middle, and if this arrangement is new, once through first
    void beginTurn()
    {
        pad = {};
        heardAfter = false;
        hearingStart = false;
        responses.clear();
        checks.clear();
        validationIndex = 0;
        phase = isIntroduced (turnArrangement) ? Phase::exploring : Phase::introducing;
        if (callbacks.turnStarted)
            callbacks.turnStarted();
        play (turnArrangement);
    }

    void play (const Arrangement& arrangement)
    {
        playing = arrangement;
        callbacks.playArrangement (arrangement);
    }

    void startValidation()
    {
        heardAfter = false;
        phase = Phase::validatingBefore;
        callbacks.preview (start);
        play (checks[(size_t) validationIndex]);
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
            startTurn (plane, turnArrangement);
            return;
        }

        if (kept && adaptiveAllowed)
        {
            if (auto adaptive = adaptivePlaneFrom (movement))
            {
                ++adaptiveRun;
                startTurn (*adaptive, nextArrangement());
                return;
            }
        }
        startStructuredTurn();
    }

    //==============================================================================
    // Passes, coverage, ranges and refining

    /// A pass with nothing kept means there's nothing more to find like this
    void endPass()
    {
        log ("pass", { { "kept", keptInPass } });
        if (! keptInPass)
            lookCloserOrNarrow();
        if (phase != Phase::finished)
            startPass();
    }

    /// More positions in the arrangement first. Only when it's as fine as it goes, a smaller range; and under
    /// the smallest range, more vertices.
    void lookCloserOrNarrow()
    {
        if (coverageLevel + 1 < (int) config.middleLayers.size())
        {
            ++coverageLevel;
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

    static juce::var toVar (const Arrangement& arrangement)
    {
        static const char* const names[] { "width", "height", "depth" };
        juce::Array<juce::var> positions, order;
        for (const auto& position : arrangement.positions())
        {
            auto* object = new juce::DynamicObject();
            object->setProperty ("pan", position.pan);
            object->setProperty ("height", position.height);
            object->setProperty ("level", position.level);
            positions.add (juce::var (object));
        }
        for (auto dimension : arrangement.order)
            order.add (names[(size_t) dimension]);

        auto* object = new juce::DynamicObject();
        object->setProperty ("order", order);
        if (const auto middle = arrangement.middleDimension())
        {
            object->setProperty ("middleLayer", names[(size_t) *middle]);
            object->setProperty ("middleLayerAt", arrangement.layersOf (*middle)[1]);
        }
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
        juce::Array<juce::var> answered, checked;
        for (auto response : responses)
            answered.add (response == Response::better ? "better" : response == Response::same ? "same" : "worse");
        for (const auto& arrangement : checks)
            checked.add (toVar (arrangement));

        auto* object = new juce::DynamicObject();
        object->setProperty ("event", "turn");
        object->setProperty ("turn", turnNumber);
        object->setProperty ("vertices", numVertices);
        object->setProperty ("rangeDb", range);
        object->setProperty ("coverage", coverageLevel);
        object->setProperty ("adaptive", plane.isAdaptive);
        object->setProperty ("structuredPlane", plane.structuredIndex);
        object->setProperty ("uName", plane.uName);
        object->setProperty ("vName", plane.vName);
        object->setProperty ("u", toVar (plane.u));
        object->setProperty ("v", toVar (plane.v));
        object->setProperty ("arrangement", toVar (turnArrangement));
        object->setProperty ("start", toVar (start));
        object->setProperty ("x", pad.x);
        object->setProperty ("y", pad.y);
        object->setProperty ("chosen", toVar (chosen));
        object->setProperty ("checks", checked);
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

    // Arrangements: what goes round from turn to turn
    int coverageLevel = 0;
    int arrangementCursor = 0, checkCursor = 0;
    std::array<bool, 2> introduced {}; // heard once through: just the corners, and with a layer between

    // Passes
    int passDone = 0, planeCursor = 0;
    bool keptInPass = false;
    int adaptiveRun = 0;
    int nextDirectionOrder = 0;

    // The turn
    Phase phase = Phase::idle;
    int turnNumber = 0;
    Plane plane;
    Arrangement turnArrangement, playing;
    Vector start, candidate;
    juce::Point<float> pad;
    std::vector<Arrangement> checks;
    std::vector<Response> responses;
    int validationIndex = 0;
    bool heardAfter = false, hearingStart = false;
};
