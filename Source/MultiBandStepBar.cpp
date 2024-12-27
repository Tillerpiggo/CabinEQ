/*
  ==============================================================================

    MultiBandStepBar.cpp
    Created: 25 Dec 2024 1:18:56pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "MultiBandStepBar.h"

MultiBandStepBar::MultiBandStepBar()
{
    
}

MultiBandStepBar::~MultiBandStepBar()
{
    
}

void MultiBandStepBar::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::green);
}

void MultiBandStepBar::resized()
{
    // Calculate bounds such that each step has a 3:2 width:height ratio
    juce::Rectangle<int> bounds (0, 0, stepViews.size() * getHeight() * 1.5f, getHeight());
    
    Layout layout (getBounds().withX (0).withY (0), 8.0f);
    for (int i = 0; i < stepViews.size(); ++i)
    {
        layout.addRow ({ Space (stepViews[i].get()) });
    }
    layout.updateComponentBounds();
}

void MultiBandStepBar::setListener (Listener* listener)
{
    this->listener = listener;
}

void MultiBandStepBar::setBackendListener (CabinPeqGraphListener* backendListener)
{
    this->backendListener = backendListener;
}

void MultiBandStepBar::updateBandProfile (BandProfile bandProfile)
{
    this->bandProfile = bandProfile;
    auto multiBandSteps = bandProfile.getMultiBandSteps();
    
    // Get the right # of views
    int numSteps = (int) multiBandSteps.size();
    int numStepViews = (int) stepViews.size();
    int numToAdd = numSteps - numStepViews;
    for (int i = 0; i < numToAdd; ++i)
    {
        stepViews.push_back (std::make_unique<MultiBandStepView>());
        stepViews[i + numStepViews]->setListener (this);
        addAndMakeVisible (stepViews[i + numStepViews].get());
    }
    if (numToAdd < 0)
    {
        stepViews.erase (stepViews.end() + numToAdd, stepViews.end());
    }
    
    // Update the views' contents
    for (int i = 0; i < multiBandSteps.size(); ++i)
    {
        stepViews[i]->setMultiBandStep (multiBandSteps[i]);
        std::cout << "multibandstep: (id: " << multiBandSteps[i].getId() << std::endl;
    }
    
    std::cout << "num step views: " << stepViews.size() << std::endl;
    
    resized();
}

void MultiBandStepBar::onStepEnabled (int stepId, bool isEnabled)
{
    if (backendListener != nullptr)
        backendListener->setStepEnabled (stepId, isEnabled);
}

void MultiBandStepBar::onStepSelected (int stepId)
{
    if (listener != nullptr)
        listener->setStepSelected (stepId);
}
