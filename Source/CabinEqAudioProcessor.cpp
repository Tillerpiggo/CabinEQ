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
                          cabinEqValueTreeManager (parameters)

#endif
{
    
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
    spec.numChannels = getTotalNumInputChannels();
    playbackManager.prepare (spec);
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
            cabinEqValueTreeManager.initProfiles();
            
            if (! hasLoadedData)
            {
                for (auto listener : listeners)
                    if (listener != nullptr)
                        listener->didLoadData();
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
void CabinEqAudioProcessor::applyCurve (int fftSize, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.updateFilterWithCurves (profile->get().getLeftAmplCurve(),
                                                profile->get().getRightAmplCurve(),
                                                fftSize);
    }
        
}

void CabinEqAudioProcessor::setIsProcessing (bool isProcessing)
{
    playbackManager.setIsProcessing (isProcessing);
}

void CabinEqAudioProcessor::setBypassBalance (float balance)
{
    playbackManager.setDryWetVolumeBalance (balance);
}

void CabinEqAudioProcessor::setWetVolume (float wetVolume)
{
    playbackManager.setWetVolume (wetVolume);
}

void CabinEqAudioProcessor::setDryVolume (float dryVolume)
{
    playbackManager.setDryVolume (dryVolume);
}

std::optional<std::reference_wrapper<Curve>> CabinEqAudioProcessor::getLeftAmplCurve (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getLeftAmplCurve();
    std::cout << "unable to get ampl curve in pluginProcessor for profile named " << profileName << std::endl;
    return std::nullopt;
}

std::optional<std::reference_wrapper<Curve>> CabinEqAudioProcessor::getRightAmplCurve (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getRightAmplCurve();
    std::cout << "unable to get pan curve in pluginProcessor for profile named " << profileName << std::endl;
    return std::nullopt;
}

const std::vector<CurvePt> CabinEqAudioProcessor::getLeftAmplPts (juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getLeftAmplPts();
    return {};
}

const std::vector<CurvePt> CabinEqAudioProcessor::getRightAmplPts (juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getRightAmplPts();
    return {};
}

const std::optional<CurvePt> CabinEqAudioProcessor::getLeftAmplPtWithId (int id, juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getLeftAmplPtWithId (id);
    return std::nullopt;
}

const std::optional<CurvePt> CabinEqAudioProcessor::getRightAmplPtWithId (int id, juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getRightAmplPtWithId (id);
    return std::nullopt;
}

int CabinEqAudioProcessor::addLeftAmplPt (const float freq, const float ampl, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().addLeftAmplPt (freq, ampl);
    return -1;
}

int CabinEqAudioProcessor::addRightAmplPt (const float freq, const float pan, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().addRightAmplPt (freq, pan);
    return -1;
}

void CabinEqAudioProcessor::removeLeftAmplPt (const int id, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().removeLeftAmplPt (id);
}

void CabinEqAudioProcessor::removeRightAmplPt (const int id, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().removeRightAmplPt (id);
}

void CabinEqAudioProcessor::updateLeftAmplPt (const int id, const float freq, const float ampl, juce::String profileName)
{
    currProfileName = profileName; // super hacky
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().updateLeftAmplPt (id, freq, ampl);
}

void CabinEqAudioProcessor::updateRightAmplPt (const int id, const float freq, const float pan, juce::String profileName)
{
    currProfileName = profileName; // super hacky
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().updateRightAmplPt (id, freq, pan);
}

void CabinEqAudioProcessor::clearEQNodes (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().resetNodes();
}

void CabinEqAudioProcessor::startLeftAmplCalibration (float freq, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.startLeftAmplCalibration (freq,
                                          profile->get().getLeftAmplCurve());
        playbackManager.setIsCalibrating (true);
    }
}

void CabinEqAudioProcessor::updateLeftAmplCalibration (float freq, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.updateLeftAmplCalibration (freq,
                                          profile->get().getLeftAmplCurve());
        playbackManager.setIsCalibrating (true);
    }
}

void CabinEqAudioProcessor::startRightAmplCalibration (float freq, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.startRightAmplCalibration (freq,
                                          profile->get().getRightAmplCurve());
        playbackManager.setIsCalibrating (true);
    }
}

