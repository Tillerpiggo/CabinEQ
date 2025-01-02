/*
  ==============================================================================

    MagicDecoder.h
    Created: 30 Dec 2024 6:58:04pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandProfile.h"
#include <tensorflow/c/tf_tensor.h>
#include "cppflow/cppflow.h"

class MagicDecoder
{
public:
    virtual ~MagicDecoder() = default;
    
    virtual std::vector<Band> decode (std::vector<float> vals) = 0;
};

class SimpleDecoder  : public MagicDecoder
{
public:
    SimpleDecoder();

    std::vector<Band> decode (std::vector<float> vals);
};

class TensorflowDecoder  : public MagicDecoder
{
public:
    TensorflowDecoder();
    
    std::vector<Band> decode (std::vector<float> vals);
};
