/*
  ==============================================================================

    MagicDecoder.cpp
    Created: 30 Dec 2024 6:58:04pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "MagicDecoder.h"

SimpleDecoder::SimpleDecoder()
{}

std::vector<Band> SimpleDecoder::decode (std::vector<float> vals)
{
    if (vals.size() != 3)
    {
        std::cerr << "SimpleDecoder expected 3 vals but got " << vals.size() << " instead :(" << std::endl;
        return {};
    }
    
    std::vector<Band> decodedBands;
    
    // Create 12 bands, with the first value indicating their spread, the second their amplitude, and the third their bandwidth
    float startFreq = 40.0f;
    float currFreq = startFreq;
    float interval = 1.5f * vals[0];
    float amplitude = vals[1] * 12.0f;
    float bandwidth = vals[2] * 2.0f + 0.5f;
    
    for (int i = 0; i < 12; ++i)
    {
        decodedBands.push_back (Band (i, currFreq, amplitude, bandwidth, Band::Type::both));
        currFreq *= interval;
    }
    
    return decodedBands;
}

TensorflowDecoder::TensorflowDecoder()
{}

std::vector<Band> TensorflowDecoder::decode (std::vector<float> vals)
{
    if (vals.size() != 3)
    {
        std::cerr << "Tensorflow decoder expected 3 vals but got " << vals.size() << " instead :(" << std::endl;
        return {};
    }
    
    std::vector<Band> decodedBands;
    
    cppflow::tensor input = cppflow::tensor ({0.5, 0.1, 0.7});
    std::cout << input << std::endl;
    cppflow::model model ("/Users/tylergee/Downloads/decoder_model_test3");
    std::cout << "got model" << std::endl;
    auto output = model(input);
    std::cout << output << std::endl;
    
    // Parse into 
    
    return {};
}
