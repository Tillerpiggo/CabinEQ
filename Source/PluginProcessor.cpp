/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "InverseFletcherMunsonCurve.h"

//==============================================================================
StartupMVPAudioProcessor::StartupMVPAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters (*this, nullptr, "Parameters", createParameterLayout (SetPointManager::NUM_SET_POINTS))

#endif
{
    isSlidingGainSlider = false;
    isSlidingPanSlider = false;
    isSlidingPhaseSlider = false;
    
    parameters.state = juce::ValueTree("savedParams");
    //stdgainProcessor.setGainDecibels(-47.6f); // Note: was 60. We should make the sine waves this much quieter.
    gainProcessor.setGainDecibels(0.f);
    
//    // Test code:
//    std::vector<float> f = {
//        20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500,
//        630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000,
//        10000, 12500
//    };
//    
//    InverseFletcherMunsonCurve c;
//    
//    std::cout << "Fletcher munson curve" << std::endl;
//    for (float freq : f)
//    {
//        std::cout << "Val at " << freq << ": " << c.valueAtFrequency (freq) << std::endl;
//    }
}

StartupMVPAudioProcessor::~StartupMVPAudioProcessor()
{
}

//==============================================================================
const juce::String StartupMVPAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StartupMVPAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StartupMVPAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StartupMVPAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StartupMVPAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StartupMVPAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StartupMVPAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StartupMVPAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StartupMVPAudioProcessor::getProgramName (int index)
{
    return {};
}

void StartupMVPAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StartupMVPAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    calibrationManager.setSampleRate (sampleRate);
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;
    
    gainFilter.prepare (spec);
    
    startTimer(16.67);
}

void StartupMVPAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    stopTimer();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StartupMVPAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void StartupMVPAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Get channel pointers and clear buffer
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    int numPoints = SetPointManager::NUM_SET_POINTS;
    
    if (isSlidingGainSlider || isSlidingPanSlider)
    {
//        double leftGain = parameters.getRawParameterValue("balance_" + std::to_string(selectedSliderIndex))->load();
//        // Figure out note gain
//        double gain = parameters.getRawParameterValue("gain_" + std::to_string(selectedSliderIndex))->load(); // TODO: This is inefficient
//        leftGain = juce::Decibels::decibelsToGain (leftGain);
//        gain = juce::Decibels::decibelsToGain (gain);
//        
//        if (!sineWaveGenerator.getIsPlayingReferenceFrequency())
//        {
//            leftGain = 1.0;
//            gain = 1.0;
//        }
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            //const float value = sequencer.getNextSample();
            const std::pair<float, float> value = calibrationManager.getNextSample();
            leftChannel[sample] = value.first * 0.05 * 0.5;
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5;
        }
    }
    
    else if (isSlidingPanSlider)
    {
//        double leftGain = parameters.getRawParameterValue("balance_" + std::to_string(selectedSliderIndex - numPoints))->load(); // TODO: This is inefficient
//        leftGain = juce::Decibels::decibelsToGain(leftGain);
//        
//        if (!sineWaveGenerator.getIsPlayingReferenceFrequency())
//        {
//            leftGain = 1.0;
//        }
//        
//        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
//        {
//            const float value = sineWaveGenerator.getNextSample();
//            leftChannel[sample] = value * 0.05 * leftGain;
//            
//            if (rightChannel)
//                rightChannel[sample] = value * 0.05;
//        }
    }
    
    else if (isSlidingPhaseSlider)
    {
        // TODO: Phase slider...
    }
    
    else
    {
        // process audio through the filter
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        
        if (isBypassed)
        {
            gainFilter.process (context);
        }
        else
        {
            gainProcessor.process (context);
            //gainFilter2.process (context);
        }
            
//        balanceFilter.process (context);
//        gainProcessor.process (context);
//        gainFilter.process (context);
    }
    
    
}

//==============================================================================
bool StartupMVPAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* StartupMVPAudioProcessor::createEditor()
{
    return new StartupMVPAudioProcessorEditor (*this);
}

//==============================================================================
void StartupMVPAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    std::unique_ptr <juce::XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary(*xml, destData);
}

void StartupMVPAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    std::unique_ptr <juce::XmlElement> savedParams(getXmlFromBinary(data, sizeInBytes));
    if (savedParams != nullptr)
    {
        if (savedParams->hasTagName(parameters.state.getType()))
        {
            parameters.state = juce::ValueTree::fromXml(*savedParams);
            
            for (int i = 0; i < SetPointManager::NUM_SET_POINTS; ++i)
            {
                double gain = parameters.getRawParameterValue("gain_" + std::to_string(i))->load();
                calibrationManager.setGainAtIdx (i, gain);
            }
            
            gainFilter.update (calibrationManager.getGainCurve(), FFT_SIZE);
            //gainFilter2.update (calibrationManager.getCurve(), gainFilter2Size);
//            balanceFilter.update();
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StartupMVPAudioProcessor();
}


juce::AudioProcessorValueTreeState::ParameterLayout StartupMVPAudioProcessor::createParameterLayout(int numPoints)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Define range for each parameter
    float minGain = -24.0f;
    float maxGain = 48.0;
    float defaultGain = 0.f;
    float step = 0.05f;
    juce::NormalisableRange<float> range (minGain, maxGain, step, 1.0f);

    // Add numPoints gain parameters
    for ( int i = 0; i < numPoints; i++ ) {
        juce::String paramID = "gain_" + std::to_string(i);

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(paramID, 1),
            paramID,
            range,
            defaultGain
        ));
    }
    
    // Add numPoints balance parameters to match
    for ( int i = 0; i < numPoints; i++ ) {
        juce::String paramID = "balance_" + std::to_string(i);

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(paramID, 1),
            paramID,
            range,
            defaultGain
        ));
    }

    return layout;
}

//==============================================================================

void StartupMVPAudioProcessor::timerCallback()
{
    // Curves should be updated implicitly
//    std::vector<float> setPointGains;
//    for (int i = 0; i < setPointLayout.getSetPoints().size(); i++)
//    {
//        double gain = parameters.getRawParameterValue("gain_" + std::to_string(i))->load(); // TODO: This is inefficient
//        
//        // Adjust to have 0 as the floor
//        setPointGains.push_back(gain + 24.f);
//    }
//    curve.setSetPointGains(setPointGains);
//    
//    std::vector<float> balanceSetPointGains;
//    for (int i = 0; i < setPointLayout.getSetPoints().size(); i++)
//    {
//        double gain = parameters.getRawParameterValue("balance_" + std::to_string(i))->load(); // TODO: This is inefficient
//        
//        // Adjust to have 0 as the floor
//        balanceSetPointGains.push_back(gain + 24.f);
//    }
//    balanceCurve.setSetPointGains (balanceSetPointGains);
}

void StartupMVPAudioProcessor::sliderDragStarted(juce::Slider *slider)
{
    // Switch to the selected note
    
    int sliderIndex = slider->getProperties().getValueAt(0);
    if (sliderIndex < SetPointManager::NUM_SET_POINTS)
    {
        isSlidingGainSlider = true;
    }
    else if (sliderIndex < SetPointManager::NUM_SET_POINTS * 2)
    {
        isSlidingPanSlider = true;
    }
    else
    {
        //isSlidingPhaseSlider = true;
        // TODO - generate sound
    }
    selectedSliderIndex = sliderIndex;
}

void StartupMVPAudioProcessor::sliderValueChanged (juce::Slider *slider)
{
    int sliderIndex = slider->getProperties().getValueAt (0);
    if (sliderIndex < SetPointManager::NUM_SET_POINTS)
    {
        calibrationManager.setCurrGain (slider->getValue());
    }
    else
    {
        calibrationManager.setCurrPan (slider->getValue());
    }
}

void StartupMVPAudioProcessor::sliderDragEnded(juce::Slider *slider)
{
    isSlidingGainSlider = false;
    isSlidingPanSlider = false;
    isSlidingPhaseSlider = false;
    
    gainFilter.update (calibrationManager.getGainCurve(), FFT_SIZE);
    //gainFilter2.update (calibrationManager.getCurve(), gainFilter2Size);
    
//    int sliderIndex = slider->getProperties().getValueAt(0);
//    int numSetPoints = setPointLayout.getSetPoints().size();
//    
//    if (sliderIndex < numSetPoints)
//    {
//        
//    }
//    else
//    {
//        balanceFilter.update();
//    }
}

//void StartupMVPAudioProcessor::makeChoice (CalibrationChoice choice)
//{
//    calibrationManager.chooseOption (choice);
//}

void StartupMVPAudioProcessor::toggleBypass()
{
    isBypassed = ! isBypassed;
    std::cout << "isBypassed: " << isBypassed << std::endl;
}
