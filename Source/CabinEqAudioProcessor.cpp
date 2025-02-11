/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "CabinEqAudioProcessor.h"
#include "CabinEqProcessorEditor.h"
#include <chrono>


//==============================================================================
CabinEqAudioProcessor::CabinEqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withInput  ("Input", juce::AudioChannelSet::mono(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::mono(), true)
                     #endif
                       ), parameters (*this, nullptr, "Params", createParameterLayout()),
                          cabinEqProfileManager (parameters)

#endif
{
    std::cout << "initialized processor" << std::endl;
}

CabinEqAudioProcessor::~CabinEqAudioProcessor()
{
}

//==============================================================================
const juce::String CabinEqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CabinEqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CabinEqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CabinEqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CabinEqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CabinEqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CabinEqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CabinEqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CabinEqAudioProcessor::getProgramName (int index)
{
    return {};
}

void CabinEqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CabinEqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    std::cout << "num input channels: " << getTotalNumInputChannels() << std::endl;
    spec.numChannels = getTotalNumInputChannels();
    playbackManager.prepare (spec);
    playbackManager.setGrid (noiseSequenceGrid);
//    playbackManager.setGlyph (getCurrGlyph());
}

void CabinEqAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CabinEqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CabinEqAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Clear buffer before handing it off to playbackManager
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    playbackManager.processBlock (buffer);
}

//==============================================================================
bool CabinEqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CabinEqAudioProcessor::createEditor()
{
    return new CabinEqProcessorEditor (*this);
}

//==============================================================================
void CabinEqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr <juce::XmlElement> xml (state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CabinEqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr <juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get())
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            cabinEqProfileManager.initProfiles();
            
            if (! hasLoadedData)
            {
                for (auto listener : listeners)
                    if (listener != nullptr)
                        listener->didLoadData();
                profileId = getLastSelectedProfileName().value_or ("NO_PROFILE");
                hasLoadedData = true;
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CabinEqAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout CabinEqAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::NormalisableRange<float> range (-1.0f, 1.0f, 0.01f);
    
    juce::String paramID = "dummyParam";
    return { std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (paramID, 1), paramID, range, 0.0f) };
}

//==============================================================================
void CabinEqAudioProcessor::freeTrialDidReset()
{
    cabinEqProfileManager.lockAllProfiles();
}

void CabinEqAudioProcessor::setVolume (float volume)
{
    playbackManager.setVolume (volume);
    updateFilter();
    setMasterVolume (volume);
}

void CabinEqAudioProcessor::setCalibrationVolume (float calibrationVolume)
{
    playbackManager.setCalibrationVolume (calibrationVolume);
    updateFilter();
}

void CabinEqAudioProcessor::setIsFilterOn (bool isFilterOn)
{
    playbackManager.setIsFilterOn (isFilterOn);
}

void CabinEqAudioProcessor::setIsPlaying (bool isPlaying)
{
    playbackManager.setIsPlayingNoise (isPlaying);
}

void CabinEqAudioProcessor::setIsCabinNoise (bool isCabinNoise)
{
    playbackManager.setIsCabinNoise (isCabinNoise);
}

void CabinEqAudioProcessor::setMinFreq (float newMinFreq)
{
    playbackManager.setMinFreq (newMinFreq);
}

void CabinEqAudioProcessor::setMaxFreq (float newMaxFreq)
{
    playbackManager.setMaxFreq (newMaxFreq);
}

void CabinEqAudioProcessor::setSpeedFactor (float speedFactor)
{
    playbackManager.setSpeedFactor (speedFactor);
}

void CabinEqAudioProcessor::setBandwidth (float bandwidth)
{
    playbackManager.setBandwidth (bandwidth);
}

void CabinEqAudioProcessor::setIIR (bool isIIR)
{
    playbackManager.setIIR (isIIR);
}

void CabinEqAudioProcessor::updateFIRFilter()
{
    playbackManager.updateFIRFilter();
}

