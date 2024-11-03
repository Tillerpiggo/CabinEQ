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
//    // Step I - Scaling Melody
    IntelligibilityStep step1;
    MelodicNotes wideScale = MelodicNotes ({ -12, 0, 5, 7, 9, 7, 11, 14 }, 800.0f);
    step1.addStage (wideScale.withTransposition (-48));
    step1.addStage (wideScale.withTransposition (-24));
    step1.addStage (wideScale.withTransposition (0));
    step1.addStage (wideScale.withTransposition (24));
    step1.addStage (wideScale.withTransposition (48));
    
    // Step II - Three Stack
    SpatialStep step2;
    MelodicNotes threeStack =
    MelodicNotes::withFreqs ({ 200, 1000, 5000 })
        .withBandwidth (2.0f);
    step2.addStage (threeStack);
    step2.addStage (threeStack.withPan (-1));
    step2.addStage (threeStack.withPan (1));
    
    // Step III - Solfeggietto
    IntelligibilityStep step3;
    MelodicNotes solfeggietto =
    MelodicNotes ({ 0, -3, 0, 4, 9, 12, 11, 9, 8, 4, 8, 11, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 23, 21, 20, 18, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 20, 16, 20, 23, 28, 26, 24, 23, 24, 21, 24, 28, 33, 36, 35, 33, 35, 33, 32, 30, 28, 26, 24, 23, 24, 21, 16, 12, 9, 33, 28, 24, 29, 2, 5, 9, 14, 17, 21, 24, 23, 19, 14, 11, 7, 31, 26, 23, 28, 0, 4, 7, 12, 16, 19, 23, 21, 18, 17, 18, 21, 18, 17, 18, 24, 21, 16, 18, 24, 21, 16, 18, 23, 21, 15, 18, 30, 21, 15, 18, 27, 21, 11, 18, 21, 18, 15, 11, 19, -8, -5, -1, 4, 7, 6, 4, 3, -1, 3, 6, 11, 9, 7, 6, 7, 4, 7, 11, 16, 19, 18, 16, 18, 16, 15, 13, 11, 9, 7, 6, 7, 4, 7, 11, 16, 19, 18, 16, 15, 11, 15, 18, 23, 21, 19, 18, 19, 16, 19, 23, 28, 31, 30, 28, 30, 28, 27, 25, 23, 21, 19, 18, 19, 4, -8, 16, 19, 23, 28, 23, 19, 16, 2, -10, 28, 23, 20, 16, 20, 23, 28, 21, 12, 16, 28, 16, 21, 12, 16, 28, 16, 20, 11, 16, 26, 16, 20, 11, 16, 26, 16, 24, 9, -3, 21, 24, 28, 33, 28, 24, 21, 7, -5, 33, 28, 25, 21, 25, 28, 33, 26, 17, 21, 33, 21, 26, 17, 21, 33, 21, 25, 16, 21, 31, 21, 25, 16, 21, 31, 21, 29, -10, -7, -3, 2, 5, 4, 2, 1, -3, 1, 4, 9, 7, 5, 4, 5, 2, 5, 9, 14, 17, 16, 14, 16, 14, 13, 11, 9, 7, 5, 4, 5, 2, 5, 9, 14, 17, 16, 14, 13, 9, 13, 16, 21, 19, 17, 16, 17, 14, 17, 21, 26, 29, 28, 26, 28, 26, 25, 23, 21, 19, 17, 16, 17, 17, 26, 21, 17, 14, 14, 21, 17, 14, 9, 9, 17, 14, 9, 5, 5, 14, 9, 5, -2, -14, 29, 26, 25, 26, 28, 26, 25, 26, -3, -15, 17, 14, 13, 14, 16, 14, 13, 14, -4, -16, 35, 26, 28, 29, 28, 26, 24, 23, 24, -3, -15, 28, 33, 28, 31, 2, 29, 28, 26, 24, 4, -8, 23, 24, 23, 21, 23, 21, 12, 16, 28, 16, 21, 12, 16, 28, 16, 20, 11, 16, 26, 16, 20, 11, 16, 26, 16, 19, 9, 16, 25, 16, 19, 9, 16, 25, 16, 18, 14, 24, 33, 24, 18, 14, 24, 33, 24, 17, 7, 14, 23, 14, 17, 7, 14, 23, 14, 16, 12, 22, 31, 22, 16, 12, 22, 31, 22, 15, 5, 12, 21, 12, 15, 5, 12, 21, 12, 12, 3, 21, 33, 21, 12, 3, 21, 33, 21, 12, 4, 21, 24, 28, 33, 28, 24, 21, 28, 24, 21, 16, 26, -8, 23, 20, 14, 12, -3, 0, 4, 9, 12, 11, 9, 8, 4, 8, 11, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 23, 21, 20, 18, 16, 14, 12, 11, 12, 9, 12, 16, 21, 24, 23, 21, 20, 16, 20, 23, 28, 26, 24, 23, 24, 21, 24, 28, 33, 36, 35, 32, 33, 28, 24, 23, 21, 16, 12, 11, 9}, 800.0f);
    step3.addStage (solfeggietto.withTranspositionInOctaves (-2));
    step3.addStage (solfeggietto.withTranspositionInOctaves (0));
    step3.addStage (solfeggietto.withTranspositionInOctaves (2));
    
    // Step IV - fancy pattern
    SpatialStep step4;
    float lowFreq = 20;
    float hiFreq = 15000;
    float sampleRate = 44100; // for now, but this isn't ideal
    SweepPattern forwardSlash ({{ lowFreq, -1 }, { hiFreq, 1 }}, 2.0f, sampleRate);
    SweepPattern downwardsRight ({{ hiFreq, 1 }, { lowFreq, 1 }}, 2.0f, sampleRate);
    SweepPattern backslash ({{ lowFreq, 1 }, { hiFreq, -1 }}, 2.0f, sampleRate);
    SweepPattern downwardsLeft ({{ hiFreq, -1 }, { lowFreq, -1 }}, 2.0f, sampleRate);
    step4.addStage (forwardSlash);
    step4.addStage (downwardsRight);
    step4.addStage  (backslash);
    step4.addStage (downwardsLeft);
    
    // Initialize QualityStepManager steps imperatively
    qualityStepManager.addIntelligibilityStep (step1);
    qualityStepManager.addSpatialStep (step2);
    qualityStepManager.addIntelligibilityStep (step3);
    qualityStepManager.addSpatialStep (step4);
    
    playbackManager.setQualityStep (qualityStepManager.getCurrStep());
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
            cabinEqProfileManager.initProfiles();
            
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
void CabinEqAudioProcessor::setVolume (float volume)
{
    playbackManager.setVolume (volume);
}

