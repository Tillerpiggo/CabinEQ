/*
  ==============================================================================

    EQProfilePage.cpp
    Created: 25 Jul 2024 9:00:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "EQProfilePage.h"

EQProfilePage::EQProfilePage (StartupMVPAudioProcessor& p) : processor (p)
{
    addAndMakeVisible (viewport);
    updateUIWithEQNodes (p.getEQNodes());
}

void EQProfilePage::resized()
{
    auto area = getLocalBounds();
    int padding = 10;
    
    viewport.setBounds(area);

    int eqNodeControllerWidth = area.getWidth();
    int eqNodeControllerHeight = 200;
    int totalHeight = static_cast<int> (eqNodeControllers.size()) * eqNodeControllerHeight;

    eqNodeControllerContainer.setSize(eqNodeControllerWidth + 2 * padding, totalHeight + 2 * padding);

    for (int i = 0; i < eqNodeControllers.size(); ++i)
    {
        auto& eqNodeController = *eqNodeControllers[i];
        eqNodeController.setBounds (0,
                                    padding + i * eqNodeControllerHeight,
                                    eqNodeControllerWidth,
                                    eqNodeControllerHeight);
    }
}

void EQProfilePage::paint (juce::Graphics& g)
{
    
}


void EQProfilePage::eqNodeChanged (int id, float frequency, float amplitude, float pan)
{
    processor.updateEQNode (id, frequency, amplitude, pan);
}

void EQProfilePage::removeButtonClicked (int id)
{
    processor.removeEQNode (id);
}

void EQProfilePage::updateUIWithEQNodes (const std::vector<EQNode>& eqNodes)
{
    
    // First, clear current UI
    eqNodeControllers.clear();
    
    // Then, add each one to the container
    for (const auto& eqNode : eqNodes)
    {
        eqNodeControllers.push_back (std::make_unique<EQNodeController> (eqNode));
        auto& eqNodeControllerThatWasJustAdded = *eqNodeControllers.back();
        eqNodeControllerContainer.addAndMakeVisible (eqNodeControllerThatWasJustAdded);
        eqNodeControllerThatWasJustAdded.setListener (std::unique_ptr<EQProfilePage> (this));
    }
    
    // Make the container visible and add it to the viewport
    addAndMakeVisible (viewport);
    addAndMakeVisible (eqNodeControllerContainer);
    viewport.setViewedComponent (&eqNodeControllerContainer, true);
     
}