void CabinEqAudioProcessor::setFIRQuality (int fftSize)
{
    playbackManager.setFIRQuality (fftSize);
}

void CabinEqAudioProcessor::setBarkScaling (bool barkScalingEnabled)
{
    playbackManager.setBarkScaling (barkScalingEnabled);
}

void CabinEqAudioProcessor::setERBScaling (bool erbScalingEnabled)
{
    playbackManager.setERBScaling (erbScalingEnabled);
}

void CabinEqAudioProcessor::setPinkNoise (bool pinkNoiseEnabled)
{
    playbackManager.setPinkNoise (pinkNoiseEnabled);
}

void CabinEqAudioProcessor::setIsCascading (bool isCascading)
{
    playbackManager.setIsCascading (isCascading);
    glyphManager.setIsCascading (isCascading);
}

void CabinEqAudioProcessor::setDensity (int density)
{
    playbackManager.setDensity (density);
    glyphManager.setDensity (density);
}

void CabinEqAudioProcessor::setStrokeOverlap (float strokeOverlap)
{
    playbackManager.setStrokeOverlap (strokeOverlap);
    glyphManager.setStrokeOverlap (strokeOverlap);
}

void CabinEqAudioProcessor::setDotOverlap (float dotOverlap)
{
    playbackManager.setDotOverlap (dotOverlap);
    glyphManager.setDotOverlap (dotOverlap);
}

void CabinEqAudioProcessor::setRampLength (float rampLength)
{
    playbackManager.setRampLength (rampLength);
    glyphManager.setRampLength (rampLength);
}

void CabinEqAudioProcessor::setCheckerboardResolution (int newResolution)
{
    checkerboardManager.setResolution (newResolution);
    playbackManager.setCheckerboard (checkerboardManager.getCheckerboard());
}

void CabinEqAudioProcessor::toggleCheckerboardPolarity()
{
    checkerboardManager.togglePolarity();
    playbackManager.setCheckerboard (checkerboardManager.getCheckerboard());
}

void CabinEqAudioProcessor::setProvisionalBands (std::vector<Band> provisionalBands)
{
    playbackManager.setProvisionalBands (provisionalBands);
}

void CabinEqAudioProcessor::setProvisionalBandsOn (bool provisionalBandsOn)
{
    playbackManager.setProvisionalBandsOn (provisionalBandsOn);
}

void CabinEqAudioProcessor::addProfile (juce::String profileName)
{
    cabinEqProfileManager.addProfile (profileName);
}

void CabinEqAudioProcessor::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    cabinEqProfileManager.addDuplicateProfile (profileName, oldProfileName);
}

void CabinEqAudioProcessor::removeProfile (juce::String profileName)
{
    cabinEqProfileManager.removeProfile (profileName);
}

void CabinEqAudioProcessor::renameProfile (juce::String profileName, juce::String newProfileName)
{
    cabinEqProfileManager.renameProfile (profileName, newProfileName);
}

void CabinEqAudioProcessor::setProfileVolume (float masterVolume)
{
    cabinEqProfileManager.setProfileVolume (profileId, masterVolume);
    updateFilter();
}

bool CabinEqAudioProcessor::isProfileLocked()
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
        return ! getHasLicense() && profile->get().getIsLocked();
    return ! getHasLicense();
}

const std::vector<juce::String> CabinEqAudioProcessor::getProfileNames() const
{
    return cabinEqProfileManager.getProfileNames();
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqAudioProcessor::getProfileNamed (juce::String profileName) const
{
    return cabinEqProfileManager.getProfileNamed (profileName);
}

BandProfile CabinEqAudioProcessor::getBandProfile()
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
        return profile->get().getBandProfile();
    return BandProfile ({}, 0.0f, 0.0f, 0.0f, false);
}

std::optional<juce::String> CabinEqAudioProcessor::getLastSelectedProfileName()
{
    return cabinEqProfileManager.getLastSelectedProfileName();
}