void CabinEqAudioProcessor::setIsProcessing (bool isProcessing)
{
    playbackManager.setIsProcessing (isProcessing);
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

void CabinEqAudioProcessor::setProfileVolume (juce::String profileName, float masterVolume)
{
    cabinEqProfileManager.setProfileVolume (profileName, masterVolume);
}

const std::vector<juce::String> CabinEqAudioProcessor::getProfileNames() const
{
    return cabinEqProfileManager.getProfileNames();
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqAudioProcessor::getProfileNamed (juce::String profileName) const
{
    return cabinEqProfileManager.getProfileNamed (profileName);
}

BandProfile CabinEqAudioProcessor::getBandProfile (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getBandProfile();
    return BandProfile ({}, 0.0f, 0.0f, 0.0f);
}

std::optional<float> CabinEqAudioProcessor::getCurrPlayingFreq()
{
    return playbackManager.getCurrPlayingFreq();
}

std::optional<juce::String> CabinEqAudioProcessor::getLastSelectedProfileName()
{
    return cabinEqProfileManager.getLastSelectedProfileName();
}

void CabinEqAudioProcessor::setLastSelectedProfileName (juce::String profileName)
{
    cabinEqProfileManager.setLastSelectedProfileName (profileName);
}

void CabinEqAudioProcessor::updateFilter (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        playbackManager.updateFilterWithBandProfile (profile->get().getBandProfile());
}

int CabinEqAudioProcessor::addBand (const float freq, const float ampl, const float bandwidth, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        return profile->get().addBand (freq, ampl, bandwidth);
    }
        
    return -1;
}

void CabinEqAudioProcessor::updateBand (const int id, const float freq, const float ampl, const float bandwidth, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().updateBand (id, freq, ampl, bandwidth);
}

void CabinEqAudioProcessor::removeBand (const int id, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().removeBand (id);
}



void CabinEqAudioProcessor::addListener (Listener* listener)
{
    this->listeners.push_back (listener);
}

void CabinEqAudioProcessor::removeListener()
{
    // VERY BAD FIX THIS: eh whatever
}

void CabinEqAudioProcessor::setDifficulty (float difficulty)
{
    playbackManager.setDifficulty (difficulty);
}

void CabinEqAudioProcessor::setOctaveShift (float octaveShift)
{
    playbackManager.setOctaveShift (octaveShift);
}

void CabinEqAudioProcessor::setIsPlaying (bool isPlaying)
{
    playbackManager.setIsCalibrating (isPlaying);
}

void CabinEqAudioProcessor::setIsCycling (bool isCycling)
{
    playbackManager.setIsCycling (isCycling);
}

QualityStep CabinEqAudioProcessor::getCurrStep()
{
    return qualityStepManager.getCurrStep();
}

QualityStep CabinEqAudioProcessor::goToPrevStep()
{
    qualityStepManager.goToPrevStep();
    QualityStep qualityStep = qualityStepManager.getCurrStep();
    playbackManager.setQualityStep (qualityStep);
    return qualityStep;
}

QualityStep CabinEqAudioProcessor::goToNextStep()
{
    qualityStepManager.goToNextStep();
    QualityStep qualityStep = qualityStepManager.getCurrStep();
    playbackManager.setQualityStep (qualityStep);
    return qualityStep;
}

int CabinEqAudioProcessor::getCurrStage()
{
    return playbackManager.getCurrStage();
}

void CabinEqAudioProcessor::setStage (int stageIdx)
{
    playbackManager.setStage (stageIdx);
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqAudioProcessor::profileNamed (juce::String profileName) const
{
    return cabinEqProfileManager.getProfileNamed (profileName);
}
