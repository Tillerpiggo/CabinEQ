/*
  ==============================================================================

    FilterChain.h
    Created: 13 Oct 2024 1:06:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <map>
#include "BandProfile.h"
#include "FilterDesign.h"

/// The EQ's filters, as a fixed pool of biquads per channel that's allocated up front.
///
/// The message thread describes the bands it wants with setBands(). The audio thread picks that
/// up at the start of its next block (without waiting on a lock) and glides each filter's
/// frequency, gain and Q towards it, so dragging a band or switching profiles never clicks.
/// A band that's added, removed, disabled or changes shape fades in or out instead of switching.
class FilterChain
{
public:
    static constexpr int maxBands = 24; // how many bands a profile can have
    static constexpr int numSlots = 32; // extra slots let removed bands fade out while new ones fade in
    static constexpr int maxChannels = 2;

    FilterChain()
    {
        slotReleasedAt.fill (0);
    }

    /// Call while audio isn't running.
    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate;
        const double samplesPerSecond = sampleRate / subBlockSize;
        smoothing = 1.0 - std::exp (-1.0 / (glideSeconds * samplesPerSecond));
        presenceStep = 1.0 / (fadeSeconds * samplesPerSecond);
        reset();
    }

    /// Jumps straight to the latest bands and clears the filters' state. Call while audio isn't running.
    void reset()
    {
        const juce::SpinLock::ScopedLockType lock (pendingLock);
        for (int channel = 0; channel < maxChannels; ++channel)
        {
            for (int i = 0; i < numSlots; ++i)
            {
                auto& slot = slots[channel][i];
                slot.target = pendingTargets[channel][i];
                slot.running = slot.target.active;
                slot.shape = slot.target.shape;
                slot.logFreq = slot.target.logFreq;
                slot.gainDb = slot.target.gainDb;
                slot.logQ = slot.target.logQ;
                slot.presence = slot.running ? 1.0 : 0.0;
                slot.s1 = slot.s2 = 0.0;
                updateCoefficients (slot);
            }
        }
        hasPendingTargets = false;
    }

    /// Audio thread. Like reset(), but never waits on the message thread.
    void clearState() noexcept
    {
        takePendingTargets();
        for (auto& channelSlots : slots)
        {
            for (auto& slot : channelSlots)
            {
                slot.running = slot.target.active;
                slot.shape = slot.target.shape;
                slot.logFreq = slot.target.logFreq;
                slot.gainDb = slot.target.gainDb;
                slot.logQ = slot.target.logQ;
                slot.presence = slot.running ? 1.0 : 0.0;
                slot.s1 = slot.s2 = 0.0;
                updateCoefficients (slot);
            }
        }
    }

    /// Message thread only. Disabled bands fade out but keep their slot, so re-enabling fades them back in.
    void setBands (const std::vector<Band>& bands)
    {
        std::array<std::array<Target, numSlots>, maxChannels> targets {};
        std::map<int, int> newSlotForBandId;

        for (const auto& band : bands)
        {
            if ((int) newSlotForBandId.size() >= numSlots)
                break;

            int slot = findOrAllocateSlot (band.id, newSlotForBandId);
            if (slot < 0)
                continue;

            newSlotForBandId[band.id] = slot;
            for (int channel = 0; channel < maxChannels; ++channel)
            {
                auto& target = targets[channel][slot];
                target.active = band.enabled && band.appliesToChannel (channel);
                target.shape = band.shape;
                target.logFreq = std::log ((double) juce::jlimit (Band::minFreq, Band::maxFreq, band.freq));
                target.gainDb = band.hasGain() ? band.ampl : 0.0;
                target.logQ = std::log ((double) juce::jlimit (Band::minQ, Band::maxQ, band.qFactor));
            }
        }

        // Remember when slots were freed, so new bands take the slot that's had longest to fade out
        for (const auto& [id, slot] : slotForBandId)
            if (newSlotForBandId.find (id) == newSlotForBandId.end())
                slotReleasedAt[(size_t) slot] = ++releaseCounter;
        slotForBandId = std::move (newSlotForBandId);

        const juce::SpinLock::ScopedLockType lock (pendingLock);
        pendingTargets = targets;
        hasPendingTargets = true;
    }

    /// Audio thread. Channel 0 gets the left bands and channel 1 the right; any others pass through.
    void process (juce::dsp::AudioBlock<float>& block) noexcept
    {
        takePendingTargets();

        const int numChannels = std::min ((int) block.getNumChannels(), maxChannels);
        const int numSamples = (int) block.getNumSamples();

        for (int start = 0; start < numSamples; start += subBlockSize)
        {
            const int length = std::min (subBlockSize, numSamples - start);
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* data = block.getChannelPointer ((size_t) channel) + start;
                for (auto& slot : slots[channel])
                {
                    if (! advance (slot))
                        continue;

                    const double b0 = slot.c.b0, b1 = slot.c.b1, b2 = slot.c.b2, a1 = slot.c.a1, a2 = slot.c.a2;
                    double s1 = slot.s1, s2 = slot.s2;
                    for (int i = 0; i < length; ++i)
                    {
                        const double x = data[i];
                        const double y = b0 * x + s1;
                        s1 = b1 * x - a1 * y + s2;
                        s2 = b2 * x - a2 * y;
                        data[i] = (float) y;
                    }

                    // Written so it also catches NaN; -ffast-math turns std::isfinite into "true"
                    if (! (std::abs (s1) < 1.0e20 && std::abs (s2) < 1.0e20))
                        s1 = s2 = 0.0;
                    slot.s1 = s1;
                    slot.s2 = s2;
                }
            }
        }
    }

private:
    struct Target
    {
        bool active = false;
        Band::Shape shape = Band::Shape::peak;
        double logFreq = std::log (1000.0);
        double gainDb = 0.0;
        double logQ = 0.0;
    };

    struct Slot
    {
        Target target;
        bool running = false;
        Band::Shape shape = Band::Shape::peak;
        double logFreq = std::log (1000.0), gainDb = 0.0, logQ = 0.0;
        double presence = 0.0; // 0 = inaudible, 1 = fully applied
        FilterDesign::Biquad c;
        double s1 = 0.0, s2 = 0.0;
    };

    int findOrAllocateSlot (int bandId, const std::map<int, int>& taken)
    {
        auto isTaken = [&taken] (int slot)
        {
            for (const auto& entry : taken)
                if (entry.second == slot)
                    return true;
            return false;
        };

        auto existing = slotForBandId.find (bandId);
        if (existing != slotForBandId.end() && ! isTaken (existing->second))
            return existing->second;

        int best = -1;
        for (int slot = 0; slot < numSlots; ++slot)
        {
            bool isFree = ! isTaken (slot);
            for (const auto& entry : slotForBandId)
                if (entry.second == slot && entry.first != bandId)
                    isFree = false;

            if (isFree && (best < 0 || slotReleasedAt[(size_t) slot] < slotReleasedAt[(size_t) best]))
                best = slot;
        }
        return best;
    }

    void takePendingTargets() noexcept
    {
        const juce::SpinLock::ScopedTryLockType lock (pendingLock);
        if (! lock.isLocked() || ! hasPendingTargets)
            return;

        for (int channel = 0; channel < maxChannels; ++channel)
            for (int i = 0; i < numSlots; ++i)
                slots[channel][i].target = pendingTargets[channel][i];
        hasPendingTargets = false;
    }

    /// Moves a slot one sub-block closer to its target. Returns false if the slot is silent and can be skipped.
    bool advance (Slot& slot) noexcept
    {
        const auto& target = slot.target;

        if (! slot.running)
        {
            if (! target.active)
                return false;

            // Start from where the target is, but inaudible, and fade in
            slot.running = true;
            slot.shape = target.shape;
            slot.logFreq = target.logFreq;
            slot.gainDb = target.gainDb;
            slot.logQ = target.logQ;
            slot.presence = 0.0;
            slot.s1 = slot.s2 = 0.0;
        }

        // A new shape can only take over once the old one has faded out
        const bool sameShape = slot.shape == target.shape;
        if (target.active && ! sameShape && slot.presence <= 0.0)
        {
            slot.shape = target.shape;
            slot.logFreq = target.logFreq;
            slot.gainDb = target.gainDb;
            slot.logQ = target.logQ;
        }

        bool changed = false;
        const double wantedPresence = (target.active && slot.shape == target.shape) ? 1.0 : 0.0;
        if (slot.presence != wantedPresence)
        {
            slot.presence = wantedPresence > slot.presence ? std::min (1.0, slot.presence + presenceStep)
                                                           : std::max (0.0, slot.presence - presenceStep);
            changed = true;
        }

        if (target.active && slot.shape == target.shape)
        {
            changed |= glide (slot.logFreq, target.logFreq, 1.0e-5);
            changed |= glide (slot.gainDb, target.gainDb, 1.0e-4);
            changed |= glide (slot.logQ, target.logQ, 1.0e-5);
        }

        if (! target.active && slot.presence <= 0.0)
        {
            slot.running = false;
            return false;
        }

        if (changed)
            updateCoefficients (slot);
        return true;
    }

    bool glide (double& value, double target, double snapDistance) const noexcept
    {
        if (value == target)
            return false;
        value += (target - value) * smoothing;
        if (std::abs (target - value) < snapDistance)
            value = target;
        return true;
    }

    void updateCoefficients (Slot& slot) const noexcept
    {
        double logFreq = slot.logFreq;
        double gainDb = slot.gainDb;

        // Fade gains towards 0 dB, and cuts towards the edge of the spectrum, where they do nothing
        if (slot.shape == Band::Shape::lowCut)
            logFreq = juce::jmap (slot.presence, std::log ((double) Band::minFreq), slot.logFreq);
        else if (slot.shape == Band::Shape::highCut)
            logFreq = juce::jmap (slot.presence, std::log (FilterDesign::maxFrequency (sampleRate)), slot.logFreq);
        else
            gainDb *= slot.presence;

        slot.c = FilterDesign::design (slot.shape, std::exp (logFreq), gainDb, std::exp (slot.logQ), sampleRate);
    }

    static constexpr int subBlockSize = 32; // how often coefficients update while gliding
    static constexpr double glideSeconds = 0.02;
    static constexpr double fadeSeconds = 0.03;

    double sampleRate = 48000.0;
    double smoothing = 0.03;
    double presenceStep = 0.02;

    // Message thread
    std::map<int, int> slotForBandId;
    std::array<juce::uint32, numSlots> slotReleasedAt;
    juce::uint32 releaseCounter = 0;

    // Handed from the message thread to the audio thread
    juce::SpinLock pendingLock;
    std::array<std::array<Target, numSlots>, maxChannels> pendingTargets {};
    bool hasPendingTargets = false;

    // Audio thread
    std::array<std::array<Slot, numSlots>, maxChannels> slots {};
};
