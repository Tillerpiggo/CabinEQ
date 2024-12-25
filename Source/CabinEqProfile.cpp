/*
  ==============================================================================

    CabinEqProfile.cpp
    Created: 10 Oct 2024 7:51:38pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqProfile.h"

CabinEqProfile::CabinEqProfile (juce::AudioProcessorValueTreeState& apvts, const juce::String identifier)
    : apvts (apvts), profileName (identifier)
{
    initValueTreeFromAPVTS();
}

const BandProfile CabinEqProfile::getBandProfile() const
{
    std::vector<Band> bands;
    
    // Compile the bands from all multi band steps
    if (valueTree.isValid())
    {
        auto amplTree = valueTree.getChildWithName (idAmplTree);
        for (const auto& multiBandStep : amplTree)
        {
            auto newBands = getBandsForMultiBandStep (multiBandStep);
            bands.insert (bands.begin(), newBands.begin(), newBands.end()); // append all of newBands to bands
        }
    }
    
    return BandProfile (bands, profileVolume, melodyVolume, noiseVolume);
}

const std::optional<Band> CabinEqProfile::getBandWithId (const int id) const
{
    if (! valueTree.isValid())
        return std::nullopt;
    
    auto amplBandTree = valueTree.getChildWithName (idAmplTree);
    if (! amplBandTree.isValid())
        return std::nullopt;
    
    auto amplBand = amplBandTree.getChildWithProperty (idId, id);
    if (amplBand.isValid())
    {
        return Band (amplBand.getProperty (idId),
                     amplBand.getProperty (idFreq),
                     amplBand.getProperty (idAmpl),
                     amplBand.getProperty (idBandwidth),
                     static_cast<Band::Type> ((int) amplBand.getProperty (idBandType)));
    }
    else
    {
        return std::nullopt;
    }
}

int CabinEqProfile::addMultiBandStep (const bool isEnabled)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplBandTree = valueTree.getChildWithName (idAmplTree);
    int id = getNextIdInValueTree (amplBandTree);
    
    // Create the multiband step
    juce::ValueTree step (idMultiBandStep);
    step.setProperty (idId, id, nullptr);
    step.setProperty (idEnabled, isEnabled, nullptr);
    amplBandTree.appendChild (step, nullptr);
    
    return id;
}

void CabinEqProfile::removeMultiBandStep (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplBandTree = valueTree.getChildWithName (idAmplTree);
    juce::ValueTree nodeToRemove = amplBandTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        amplBandTree.removeChild (nodeToRemove, nullptr);
}

int CabinEqProfile::addBand (const float freq, const float ampl, const float bandwidth, const Band::Type type, const int stepId)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplBandTree = valueTree.getChildWithName (idAmplTree);
    auto multiBandStep = amplBandTree.getChildWithProperty (idId, stepId);
    
    if (! multiBandStep.isValid())
    {
        std::cerr << "Couldn't find multiband step with id " << stepId << " in addBand" << std::endl;
        return -1;
    }
    
    int id = getNextIdInValueTree (multiBandStep);
    addBandToMultiBandStep (id, freq, ampl, bandwidth, type, multiBandStep);
    
    return id;
}

void CabinEqProfile::removeBand (const int id, const int stepId)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplBandTree = valueTree.getChildWithName (idAmplTree);
    auto multiBandStep = amplBandTree.getChildWithProperty (idId, stepId);
    
    if (! multiBandStep.isValid())
    {
        std::cerr << "Couldn't find multiband step with id " << stepId << " in removeBand" << std::endl;
        return;
    }
    
    juce::ValueTree nodeToRemove = amplBandTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        amplBandTree.removeChild (nodeToRemove, nullptr);
}

void CabinEqProfile::updateBand (const int id, const float freq, const float ampl, const float bandwidth, const Band::Type type, const int stepId)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplBandTree = valueTree.getChildWithName (idAmplTree);
    auto multiBandStep = amplBandTree.getChildWithProperty (idId, stepId);
    
    if (! multiBandStep.isValid())
    {
        std::cerr << "Couldn't find multi-band step with id in updateBand" << std::endl;
        return;
    }
    
    updateBandInMultiBandStep (id, freq, ampl, bandwidth, type, multiBandStep);
}

void CabinEqProfile::initValueTreeFromAPVTS()
{
    valueTree = apvts.state.getChildWithProperty (idProfileName, profileName);
    
    // Initialize value tree if we can't load it
    if (! valueTree.isValid())
    {
        valueTree = juce::ValueTree (idProfile);
        valueTree.setProperty (idProfileName, profileName, nullptr);
        valueTree.setProperty (idProfileVolume, 0.0f, nullptr);
        valueTree.setProperty (idMelodyVolume, 0.0f, nullptr);
        valueTree.setProperty (idNoiseVolume, 0.0f, nullptr);
        
        auto amplBandTree = juce::ValueTree (idAmplTree);
        valueTree.addChild (amplBandTree, 0, nullptr);
        
        auto multiBandStep = juce::ValueTree (idMultiBandStep);
        multiBandStep.setProperty (idId, 0, nullptr);
        multiBandStep.setProperty (idEnabled, true, nullptr);
        amplBandTree.addChild (multiBandStep, 0, nullptr);
        
        apvts.state.addChild (valueTree, -1, nullptr);
    }
    else
    {
        profileName = valueTree.getProperty (idProfileName);
        profileVolume = valueTree.getProperty (idProfileVolume);
        melodyVolume = valueTree.getProperty (idMelodyVolume);
        noiseVolume = valueTree.getProperty (idNoiseVolume);
    }
    
    hasBeenInitialized = true;
}

const juce::String CabinEqProfile::getName() const
{
    return profileName;
}

const float CabinEqProfile::getVolume() const
{
    return profileVolume;
}

const float CabinEqProfile::getMelodyVolume() const
{
    return melodyVolume;
}

const float CabinEqProfile::getNoiseVolume() const
{
    return noiseVolume;
}

void CabinEqProfile::copyFrom (CabinEqProfile other)
{
    initValueTreeFromAPVTS();
    juce::String newProfileName = valueTree.getProperty (idProfileName);
    valueTree.copyPropertiesAndChildrenFrom (other.valueTree, nullptr);
    valueTree.setProperty (idProfileName, newProfileName, nullptr);
    
    // Update local variables
    profileVolume = valueTree.getProperty (idProfileVolume);
    profileName = valueTree.getProperty (idProfileName);
}

void CabinEqProfile::renameTo (juce::String newName)
{
    profileName = newName;
    valueTree.setProperty (idProfileName, newName, nullptr);
}

void CabinEqProfile::setVolume (float profileVolume)
{
    this->profileVolume = profileVolume;
    valueTree.setProperty (idProfileVolume, profileVolume, nullptr);
}

void CabinEqProfile::setMelodyVolume (float melodyVolume)
{
    this->melodyVolume = melodyVolume;
    valueTree.setProperty (idMelodyVolume, melodyVolume, nullptr);
}

void CabinEqProfile::setNoiseVolume (float noiseVolume)
{
    this->noiseVolume = noiseVolume;
    valueTree.setProperty (idNoiseVolume, noiseVolume, nullptr);
}

void CabinEqProfile::addBandToMultiBandStep (int id, float freq, float ampl, float bandwidth, Band::Type type, juce::ValueTree multiBandStep)
{
    juce::ValueTree band (idBand);
    band.setProperty (idId, id, nullptr);
    band.setProperty (idFreq, freq, nullptr);
    band.setProperty (idAmpl, ampl, nullptr);
    band.setProperty (idBandwidth, bandwidth, nullptr);
    band.setProperty (idBandType, static_cast<int> (type), nullptr);
    multiBandStep.appendChild (band, nullptr);
}

void CabinEqProfile::updateBandInMultiBandStep (int id, float freq, float ampl, float bandwidth, Band::Type type, juce::ValueTree multiBandStep)
{
    juce::ValueTree bandToModify = multiBandStep.getChildWithProperty (idId, id);
    if (bandToModify.isValid())
    {
        bandToModify.setProperty (idFreq, freq, nullptr);
        bandToModify.setProperty (idAmpl, ampl, nullptr);
        bandToModify.setProperty (idBandwidth, bandwidth, nullptr);
        bandToModify.setProperty (idBandType, static_cast<int> (type), nullptr);
    }
}

int CabinEqProfile::getNextIdInValueTree (juce::ValueTree valueTree)
{
    // Assume tree is valid; otherwise this should crash
    int id = -1;
    for (const auto& identifiableNode : valueTree)
    {
        id = std::max ((int) identifiableNode.getProperty (idId), id);
    }
    id++;
    return id;
}

void CabinEqProfile::printMultiBandStep (juce::ValueTree multiBandStep) const
{
    if (multiBandStep.isValid())
    {
        std::cout << "MultiBandStep (numNodes: " << multiBandStep.getNumChildren() << ")" << std::endl;
        if (multiBandStep.getNumChildren() > 0)
        {
            for (const auto& band : multiBandStep)
            {
                float id = band.getProperty (idId);
                float freq = band.getProperty (idFreq);
                float ampl = band.getProperty (idAmpl);
                float bandwidth = band.getProperty (idBandwidth);
                int type = static_cast<int> (band.getProperty (idBandType));
                
                std::cout << "Band (id: " << id << ", freq: " << freq << ", ampl: " << ampl << ", bandwidth: " << bandwidth << ", type: " << type << ")" << std::endl;
            }
        }
    }
    else
    {
        std::cout << "NO_TREE" << std::endl;
    }
    
    std::cout << std::endl;
}

std::vector<Band> CabinEqProfile::getBandsForMultiBandStep (juce::ValueTree multiBandStep) const
{
    std::vector<Band> bands;
    if (multiBandStep.isValid())
    {
        for (const auto& band : multiBandStep)
        {
            int id = band.getProperty (idId);
            float freq = band.getProperty (idFreq);
            float ampl = band.getProperty (idAmpl);
            float bandwidth = band.getProperty (idBandwidth);
            Band::Type type = static_cast<Band::Type> ((int) band.getProperty (idBandType));
            bands.emplace_back (id, freq, ampl, bandwidth, type);
        }
    }
    return bands;
}
