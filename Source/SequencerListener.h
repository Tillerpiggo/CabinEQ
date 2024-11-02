/*
  ==============================================================================

    SequencerListener.h
    Created: 2 Nov 2024 2:30:21pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

class SequencerListener
{
public:
    virtual ~SequencerListener() = default;
    virtual void onCycleFinish();
};
