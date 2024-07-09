/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
/*
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StartupMVPAudioProcessorEditor::StartupMVPAudioProcessorEditor (StartupMVPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), curveComponent (p.getCurve())
{
    setSize (800, 620);
    
    addAndMakeVisible (curveComponent);
    
    addAndMakeVisible (lowerPreferredButton);
    addAndMakeVisible (higherPreferredButton);
    
    addAndMakeVisible (toggleCalibrationButton);
    addAndMakeVisible (bypassButton);
    
    addAndMakeVisible (applyCurveButton);
    addAndMakeVisible (referenceSlider);
    
    addAndMakeVisible (redCircle);
    addAndMakeVisible (blueCircle);
    addAndMakeVisible (greenCircle);
    
    redCircle.setColor(juce::Colours::lightcoral.withAlpha (0.7f));
    blueCircle.setColor(juce::Colours::lightskyblue.withAlpha (0.7f));
    
    toggleCalibrationButton.addListener (this);
    lowerPreferredButton.addListener (this);
    higherPreferredButton.addListener (this);
    bypassButton.addListener (this);
    applyCurveButton.addListener (this);
    
    referenceSlider.addListener (this);
    referenceSlider.setRange (-12.0f, 12.0f);
    
    lowerPreferredButton.setEnabled (false);
    higherPreferredButton.setEnabled (false);

    resized(); // to update UI to be correct
    
    startTimer(16); // 60+ times/s
}

StartupMVPAudioProcessorEditor::~StartupMVPAudioProcessorEditor()
{
    toggleCalibrationButton.removeListener (this);
    lowerPreferredButton.removeListener();
    higherPreferredButton.removeListener();
    bypassButton.removeListener (this);
    applyCurveButton.removeListener (this);
    referenceSlider.removeListener (this);
    
    setLookAndFeel(nullptr);
    
    stopTimer();
}

//==============================================================================
void StartupMVPAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void StartupMVPAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    int padding = 10;
    int curveComponentHeight = 200;

    // Reserve the top 200 points for the curve component
    curveComponent.setBounds(area.removeFromTop(curveComponentHeight).reduced(padding));

    // Calculate the height for the switch and bypass button
    int switchHeight = 30;
    int buttonHeight = 80; // Making both buttons taller

    // Calculate the remaining height for the lower and higher preferred buttons
    int availableHeight = area.getHeight() - switchHeight - buttonHeight - (3 * padding);

    // Calculate the new height for the lower and higher preferred buttons (2/3 of available height)
    int buttonSectionHeight = (2 * availableHeight) / 3;
    int theSameButtonHeight = availableHeight / 3;

    // Calculate the width for each button
    int buttonWidth = (area.getWidth() - (3 * padding)) / 2;

    // Layout the lower and higher preferred buttons side by side, taking up 2/3 of available height
    auto buttonsArea = area.removeFromTop(buttonSectionHeight).reduced(padding);
    auto lowerButtonBounds = buttonsArea.removeFromLeft(buttonWidth).reduced(padding / 2);
    lowerPreferredButton.setBounds(lowerButtonBounds);
    auto higherButtonBounds = buttonsArea.reduced(padding / 2);
    higherPreferredButton.setBounds(higherButtonBounds);

    // Place the green circle below the lower and higher preferred buttons
    auto greenCircleArea = area.removeFromTop(theSameButtonHeight).reduced(padding);
    int circleDiameter = 20;
    auto greenCircleBounds = greenCircleArea.withSizeKeepingCentre(circleDiameter, circleDiameter);
    greenCircle.setBounds(greenCircleBounds);

    // Place the calibration switch below the buttons
    toggleCalibrationButton.setBounds(area.removeFromTop(switchHeight).reduced(padding / 2));

    // Calculate the area for the bypass and apply curve buttons, but only on the right half
    auto bottomButtonArea = area.removeFromBottom(buttonHeight).reduced(padding);
    auto rightHalfArea = bottomButtonArea.removeFromRight(bottomButtonArea.getWidth() / 2);
    int buttonSpacing = 5; // Small spacing between buttons

    // Calculate bounds for applyCurveButton and bypassButton
    auto applyCurveButtonArea = rightHalfArea.removeFromLeft((rightHalfArea.getWidth() / 2) - (buttonSpacing / 2));
    auto bypassButtonArea = rightHalfArea.reduced(buttonSpacing / 2);

    // Calculate the area for the referenceSlider to the left of the applyCurveButton
    auto referenceSliderArea = bottomButtonArea.removeFromLeft((bottomButtonArea.getWidth()) - (buttonSpacing / 2)).reduced(padding);

    referenceSlider.setBounds(referenceSliderArea);
    applyCurveButton.setBounds(applyCurveButtonArea);
    bypassButton.setBounds(bypassButtonArea);

    // Calculate the size and position for the circles
    int margin = 5;
    int circleSpacing = 4;
    int leftOffset = 3;

    // Calculate individual circle bounds
    auto redCircleBounds = getLocalBounds().removeFromBottom(circleDiameter + margin).removeFromLeft(circleDiameter + margin).withSizeKeepingCentre(circleDiameter, circleDiameter);
    redCircleBounds.translate(leftOffset + (margin / 2), -margin / 2);
    
    auto blueCircleBounds = redCircleBounds.translated(circleDiameter + circleSpacing, 0);

    redCircle.setBounds(redCircleBounds);
    blueCircle.setBounds(blueCircleBounds);
}

void StartupMVPAudioProcessorEditor::sliderValueChanged (juce::Slider *slider)
{
    std::cout << "slider value changed" << std::endl;
    if (slider == &referenceSlider)
    {
        audioProcessor.setBypassVolume (slider->getValue());
    }
}

void StartupMVPAudioProcessorEditor::buttonClicked (juce::Button *button)
{
    if (button == &toggleCalibrationButton)
    {
        audioProcessor.toggleCalibration();
        
        bool isEnabled = toggleCalibrationButton.getToggleState();
        lowerPreferredButton.setEnabled (isEnabled);
        higherPreferredButton.setEnabled (isEnabled);
    }
    else if (button == lowerPreferredButton.getButtonPointer())
    {
        audioProcessor.calibrateWith (CalibrationChoice::LowerPreferred);
    }
    else if (button == higherPreferredButton.getButtonPointer())
    {
        audioProcessor.calibrateWith (CalibrationChoice::HigherPreferred);
    }
    else if (button == &bypassButton)
    {
        audioProcessor.toggleBypass();
    }
    else if (button == &applyCurveButton)
    {
        audioProcessor.applyCurve();
    }
}

void StartupMVPAudioProcessorEditor::timerCallback()
{
    // Update button text to match question
    Question currentQuestion = audioProcessor.getCurrentQuestion();
    lowerPreferredButton.setLabelText (currentQuestion.lowerText());
    higherPreferredButton.setLabelText (currentQuestion.higherText());
        
    // Update lights to match which tone is playing
    if (! toggleCalibrationButton.getToggleState())
    {
        redCircle.setColor(juce::Colours::lightcoral.withAlpha (0.7f));
        blueCircle.setColor(juce::Colours::lightskyblue.withAlpha (0.7f));
    }
    else if (audioProcessor.isPlayingFirstNote())
    {
        if (currentQuestion.getType() == QuestionType::Pan)
        {
            greenCircle.setColor (juce::Colours::green.withSaturation (1.0f));
        }
        else
        {
            redCircle.setColor (juce::Colours::lightcoral.withSaturation (1.0f));
            blueCircle.setColor (juce::Colours::lightskyblue.withAlpha (0.7f));
            greenCircle.setColor (juce::Colours::green.withAlpha (0.2f));
            lowerPreferredButton.setDeepRed (true);
            higherPreferredButton.setDeepRed (false);
        }
        
    }
    else
    {
        if (currentQuestion.getType() == QuestionType::Pan)
        {
            lowerPreferredButton.setDeepRed (true);
            higherPreferredButton.setDeepRed (true);
        }
        else
        {
            redCircle.setColor (juce::Colours::lightcoral.withAlpha (0.7f));
            blueCircle.setColor (juce::Colours::lightskyblue.withSaturation (1.0f));
            lowerPreferredButton.setDeepRed (false);
            higherPreferredButton.setDeepRed (true);
        }
    }
    
}
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StartupMVPAudioProcessorEditor::StartupMVPAudioProcessorEditor (StartupMVPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), curveComponent (p.getCurve())
{
    setSize (800, 620);
    
    addAndMakeVisible (curveComponent);
    
    addAndMakeVisible (lowerPreferredButton);
    addAndMakeVisible (higherPreferredButton);
    
    addAndMakeVisible (toggleCalibrationButton);
    addAndMakeVisible (bypassButton);
    
    addAndMakeVisible (applyCurveButton);
    addAndMakeVisible (referenceSlider);
    
    addAndMakeVisible (redCircle);
    addAndMakeVisible (blueCircle);
    
    redCircle.setColor(juce::Colours::lightcoral.withAlpha (0.7f));
    blueCircle.setColor(juce::Colours::lightskyblue.withAlpha (0.7f));
    
    toggleCalibrationButton.addListener (this);
    lowerPreferredButton.addListener (this);
    higherPreferredButton.addListener (this);
    bypassButton.addListener (this);
    applyCurveButton.addListener (this);
    
    referenceSlider.addListener (this);
    referenceSlider.setRange (-12.0f, 12.0f);
    calibrationDBSlider.setValue (71.0f);
    calibrationFactorSlider.setValue (0.5f);
    fmDBSlider.setValue (82.5f);
    
    lowerPreferredButton.setEnabled (false);
    higherPreferredButton.setEnabled (false);

    // Add sliders
    addAndMakeVisible(calibrationDBSlider);
    addAndMakeVisible(calibrationFactorSlider);
    addAndMakeVisible(fmDBSlider);

    calibrationDBSlider.addListener(this);
    calibrationDBSlider.setRange(40, 100);
    calibrationDBSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    calibrationFactorSlider.addListener(this);
    calibrationFactorSlider.setRange(0.0, 1.0);
    calibrationFactorSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    fmDBSlider.addListener(this);
    fmDBSlider.setRange(60, 120);
    fmDBSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    resized(); // to update UI to be correct
    
    startTimer(16); // 60+ times/s
}

StartupMVPAudioProcessorEditor::~StartupMVPAudioProcessorEditor()
{
    toggleCalibrationButton.removeListener (this);
    lowerPreferredButton.removeListener();
    higherPreferredButton.removeListener();
    bypassButton.removeListener (this);
    applyCurveButton.removeListener (this);
    referenceSlider.removeListener (this);

    calibrationDBSlider.removeListener(this);
    calibrationFactorSlider.removeListener(this);
    fmDBSlider.removeListener(this);
    
    setLookAndFeel(nullptr);
    
    stopTimer();
}

//==============================================================================
void StartupMVPAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void StartupMVPAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    int padding = 10;
    int curveComponentHeight = 200;

    // Reserve the top 200 points for the curve component
    curveComponent.setBounds(area.removeFromTop(curveComponentHeight).reduced(padding));

    // Calculate the height for the switch and bypass button
    int switchHeight = 30;
    int buttonHeight = 80; // Making both buttons taller

    // Calculate the remaining height for the lower and higher preferred buttons
    int availableHeight = area.getHeight() - switchHeight - buttonHeight - (3 * padding);

    // Calculate the new height for the lower and higher preferred buttons (2/3 of available height)
    int buttonSectionHeight = (2 * availableHeight) / 3;
    int theSameButtonHeight = availableHeight / 3;

    // Calculate the width for each button
    int buttonWidth = (area.getWidth() - (3 * padding)) / 2;

    // Layout the lower and higher preferred buttons side by side, taking up 2/3 of available height
    auto buttonsArea = area.removeFromTop(buttonSectionHeight).reduced(padding);
    auto lowerButtonBounds = buttonsArea.removeFromLeft(buttonWidth).reduced(padding / 2);
    lowerPreferredButton.setBounds(lowerButtonBounds);
    auto higherButtonBounds = buttonsArea.reduced(padding / 2);
    higherPreferredButton.setBounds(higherButtonBounds);

    // Remove the green circle area and move up the toggle calibration button
    toggleCalibrationButton.setBounds(area.removeFromTop(switchHeight).reduced(padding / 2));

    // Calculate the area for the bypass and apply curve buttons, but only on the right half
    auto bottomButtonArea = area.removeFromBottom(buttonHeight).reduced(padding);
    auto rightHalfArea = bottomButtonArea.removeFromRight(bottomButtonArea.getWidth() / 2);
    int buttonSpacing = 5; // Small spacing between buttons

    // Calculate bounds for applyCurveButton and bypassButton
    auto applyCurveButtonArea = rightHalfArea.removeFromLeft((rightHalfArea.getWidth() / 2) - (buttonSpacing / 2));
    auto bypassButtonArea = rightHalfArea.reduced(buttonSpacing / 2);

    // Calculate the area for the referenceSlider to the left of the applyCurveButton
    auto referenceSliderArea = bottomButtonArea.removeFromLeft((bottomButtonArea.getWidth()) - (buttonSpacing / 2)).reduced(padding);

    referenceSlider.setBounds(referenceSliderArea);
    applyCurveButton.setBounds(applyCurveButtonArea);
    bypassButton.setBounds(bypassButtonArea);

    // Calculate the size and position for the circles
    int margin = 5;
    int circleSpacing = 4;
    int leftOffset = 3;

    // Calculate individual circle bounds
    int circleDiameter = 20;
    auto redCircleBounds = getLocalBounds().removeFromBottom(circleDiameter + margin).removeFromLeft(circleDiameter + margin).withSizeKeepingCentre(circleDiameter, circleDiameter);
    redCircleBounds.translate(leftOffset + (margin / 2), -margin / 2);
    
    auto blueCircleBounds = redCircleBounds.translated(circleDiameter + circleSpacing, 0);

    redCircle.setBounds(redCircleBounds);
    blueCircle.setBounds(blueCircleBounds);

    // Set bounds for the sliders horizontally with more vertical space
    int sliderWidth = (area.getWidth() - (4 * padding)) / 3;
    int sliderHeight = 70;  // Increased height for more vertical space

    auto sliderArea = area.removeFromTop(sliderHeight).reduced(padding);
    calibrationDBSlider.setBounds(sliderArea.removeFromLeft(sliderWidth).reduced(padding / 2));
    calibrationFactorSlider.setBounds(sliderArea.removeFromLeft(sliderWidth).reduced(padding / 2));
    fmDBSlider.setBounds(sliderArea.reduced(padding / 2));
}

void StartupMVPAudioProcessorEditor::sliderValueChanged (juce::Slider *slider)
{
    std::cout << "slider value changed" << std::endl;
    if (slider == &referenceSlider)
    {
        audioProcessor.setBypassVolume (slider->getValue());
    }
    else if (slider == &calibrationDBSlider ||
             slider == &calibrationFactorSlider ||
             slider == &fmDBSlider)
    {
        audioProcessor.setFletcherMunsonCompensation (calibrationDBSlider.getValue(),
                                                      calibrationFactorSlider.getValue(),
                                                      fmDBSlider.getValue());
    }
}

void StartupMVPAudioProcessorEditor::buttonClicked (juce::Button *button)
{
    if (button == &toggleCalibrationButton)
    {
        audioProcessor.toggleCalibration();
        
        bool isEnabled = toggleCalibrationButton.getToggleState();
        lowerPreferredButton.setEnabled (isEnabled);
        higherPreferredButton.setEnabled (isEnabled);
    }
    else if (button == lowerPreferredButton.getButtonPointer())
    {
        audioProcessor.calibrateWith (CalibrationChoice::LowerPreferred);
    }
    else if (button == higherPreferredButton.getButtonPointer())
    {
        audioProcessor.calibrateWith (CalibrationChoice::HigherPreferred);
    }
    else if (button == &bypassButton)
    {
        audioProcessor.toggleBypass();
    }
    else if (button == &applyCurveButton)
    {
        audioProcessor.applyCurve();
    }
}

void StartupMVPAudioProcessorEditor::timerCallback()
{
    // Update button text to match question
    Question currentQuestion = audioProcessor.getCurrentQuestion();
    lowerPreferredButton.setLabelText (currentQuestion.lowerText());
    higherPreferredButton.setLabelText (currentQuestion.higherText());
        
    // Update lights to match which tone is playing
    if (! toggleCalibrationButton.getToggleState())
    {
        redCircle.setColor(juce::Colours::lightcoral.withAlpha (0.7f));
        blueCircle.setColor(juce::Colours::lightskyblue.withAlpha (0.7f));
    }
    else if (audioProcessor.isPlayingFirstNote())
    {
        if (currentQuestion.getType() == QuestionType::Pan)
        {
            greenCircle.setColor (juce::Colours::green.withSaturation (1.0f));
        }
        else
        {
            redCircle.setColor (juce::Colours::lightcoral.withSaturation (1.0f));
            blueCircle.setColor (juce::Colours::lightskyblue.withAlpha (0.7f));
            greenCircle.setColor (juce::Colours::green.withAlpha (0.2f));
            lowerPreferredButton.setDeepRed (true);
            higherPreferredButton.setDeepRed (false);
        }
        
    }
    else
    {
        if (currentQuestion.getType() == QuestionType::Pan)
        {
            lowerPreferredButton.setDeepRed (true);
            higherPreferredButton.setDeepRed (true);
        }
        else
        {
            redCircle.setColor (juce::Colours::lightcoral.withAlpha (0.7f));
            blueCircle.setColor (juce::Colours::lightskyblue.withSaturation (1.0f));
            lowerPreferredButton.setDeepRed (false);
            higherPreferredButton.setDeepRed (true);
        }
    }
}
