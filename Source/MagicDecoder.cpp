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
    
    std::vector<float> data = vals;
    std::vector<int64_t> shape = {1, 3};
    cppflow::tensor input = cppflow::tensor (data, shape);
    cppflow::model model ("/Users/tylergee/Downloads/decoder_model_endpoint2");
    auto operations = model.get_operations();
    
//    std::cout << "operations:" << std::endl;
//    for (const auto& operation : operations)
//        std::cout << operation << std::endl;
    
    std::vector<std::tuple<std::string, cppflow::tensor>> inputs = {{"serving_default_inputs:0", input}};
    std::vector<std::string> outputs = {"StatefulPartitionedCall:0"};
    std::vector<cppflow::tensor> output = model(inputs, outputs);
//    std::cout << output[0] << std::endl;
    
    cppflow::tensor firstOutput = output[0];
    std::vector<float> bandData = firstOutput.get_data<float>();
    
    // Parse into bands
    std::vector<Band> bands;
    for (int i = 0; i < 24; ++i)
    {
        float freq, ampl, q;
        freq = bandData[3 * i];
        q = bandData[3 * i + 1];
        ampl = bandData[3 * i + 2] * 10;
        bands.push_back (Band::withQ (i, freq, ampl, q, Band::Type::both));
        
        std::cout << "adding band (freq: " << freq << ", " << ampl << ", q: " << q << std::endl;
    }
    
    return bands;
}
