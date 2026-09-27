/*
  ==============================================================================

    CabinEqProfile.cpp
    Created: 10 Oct 2024 7:51:38pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqProfile.h"
#include "FilterChain.h"

namespace
{
    // What profiles used to store before they were flattened
    const juce::Identifier idAmplTree { "AmplTree" };
    const juce::Identifier idMultiBandStep { "MultiBandStep" };
    const juce::Identifier idStepEnabled { "Enabled" };
    const juce::Identifier idLocked { "Locked" };
    const juce::Identifier idMelodyVolume { "MelodyVolume" };
    const juce::Identifier idNoiseVolume { "NoiseVolume" };
}

CabinEqProfile::CabinEqProfile (juce::ValueTree profileTree, juce::UndoManager* undoManager)
    : tree (profileTree), undoManager (undoManager)
{}

juce::ValueTree CabinEqProfile::createTree (const juce::String& name, const BandProfile& bandProfile)
{
    juce::ValueTree profileTree (idProfile);
    profileTree.setProperty (idProfileName, name, nullptr);
    profileTree.setProperty (idProfileVolume, bandProfile.getVolume(), nullptr);

    juce::ValueTree bandsTree (idBands);
    int nextId = 0;
    for (auto band : bandProfile.getBands())
    {
        band.id = nextId++;
        bandsTree.appendChild (treeFromBand (band), nullptr);
    }
    profileTree.appendChild (bandsTree, nullptr);
    return profileTree;
}

bool CabinEqProfile::migrate (juce::ValueTree profileTree)
{
    bool changed = false;

    for (const auto& oldProperty : { idLocked, idMelodyVolume, idNoiseVolume })
    {
        if (profileTree.hasProperty (oldProperty))
        {
            profileTree.removeProperty (oldProperty, nullptr);
            changed = true;
        }
    }

    if (! profileTree.hasProperty (idProfileVolume))
    {
        profileTree.setProperty (idProfileVolume, 0.0f, nullptr);
        changed = true;
    }

    auto bandsTree = profileTree.getChildWithName (idBands);
    if (! bandsTree.isValid())
    {
        bandsTree = juce::ValueTree (idBands);
        profileTree.appendChild (bandsTree, nullptr);
        changed = true;
    }

    auto amplTree = profileTree.getChildWithName (idAmplTree);
    if (amplTree.isValid())
    {
        int nextId = bandsTree.getNumChildren();
        for (const auto& step : amplTree)
        {
            if (! step.hasType (idMultiBandStep))
                continue;

            const bool stepIsEnabled = step.getProperty (idStepEnabled, true);
            for (const auto& oldBand : step)
            {
                if (! oldBand.hasType (idBand) || nextId >= FilterChain::maxBands)
                    continue;

                auto band = bandFromTree (oldBand);
                band.id = nextId++;
                band.enabled = band.enabled && stepIsEnabled;
                bandsTree.appendChild (treeFromBand (band), nullptr);
            }
        }

        profileTree.removeChild (amplTree, nullptr);
        changed = true;
    }

    return changed;
}

bool CabinEqProfile::isValid() const
{
    return tree.isValid() && tree.hasType (idProfile);
}

juce::ValueTree CabinEqProfile::getTree() const
{
    return tree;
}

juce::String CabinEqProfile::getName() const
{
    return tree.getProperty (idProfileName).toString();
}

float CabinEqProfile::getVolume() const
{
    return tree.getProperty (idProfileVolume, 0.0f);
}

BandProfile CabinEqProfile::getBandProfile() const
{
    std::vector<Band> bands;
    for (const auto& bandTree : tree.getChildWithName (idBands))
        if (bandTree.hasType (idBand))
            bands.push_back (bandFromTree (bandTree));
    return BandProfile (std::move (bands), getVolume());
}

std::optional<Band> CabinEqProfile::getBand (int id) const
{
    auto bandTree = tree.getChildWithName (idBands).getChildWithProperty (idId, id);
    if (bandTree.isValid())
        return bandFromTree (bandTree);
    return std::nullopt;
}

int CabinEqProfile::getNumBands() const
{
    return tree.getChildWithName (idBands).getNumChildren();
}

int CabinEqProfile::addBand (const Band& band)
{
    if (getNumBands() >= FilterChain::maxBands)
        return -1;

    auto newBand = band;
    newBand.id = getNextBandId();
    getBandsTree().appendChild (treeFromBand (newBand), undoManager);
    return newBand.id;
}

void CabinEqProfile::updateBand (const Band& band)
{
    auto bandTree = getBandsTree().getChildWithProperty (idId, band.id);
    if (! bandTree.isValid())
        return;

    // setProperty does nothing when the value's the same, so unchanged properties cost nothing
    bandTree.setProperty (idFreq, band.freq, undoManager);
    bandTree.setProperty (idAmpl, band.ampl, undoManager);
    bandTree.setProperty (idBandwidth, band.bandwidth, undoManager);
    bandTree.setProperty (idBandType, static_cast<int> (band.type), undoManager);
    bandTree.setProperty (idShape, static_cast<int> (band.shape), undoManager);
    bandTree.setProperty (idEnabled, band.enabled, undoManager);
}

void CabinEqProfile::removeBand (int id)
{
    auto bandsTree = getBandsTree();
    auto bandTree = bandsTree.getChildWithProperty (idId, id);
    if (bandTree.isValid())
        bandsTree.removeChild (bandTree, undoManager);
}

void CabinEqProfile::setBands (const std::vector<Band>& bands)
{
    auto bandsTree = getBandsTree();
    bandsTree.removeAllChildren (undoManager);

    int nextId = 0;
    for (auto band : bands)
    {
        if (nextId >= FilterChain::maxBands)
            break;
        band.id = nextId++;
        bandsTree.appendChild (treeFromBand (band), undoManager);
    }
}

void CabinEqProfile::setVolume (float volume)
{
    tree.setProperty (idProfileVolume, volume, undoManager);
}

void CabinEqProfile::renameTo (const juce::String& newName)
{
    tree.setProperty (idProfileName, newName, undoManager);
}

Band CabinEqProfile::bandFromTree (const juce::ValueTree& bandTree)
{
    Band band ((int) bandTree.getProperty (idId, 0),
               (float) bandTree.getProperty (idFreq, 1000.0f),
               (float) bandTree.getProperty (idAmpl, 0.0f),
               (float) bandTree.getProperty (idBandwidth, 1.0f),
               static_cast<Band::Type> (juce::jlimit (0, 2, (int) bandTree.getProperty (idBandType, 0))),
               static_cast<Band::Shape> (juce::jlimit (0, 4, (int) bandTree.getProperty (idShape, 0))),
               (bool) bandTree.getProperty (idEnabled, true));

    band.freq = juce::jlimit (Band::minFreq, Band::maxFreq, band.freq);
    band.ampl = juce::jlimit (Band::minGain, Band::maxGain, band.ampl);
    band.setQ (band.qFactor);
    return band;
}

juce::ValueTree CabinEqProfile::treeFromBand (const Band& band)
{
    juce::ValueTree bandTree (idBand);
    bandTree.setProperty (idId, band.id, nullptr);
    bandTree.setProperty (idFreq, band.freq, nullptr);
    bandTree.setProperty (idAmpl, band.ampl, nullptr);
    bandTree.setProperty (idBandwidth, band.bandwidth, nullptr);
    bandTree.setProperty (idBandType, static_cast<int> (band.type), nullptr);
    bandTree.setProperty (idShape, static_cast<int> (band.shape), nullptr);
    bandTree.setProperty (idEnabled, band.enabled, nullptr);
    return bandTree;
}

juce::ValueTree CabinEqProfile::getBandsTree()
{
    auto bandsTree = tree.getChildWithName (idBands);
    if (! bandsTree.isValid())
    {
        bandsTree = juce::ValueTree (idBands);
        tree.appendChild (bandsTree, undoManager);
    }
    return bandsTree;
}

int CabinEqProfile::getNextBandId() const
{
    int id = -1;
    for (const auto& bandTree : tree.getChildWithName (idBands))
        id = std::max ((int) bandTree.getProperty (idId, 0), id);
    return id + 1;
}
