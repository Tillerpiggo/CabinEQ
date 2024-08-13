/*
  ==============================================================================

    PinkNoise.h
    Created: 12 Aug 2024 10:28:34pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// Generates pink noise. Adapted from https://github.com/johnfmcrae/NoiseGenerator
class PinkNoise {
public:
    // Author says 12 rows worked out well for him
    PinkNoise (int numRows = 12)
    {
        pinkIndex = 0;
        
        // mask the index so it does not spill outside of the pinkRows vector range
        pinkIndexMask = (1 << numRows) - 1;
        
        // initialize normalization variable
        pinkNorm = 1.0 / (numRows + 1);
        
        // in testing, I found it was better to initialize the rows with noise
        // this avoids a climb up to some max value during the first run through the rows
        for (int i = 0; i < numRows; i++)
            pinkRows.push_back(noiseSrc.nextFloat());
        pinkRunSum = noiseSrc.nextFloat();
    }
    
    // generates pink noise one sample at a time
    float generate() 
    {
        float newRandom, sum;

        // increment and mask index
        pinkIndex = (pinkIndex + 1) & pinkIndexMask;

        // ensure pink index is not zero, if it is, do not update any of the random vals
        if (pinkIndex != 0) {
            // determine the number of trailing zeros in pinkIndex
            int numZeros = 0;
            int n = pinkIndex;
            while ((n & 1) == 0) 
            {
                // bit shift until you run out of trailing zeros
                n = n >> 1;
                numZeros++;
            }
            // McCARTNEY-VOSS ALGORITHM
            // subtract previous value from running sum
            pinkRunSum -= pinkRows[numZeros];
            // generate a new random number
            newRandom = noiseSrc.nextFloat();
            // add the new random number
            pinkRunSum += newRandom;
            // replace the row value at index numZeros with the new random value
            pinkRows[numZeros] = newRandom;
        }

        // add extra white noise value
        sum = pinkRunSum + noiseSrc.nextFloat();

        // scale and return value
        return (sum * pinkNorm);
    }

    // Changes the number of noise generating rows
    // Note that this overrides the initialization found in the constructor
    // AS WELL AS the pinkRows vector. Therefore, it is advised that this
    // function only be called on initialization
    void setRows(int newRows) 
    {
        // reset pinkIndex
        pinkIndex = 0;
        pinkIndexMask = (1 << newRows) - 1;
        pinkNorm = 1.0 / (newRows + 1);
        // clear the pinkRows vector
        pinkRows.clear();
        // reinitialize the pinkRows vector
        for (int i = 0; i < newRows; i++)
            pinkRows.push_back(noiseSrc.nextFloat());
        pinkRunSum = noiseSrc.nextFloat();
    }
    
private:
    juce::Random noiseSrc;
    
    // Math constants for pink noise generation that I don't understand
    std::vector<float> pinkRows; // each row effectively holds an independent random number generator
    float pinkRunSum; // running sum for noise output
    int pinkIndex; // the column index, incremented each sample
    int pinkIndexMask; // the row mask, which ensures that the index of the pinkRows vector is never exceeded
    float pinkNorm; // used to normalize the noise at the output
};