float CabinEqAudioProcessor::getMasterVolume()
{
    return cabinEqProfileManager.getMasterVolume();
}

bool CabinEqAudioProcessor::getHasLicense()
{
    return marketplaceStatus.isUnlocked();
//    return cabinEqProfileManager.getHasLicense();
}

void CabinEqAudioProcessor::setLastSelectedProfileName (juce::String profileName)
{
    cabinEqProfileManager.setLastSelectedProfileName (profileName);
    profileId = profileName;
}

void CabinEqAudioProcessor::setMasterVolume (float masterVolume)
{
    cabinEqProfileManager.setMasterVolume (masterVolume);
}

void CabinEqAudioProcessor::setHasLicense (bool hasLicense)
{
    cabinEqProfileManager.setHasLicense (hasLicense);
}

void CabinEqAudioProcessor::updateFilter()
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
        playbackManager.updateFilterWithBandProfile (profile->get().getBandProfile());
}

int CabinEqAudioProcessor::addMultiBandStep()
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
    {
        int stepId = profile->get().addMultiBandStep();
        return stepId;
    }
    return -1;
}

void CabinEqAudioProcessor::removeMultiBandStep (int stepId)
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
        profile->get().removeMultiBandStep (stepId);
}

void CabinEqAudioProcessor::setStepEnabled (int stepId, bool isEnabled)
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
        profile->get().setStepEnabled (stepId, isEnabled);
}

int CabinEqAudioProcessor::addBand (float freq, float ampl, float bandwidth, Band::Type type, int stepId)
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
    {
        int bandId = profile->get().addBand (freq, ampl, bandwidth, type, stepId);
        updateFilter();
        return bandId;
    }
        
    return -1;
}

void CabinEqAudioProcessor::updateBand (int bandId, float freq, float ampl, float bandwidth, Band::Type type, int stepId)
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
    {
        profile->get().updateBand (bandId, freq, ampl, bandwidth, type, stepId);
        updateFilter();
    }
}

void CabinEqAudioProcessor::removeBand (int bandId, int stepId)
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
    {
        profile->get().removeBand (bandId, stepId);
        updateFilter();
    }
}

void CabinEqAudioProcessor::addSequence (NoiseSequence sequence)
{
    noiseSequenceGrid.addSequence (sequence);
    playbackManager.setGrid (noiseSequenceGrid);
}

void CabinEqAudioProcessor::addSequenceWithCoords (std::vector<std::pair<int, int>> coords)
{
    int id = noiseSequenceGrid.getNextAvailableId();
    noiseSequenceGrid.addSequence (NoiseSequence (coords, id));
    playbackManager.setGrid (noiseSequenceGrid);
}

void CabinEqAudioProcessor::removeSequence (std::pair<int, int> origin)
{
    noiseSequenceGrid.removeSequence (origin);
    playbackManager.setGrid (noiseSequenceGrid);
}

void CabinEqAudioProcessor::moveSequence (int id, std::pair<int, int> newOrigin)
{
    noiseSequenceGrid.moveSequence (id, newOrigin);
    playbackManager.setGrid (noiseSequenceGrid);
}

void CabinEqAudioProcessor::toggleCoords (std::pair<int, int> point)
{
    noiseSequenceGrid.toggleCoords (point);
    playbackManager.setGrid (noiseSequenceGrid);
}

void CabinEqAudioProcessor::scaleUpGrid()
{
    noiseSequenceGrid.scaleUpGrid();
    playbackManager.setGrid (noiseSequenceGrid);
}

void CabinEqAudioProcessor::scaleDownGrid()
{
    noiseSequenceGrid.scaleDownGrid();
    playbackManager.setGrid (noiseSequenceGrid);
}

NoiseSequenceGrid CabinEqAudioProcessor::getNoiseGrid()
{
    return noiseSequenceGrid;
}

std::pair<int, int> CabinEqAudioProcessor::getNumRowsAndNumCols()
{
    return noiseSequenceGrid.getNumRowsAndNumCols();
}

