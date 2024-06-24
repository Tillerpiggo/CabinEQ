/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SetPointManager.h"


//==============================================================================
StartupMVPAudioProcessorEditor::StartupMVPAudioProcessorEditor (StartupMVPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), curveComponent (p.getCurve())
{
    setSize (800, 600);
    
    // Retrieve note data from the NoteDataManager
//    const auto& setPoints = p.getSetPointManager().getSetPoints();

//    // Create a slider for each note
//    for ( int i = 0; i < setPoints.size() * 2; i++ )
//    {
//        std::string name;
//        std::string paramName;
//        
//        if (i < setPoints.size())
//        {
//            name = std::to_string (setPoints.at(i));
//            paramName = "gain_" + std::to_string (i);
//            addSlider(name, paramName, i);
//        }
//        else
//        {
//            name = std::to_string (setPoints.at(i - setPoints.size()));
//            paramName = "balance_" + std::to_string (i - setPoints.size());
//            addSlider(name, paramName, i);
//        }
//    }
    
    // Configure viewport
//    viewport.setViewedComponent(&sliderContainer, false);
//    viewport.setScrollBarsShown(true, false);
//    viewport.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all);
//    addAndMakeVisible(viewport);
    
    // Initialize curve component
//    Curve curve = p.getCurve();
//    curveComponent = CurveComponent (curve);
//    
//    Curve balanceCurve = p.getBalanceCurve();
//    balanceCurveComponent = CurveComponent (balanceCurve);
//    
    addAndMakeVisible(curveComponent);
//    addAndMakeVisible(balanceCurveComponent);
    
//    addAndMakeVisible (buttonA);
//    addAndMakeVisible (buttonB);
//    
//    buttonA.addListener (this);
//    buttonB.addListener (this);
    
    resized();
}

StartupMVPAudioProcessorEditor::~StartupMVPAudioProcessorEditor()
{
    buttonA.removeListener (this);
    buttonB.removeListener (this);
}

//==============================================================================
void StartupMVPAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void StartupMVPAudioProcessorEditor::resized()
{
    curveComponent.setBounds(getLocalBounds());
//    balanceCurveComponent.setBounds(getLocalBounds().withTop(200).withBottom(400));
//    viewport.setBounds(getLocalBounds().withTop(400));
    //viewport.setBounds (getLocalBounds().withTop(200));
    
    // Constants for adjustment
//    const int padding = 10;
//    int sliderHeight = 50;
//
//    // Calculate frame sizes
//    int totalHeight = (50 + padding * 2) * static_cast<int>(sliders.size());
//    sliderContainer.setSize(viewport.getWidth(), totalHeight);
//    auto sliderArea = sliderContainer.getLocalBounds().reduced(60, 20);
//    
//    // Position each slider
//    for (auto& slider : sliders) {
//        auto individualSliderArea = sliderArea.removeFromTop(sliderHeight + padding * 2);
//        slider->setBounds(individualSliderArea.reduced(0, padding));
//    }
    
    // Add the A and B buttons
//    const int buttonHeight = 40; // Height of the buttons
//    const int spacing = 20;      // Spacing between the buttons
//    const int bottomMargin = 80; // Margin from the bottom of the view
//
//    // Calculate the width and position of each button
//    const int buttonWidth = (getWidth() - spacing) / 2;
//    const int buttonY = getHeight() - buttonHeight - bottomMargin;
//
//    // Center the buttons horizontally
//    const int buttonAX = (getWidth() / 2) - buttonWidth - (spacing / 2);
//    const int buttonBX = (getWidth() / 2) + (spacing / 2);
//
//    // Set bounds for the buttons
//    buttonA.setBounds(buttonAX, buttonY, buttonWidth, buttonHeight);
//    buttonB.setBounds(buttonBX, buttonY, buttonWidth, buttonHeight);
}

// Creates a slider and label with the given name, for the given parameter, at the given index,
// and connects that slider to the parameter via attachment
void StartupMVPAudioProcessorEditor::addSlider(std::string name, std::string paramName, int idx)
{
    // Create the slider and label
    auto* slider = new juce::Slider();
    auto* label = new juce::Label();
    
    // Configure the slider and label
    slider->setSliderStyle(juce::Slider::LinearHorizontal);
    slider->setRange(-24.f, 48.f, 0.1f);
    slider->getProperties().set("index", idx);
    slider->addListener(&audioProcessor);
    label->setText(name, juce::dontSendNotification);
    label->attachToComponent(slider, true);
    sliders.push_back(std::unique_ptr<juce::Slider>(slider));
    
    // Add to slider container
    sliderContainer.addAndMakeVisible(slider);
    sliderContainer.addAndMakeVisible(label);

    // Create the slider attachment
    auto attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                                                                                             audioProcessor.parameters, paramName, *slider);
    sliderAttachments.push_back(std::move(attachment));
}

void StartupMVPAudioProcessorEditor::buttonClicked (juce::Button *button)
{
//    std::cout << "button clicked" << std::endl;
//    if (button == &buttonA)
//    {
//        std::cout << "chose smaller" << std::endl;
//        audioProcessor.makeChoice (CalibrationChoice::LowerPreferred);
//    }
//    else if (button == &buttonB)
//    {
//        std::cout << "chose bigger" << std::endl;
//        audioProcessor.makeChoice (CalibrationChoice::HigherPreferred);
//    }
}
