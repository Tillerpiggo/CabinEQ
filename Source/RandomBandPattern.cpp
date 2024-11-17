///*
//  ==============================================================================
//
//    RandomBandPattern.cpp
//    Created: 11 Nov 2024 6:57:13pm
//    Author:  Tyler Gee
//
//  ==============================================================================
//*/
//
//#include "RandomBandPattern.h"
//
//RandomBandPattern::RandomBandPattern (int numPositions, int numBands)
//{
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_real_distribution<float> freqDistr (2000.0f, 16000.0f);
//    std::uniform_real_distribution<float> amplDistr (-15.0f, 5.0f);
//    std::uniform_real_distribution<float> bandwidthDistr (0.1f, 0.4f);
//    
////    // Generate the random band combinations
////    for (int i = 0; i < numPositions; ++i)
////    {
////        // Add a random position to random positions
////        std::vector<Band> randomProfile;
////        for (int j = 0; j < numBands; ++j)
////        {
////            randomProfile.push_back (Band (j, freqDistr (gen), amplDistr (gen), bandwidthDistr (gen)));
////        }
////        
////        randomPositions.push_back (randomProfile);
////    }
//}
//
//std::vector<Band> RandomBandPattern::getBandsAtTime (float time)
//{
//    // Interpolate the band profile based on the time
//    std::vector<Band> interpolatedProfile;
//    
//    // Get the two nearest indices to time
//    float timeIdx = time * (randomPositions.size() - 1);
//    float idxBelow = floor (timeIdx);
//    float idxAbove = ceil (timeIdx);
//    float blendFactorAbove = timeIdx - floor (timeIdx); // % idx above
//    float blendFactorBelow = 1 - blendFactorAbove;
//    
////    for (int i = 0; i < numBands; ++i)
////    {
////        auto bandBelow = randomPositions[idxBelow][i];
////        auto bandAbove = randomPositions[idxAbove][i];
////        float freq = bandBelow.freq * blendFactorBelow + bandAbove.freq * blendFactorAbove;
////        float ampl = bandBelow.ampl * blendFactorBelow + bandAbove.ampl * blendFactorAbove;
////        float bandwidth = bandBelow.bandwidth * blendFactorBelow + bandAbove.bandwidth * blendFactorAbove;
////        
////        interpolatedProfile.push_back (Band (i, freq, ampl, bandwidth));
////    }
//    
//    return interpolatedProfile;
//}
