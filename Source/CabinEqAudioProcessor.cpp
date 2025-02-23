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
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters (*this, nullptr, "Params", createParameterLayout()),
                          cabinEqProfileManager (parameters)

#endif
{
    std::cout << "initialized processor" << std::endl;
    startTimer (5000); // autosave every 5 seconds if data has changed
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
    dataHasChanged = true;
}

void CabinEqAudioProcessor::setIsAudioFilePlaying (bool isPlaying)
{
    playbackManager.setIsAudioFilePlaying (isPlaying);
}

void CabinEqAudioProcessor::setFile (juce::File file)
{
    playbackManager.setAudioFile (file);
}

void CabinEqAudioProcessor::addAsListener (PlaybackManagerListener* listener)
{
    playbackManager.setListener (listener);
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

void CabinEqAudioProcessor::setPinkNoise (bool pinkNoiseEnabled)
{
    playbackManager.setPinkNoise (pinkNoiseEnabled);
}

void CabinEqAudioProcessor::goToNext()
{
    checkerboardManager.goToNext();
    playbackManager.setCheckerboard (checkerboardManager.getCurrCheckerboard());
}

void CabinEqAudioProcessor::goToPrev()
{
    checkerboardManager.goToPrev();
    playbackManager.setCheckerboard (checkerboardManager.getCurrCheckerboard());
}

bool CabinEqAudioProcessor::hasNext()
{
    return checkerboardManager.hasNext();
}

bool CabinEqAudioProcessor::hasPrev()
{
    return checkerboardManager.hasPrev();
}

void CabinEqAudioProcessor::toggleCheckerboardPolarity()
{
    checkerboardManager.togglePolarity();
    playbackManager.setCheckerboard (checkerboardManager.getCurrCheckerboard());
}

void CabinEqAudioProcessor::selectCheckerboardAtIdx (int idx)
{
    checkerboardManager.selectIdx (idx);
    playbackManager.setCheckerboard (checkerboardManager.getCurrCheckerboard());
}

void CabinEqAudioProcessor::setSoloSquareCoords (std::set<std::pair<int, int>> soloSquareCoords)
{
    playbackManager.setSoloSquareCoords (soloSquareCoords);
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
    dataHasChanged = true;
}

void CabinEqAudioProcessor::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    cabinEqProfileManager.addDuplicateProfile (profileName, oldProfileName);
    dataHasChanged = true;
}

void CabinEqAudioProcessor::removeProfile (juce::String profileName)
{
    cabinEqProfileManager.removeProfile (profileName);
    dataHasChanged = true;
}

void CabinEqAudioProcessor::renameProfile (juce::String profileName, juce::String newProfileName)
{
    cabinEqProfileManager.renameProfile (profileName, newProfileName);
    dataHasChanged = true;
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
    dataHasChanged = true;
}

void CabinEqAudioProcessor::setMasterVolume (float masterVolume)
{
    cabinEqProfileManager.setMasterVolume (masterVolume);
}

void CabinEqAudioProcessor::setHasLicense (bool hasLicense)
{
    cabinEqProfileManager.setHasLicense (hasLicense);
    dataHasChanged = true;
}

void CabinEqAudioProcessor::updateFilter()
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
        playbackManager.updateFilterWithBandProfile (profile->get().getBandProfile());
    dataHasChanged = true;
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

float CabinEqAudioProcessor::getBandwidth()
{
    return playbackManager.getBandwidth();
}

const Checkerboard CabinEqAudioProcessor::getCheckerboard()
{
    return checkerboardManager.getCurrCheckerboard();
}

bool CabinEqAudioProcessor::getIsPlaying()
{
    return playbackManager.getIsPlaying();
}

int CabinEqAudioProcessor::getNumCheckerboards()
{
    return checkerboardManager.getNumCheckerboards();
}

std::string CabinEqAudioProcessor::getNameAtIdx (int idx)
{
    return checkerboardManager.getNameAtIdx (idx);
}

int CabinEqAudioProcessor::getSelectedRow()
{
    return checkerboardManager.getSelectedRow();
}

std::vector<juce::String> CabinEqAudioProcessor::getProfileNames()
{
    return cabinEqProfileManager.getProfileNames();
}

bool CabinEqAudioProcessor::getIsProfileLocked (int rowIdx)
{
    return false; // for now
}


void CabinEqAudioProcessor::saveData()
{
    juce::StandalonePluginHolder::getInstance()->savePluginState();
}

void CabinEqAudioProcessor::restartAudio()
{
    juce::StandalonePluginHolder::getInstance()->restartAudio();
}

void CabinEqAudioProcessor::showAudioSettingsDialog()
{
    juce::StandalonePluginHolder::getInstance()->showAudioSettingsDialog();
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

void CabinEqAudioProcessor::addListener (Listener* listener)
{
    this->listeners.push_back (listener);
}

void CabinEqAudioProcessor::removeListener()
{
    // VERY BAD FIX THIS: eh whatever 
}

void CabinEqAudioProcessor::timerCallback()
{
    if (dataHasChanged)
    {
        saveData();
        dataHasChanged = false;
    }
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqAudioProcessor::profileNamed (juce::String profileName) const
{
    return cabinEqProfileManager.getProfileNamed (profileName);
}
