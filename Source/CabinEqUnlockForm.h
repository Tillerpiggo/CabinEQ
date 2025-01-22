/*
  ==============================================================================

    CabinEqUnlockForm.h
    Created: 20 Jan 2025 7:21:24pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "CabinEqMarketplaceStatus.h"

#include <JuceHeader.h>

class CabinEqUnlockForm    : public juce::OnlineUnlockForm
{
public:
    CabinEqUnlockForm (CabinEqMarketplaceStatus& status)
        : OnlineUnlockForm (status, "Please provide your email and password.")
    {}

    void dismiss() override
    {
        setVisible (false);
    }
};