void CabinEqAudioProcessor::updateRightAmplCalibration (float freq, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.updateRightAmplCalibration (freq,
                                          profile->get().getRightAmplCurve());
        playbackManager.setIsCalibrating (true);
    }
}

void CabinEqAudioProcessor::stopCalibration()
{
    playbackManager.setIsCalibrating (false);
}

float CabinEqAudioProcessor::getCurrPlayingFreq()
{
    return playbackManager.getCurrPlayingFreq();
}

void CabinEqAudioProcessor::startTestingAt (float freq, juce::String profileName)
{
//    auto curve = getAmplCurve (profileName); // hacky for now
//    if (curve.has_value())
//        playbackManager.startTestingFreq (freq, curve->get());
}

void CabinEqAudioProcessor::updateTestingAt (float freq, juce::String profileName)
{
//    auto curve = getAmplCurve (profileName); // hacky for now
//    if (curve.has_value())
//        playbackManager.updateTestingFreq (freq, curve->get());
}

void CabinEqAudioProcessor::endTesting()
{
    playbackManager.stopTestingFreq();
}

float CabinEqAudioProcessor::getCurrTestingFreq()
{
    return playbackManager.getCurrTestingFreq();
}

void CabinEqAudioProcessor::addProfile (juce::String profileName)
{
    cabinEqValueTreeManager.addProfile (profileName);
}

void CabinEqAudioProcessor::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    cabinEqValueTreeManager.addDuplicateProfile (profileName, oldProfileName);
}

void CabinEqAudioProcessor::removeProfile (juce::String profileName)
{
    cabinEqValueTreeManager.removeProfile (profileName);
}

const std::vector<juce::String> CabinEqAudioProcessor::getProfileNames() const
{
    return cabinEqValueTreeManager.getProfileNames();
}

std::optional<std::reference_wrapper<CabinEqValueTree>> CabinEqAudioProcessor::getProfileNamed (juce::String profileName) const
{
    return cabinEqValueTreeManager.getProfileNamed (profileName);
}

std::optional<juce::String> CabinEqAudioProcessor::getLastSelectedProfileName()
{
    return cabinEqValueTreeManager.getLastSelectedProfileName();
}

void CabinEqAudioProcessor::setLastSelectedProfileName (juce::String profileName)
{
    cabinEqValueTreeManager.setLastSelectedProfileName (profileName);
}

void CabinEqAudioProcessor::startSineSweep (float centerFreq, juce::String profileName)
{
//    auto profile = profileNamed (profileName);
//    if (profile.has_value())
//    {
//        playbackManager.setIsSweeping (true);
//        playbackManager.startSineSweep (centerFreq,
//                                        profile->get().getAmplCurve(),
//                                        profile->get().getPanCurve());
//    }
}

void CabinEqAudioProcessor::updateSineSweep (float centerFreq, juce::String profileName)
{
//    auto profile = profileNamed (profileName);
//    if (profile.has_value())
//    {
//        playbackManager.setIsSweeping (true);
//        playbackManager.updateSineSweep (centerFreq,
//                                         profile->get().getAmplCurve(),
//                                         profile->get().getPanCurve());
//    }
}

void CabinEqAudioProcessor::endSineSweep()
{
    playbackManager.setIsSweeping (false);
}

float CabinEqAudioProcessor::getCurrSineSweepFreq()
{
    return playbackManager.getCurrSineSweepFreq();
}

void CabinEqAudioProcessor::setReferenceVolume (float volume)
{
    playbackManager.setReferenceVolume (volume);
}

void CabinEqAudioProcessor::setReferencePan (float pan)
{
    playbackManager.setReferencePan (pan);
}

void CabinEqAudioProcessor::setReferenceVolume1 (float volume)
{
    playbackManager.setReferenceVolume1 (volume);
}

void CabinEqAudioProcessor::setReferenceVolume2 (float volume)
{
    playbackManager.setReferenceVolume2 (volume);
}

void CabinEqAudioProcessor::addListener (Listener* listener)
{
    this->listeners.push_back (listener);
}

void CabinEqAudioProcessor::removeListener()
{
    // VERY BAD FIX THIS: eh whatever
}

std::optional<std::reference_wrapper<CabinEqValueTree>> CabinEqAudioProcessor::profileNamed (juce::String profileName) const
{
    return cabinEqValueTreeManager.getProfileNamed (profileName);
}
