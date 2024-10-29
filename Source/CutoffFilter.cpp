/*
  ==============================================================================

    CutoffFilter.cpp
    Created: 27 Oct 2024 1:12:36pm
    Author:  Tyler Gee

  ==============================================================================
*/

//#include "CutoffFilter.h"
//
//CutoffFilter::CutoffFilter()
//{
//    
//}
//
//void CutoffFilter::setCutoff (CutoffType type, float freq)
//{
//    filterType = type;
//    filterFreq = freq;
//    shouldUpdateFilter = true;
//}
//
//void CutoffFilter::process (juce::dsp::AudioBlock<float>& block)
//{
//    if (shouldUpdateFilter)
//    {
//        switch (filterType)
//        {
//            case CutoffType::lowPass:
//                setWithLowPassCoefficients (filterFreq);
//                break;
//            case CutoffType::highPass:
//                setWithHighPassCoefficients (filterFreq);
//                break;
//        }
//        shouldUpdateFilter = false;
//    }
//    
//    juce::dsp::ProcessContextReplacing<float> context (block);
//    cutoffFilter.process (context);
//}
//
//using Filter = juce::dsp::IIR::Filter<float>;
//using Coefficients = Filter::CoefficientsPtr;
//void CutoffFilter::setWithLowPassCoefficients (float freq)
//{
//    setFilterCoefficients (Coefficients::makeLowPass);
//}
//
//void CutoffFilter::setWithHighPassCoefficients (float freq)
//{
//    
//}
//
//void CutoffFilter::setFilterCoefficients (Filter::CoefficientsPtr coefficients)
//{
//    
//}

