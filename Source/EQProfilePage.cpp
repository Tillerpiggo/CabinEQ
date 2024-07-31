/*
  ==============================================================================

    EQProfilePage.cpp
    Created: 25 Jul 2024 9:00:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

/*
#include "EQProfilePage.h"

EQProfilePage::EQProfilePage (StartupMVPAudioProcessor& p) 
    : processor (p), curveComponent (p.getCurve())
{
    addAndMakeVisible (curveComponent);
    addAndMakeVisible (addEQNodeButton);
    addAndMakeVisible (viewport);
    addAndMakeVisible (eqNodeControllerContainer);
    addEQNodeButton.addListener (this);
    updateUIWithEQNodes (p.getEQNodes());
}

EQProfilePage::~EQProfilePage()
{
    addEQNodeButton.removeListener (this);
}

void EQProfilePage::resized()
{
    auto area = getLocalBounds();
    int padding = 10;

    int curveComponentHeight = 200;
    curveComponent.setBounds(area.removeFromTop(curveComponentHeight).reduced(padding));

    int buttonHeight = 100;
    addEQNodeButton.setBounds(area.removeFromBottom(buttonHeight).reduced(padding));
    viewport.setBounds(area);

    int eqNodeControllerWidth = area.getWidth();
    int eqNodeControllerHeight = 200;
    int totalHeight = static_cast<int>(eqNodeControllers.size()) * (eqNodeControllerHeight + padding) - padding;

    eqNodeControllerContainer.setSize(eqNodeControllerWidth + 2 * padding, totalHeight + 2 * padding);

    for (int i = 0; i < eqNodeControllers.size(); ++i)
    {
        auto& eqNodeController = *eqNodeControllers[i];
        eqNodeController.setBounds(0,
                                   i * (eqNodeControllerHeight),
                                   eqNodeControllerWidth,
                                   eqNodeControllerHeight);
    }
}

void EQProfilePage::paint (juce::Graphics& g)
{
    
}

void EQProfilePage::eqNodeStartedChange (int id, float frequency, float amplitude, float pan)
{
    processor.updateEQNode (id, frequency, amplitude, pan);
    processor.startCalibratingEQNode (EQNode (id, frequency, amplitude, pan));
}

void EQProfilePage::eqNodeChanged (int id, float frequency, float amplitude, float pan)
{
    processor.updateEQNode (id, frequency, amplitude, pan);
    processor.updateCalibratingEQNode (EQNode (id, frequency, amplitude, pan));
}

void EQProfilePage::eqNodeEndedChange (int id, float frequency, float amplitude, float pan)
{
    processor.updateEQNode (id, frequency, amplitude, pan);
    processor.endCalibratingEQNode();
}

void EQProfilePage::removeButtonClicked (int id)
{
    processor.removeEQNode (id);
    updateUIWithEQNodes (processor.getEQNodes());
    resized();
}

void EQProfilePage::buttonClicked (juce::Button *button)
{
    if (button == &addEQNodeButton)
    {
        processor.addEQNode (1000.0f, 0.0f, 0.0f);
//        std::cout << "Added node (again)" << std::endl;
        updateUIWithEQNodes (processor.getEQNodes());
    }
}

void EQProfilePage::updateUIWithEQNodes (const std::vector<EQNode>& eqNodes)
{
    // First, clear current UI
    for (auto& eqNodeController : eqNodeControllers)
    {
        eqNodeController->removeListener();
        eqNodeController.reset();
    }
    eqNodeControllers.clear();
    
    // Then, add each one to the container
    for (const auto& eqNode : eqNodes)
    {
        eqNodeControllers.push_back (std::make_unique<EQNodeController> (eqNode));
        auto& eqNodeControllerThatWasJustAdded = *eqNodeControllers.back();
        eqNodeControllerContainer.addAndMakeVisible (eqNodeControllerThatWasJustAdded);
        eqNodeControllerThatWasJustAdded.setListener (this);
    }
    
    // Make the container visible and add it to the viewport
    viewport.setViewedComponent (&eqNodeControllerContainer, true);
    resized();
}
 */