int CabinEqAudioProcessor::getSequenceIdAtCoords (std::pair<int, int> coords)
{
    return noiseSequenceGrid.getSequenceIdAtCoords (coords);
}

float CabinEqAudioProcessor::getCurrTime()
{
    return playbackManager.getCurrPlayingTime();
}

//bool CabinEqAudioProcessor::getIsPlaying()
//{
//    return playbackManager.getIsPlaying();
//}

float CabinEqAudioProcessor::getBandwidth()
{
    return playbackManager.getBandwidth();
}

const Checkerboard CabinEqAudioProcessor::getCheckerboard()
{
    return checkerboardManager.getCheckerboard();
}

bool CabinEqAudioProcessor::getIsPlaying()
{
    return playbackManager.getIsPlaying();
}

CabinEqMarketplaceStatus& CabinEqAudioProcessor::getMarketplaceStatus()
{
    return marketplaceStatus;
}

std::vector<float> CabinEqAudioProcessor::getCurrPlayingFreqs()
{
    return playbackManager.getCurrPlayingFreqs();
}

std::vector<std::pair<float, float>> CabinEqAudioProcessor::getCurrPlayingFreqsAndVols()
{
    return playbackManager.getCurrPlayingFreqsAndVols();
}

void CabinEqAudioProcessor::addGlyph (ArchetypalGlyph archetype, juce::Point<float> centerPos, float sizeFactor)
{
    glyphManager.addGlyph (archetype, centerPos, sizeFactor);
    playbackManager.setGlyphs (glyphManager.getGlyphs());
}

void CabinEqAudioProcessor::moveGlyph (int glyphId, juce::Point<float> centerPos)
{
    glyphManager.moveGlyph (glyphId, centerPos);
    playbackManager.setGlyphs (glyphManager.getGlyphs());
}

void CabinEqAudioProcessor::removeGlyph (int glyphId)
{
    glyphManager.removeGlyph (glyphId);
    playbackManager.setGlyphs (glyphManager.getGlyphs());
}

void CabinEqAudioProcessor::incrementGlyphVolume (int glyphId, float increment)
{
    glyphManager.incrementGlyphVolume (glyphId, increment);
    playbackManager.setGlyphs (glyphManager.getGlyphs());
}

void CabinEqAudioProcessor::incrementSizeFactor (int glyphId, float horizontalIncrement, float verticalIncrement)
{
    glyphManager.incrementSizeFactor (glyphId, horizontalIncrement, verticalIncrement);
    playbackManager.setGlyphs (glyphManager.getGlyphs());
}

void CabinEqAudioProcessor::moveGlyphs (std::unordered_map<int, juce::Point<float>> idsToPositions)
{
    glyphManager.moveGlyphs (idsToPositions);
    playbackManager.setGlyphs (glyphManager.getGlyphs());
}

void CabinEqAudioProcessor::scaleGlyphs (std::unordered_set<int> glyphIds, float increment)
{
    glyphManager.scaleGlyphs (glyphIds, increment);
    playbackManager.setGlyphs (glyphManager.getGlyphs());
}

const std::vector<ArchetypalGlyph>& CabinEqAudioProcessor::getArchetypalGlyphs()
{
    return glyphManager.getArchetypalGlyphs();
}

const std::vector<Glyph>& CabinEqAudioProcessor::getGlyphs()
{
    return glyphManager.getGlyphs();
}

float CabinEqAudioProcessor::getCurrPlayingTime()
{
    return playbackManager.getCurrPlayingTime();
}

void CabinEqAudioProcessor::addListener (Listener* listener)
{
    this->listeners.push_back (listener);
}

void CabinEqAudioProcessor::removeListener()
{
    // VERY BAD FIX THIS: eh whatever 
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqAudioProcessor::profileNamed (juce::String profileName) const
{
    return cabinEqProfileManager.getProfileNamed (profileName);
}
