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
    
    std::vector<float> data = {1.0, 2.0, 3.0};
    std::vector<int64_t> shape = {1, 3};
//    cppflow::tensor input = cppflow::tensor (data, shape);
//    std::cout << input << std::endl;
    auto input = cppflow::fill({10, 5}, 1.0f);
    cppflow::model model ("/Users/tylergee/Downloads/testtest");
    std::cout << "got model" << std::endl;
    auto operations = model.get_operations();
    std::cout << "operations:" << std::endl;
    for (const auto& operation : operations)
        std::cout << operation << std::endl;
    
//    auto output = model(input);
    std::vector<std::tuple<std::string, cppflow::tensor>> inputs = {{"serve_input_1:0", input}, {"serving_default_input_1:0", input}};
    std::vector<std::string> outputs = {"StatefulPartitionedCall:0"};
    std::vector<cppflow::tensor> output = model(inputs, outputs);
    std::cout << output[0] << std::endl;
    
    // Parse into 
    
    return {};
}
