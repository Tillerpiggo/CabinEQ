/*
  ==============================================================================

    KnobView.cpp
    Created: 20 Nov 2024 6:30:19pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "KnobView.h"

KnobView::KnobView()
{
    // Sliders
    addSliderAndLabel (&randomSlider, &randomLabel, "Random", 0.0f, 1.0f, 0.5f);
    addSliderAndLabel (&fineSlider, &fineLabel, "Fine", 0.0f, 1.0f, 0.0f);
    addSliderAndLabel (&bandwidthSlider, &bandwidthLabel, "Bandwidth", 0.5f, 12.0f, 1.0f);
    addSliderAndLabel (&spacingSlider, &spacingLabel, "Spacing", 0.0f, 8.0f, 1.0f); // relative units
    addSliderAndLabel (&pitchSlider, &pitchLabel, "Pitch", -5.0f, 5.0f, 0.0f); // in octaves, from 800hz
    addSliderAndLabel (&gainSlider, &gainLabel, "Gain", -6.0f, 6.0f); // in db
    
    // Slider Actions
    addSliderAction (&randomSlider, [this](juce::Slider*) {
        updateBands();
    });
    addSliderAction (&fineSlider, [this](juce::Slider*) {
        updateBands();
    });
    addSliderAction (&bandwidthSlider, [this](juce::Slider*) {
        updateBands();
    });
    addSliderAction (&spacingSlider, [this](juce::Slider*) {
        updateBands();
    });
    addSliderAction (&pitchSlider, [this](juce::Slider*) {
        updateBands();
    });
    addSliderAction (&gainSlider, [this](juce::Slider*) {
        updateBands();
    });
    
    // Buttons
    addButton (&addBandsButton);
    addButton (&onButton);
    
    // Button Actions
    addButtonAction (&addBandsButton, [this](juce::Button*) {
        if (listener != nullptr)
        {
            listener->addBands (bands);
            setIsOn (false);
        }
    });
    addButtonAction (&onButton, [this](juce::Button*) {
        isOn = ! isOn;
        setIsOn (isOn);
    });
}

KnobView::~KnobView()
{
}

void KnobView::paint (juce::Graphics& g)
{
    g.fillAll (CONTROL_BAR_BACKGROUND_COLOR);
}

void KnobView::resized()
{
    Layout layout (getBounds().withX (0).withY (0), 8.0f);
    layout.addRow ({ Space (80), Space (&randomSlider) });
    layout.addRow ({ Space (80), Space (&fineSlider) });
    layout.addRow ({ Space (80), Space (&bandwidthSlider) });
    layout.addRow ({ Space (80), Space (&spacingSlider) });
    layout.addRow ({ Space (80), Space (&pitchSlider) });
    layout.addRow ({ Space (80), Space (&gainSlider) });
    layout.addRow ({ Space (&addBandsButton), Space (&onButton, 80) });
    layout.updateComponentBounds();
}

void KnobView::setListener (Listener* listener)
{
    this->listener = listener;
}

//void KnobView::updateBands()
//{
//    // Calculate out the values for bands
//    
//    // Start with the default bands
//    float centerFreq = 800.0f * std::pow (2.0f, pitchSlider.getValue());
//    float spacingFactor = 0.5f; // octaves
//    float spacingRatio = std::pow (2.0f, spacingFactor * spacingSlider.getValue());
//    
//    float ampl = gainSlider.getValue();
//    float bandwidth = bandwidthSlider.getValue();
//    Band::Type type = Band::Type::both;
//    
//    std::vector<Band> provisionalBands = {
//        Band (0, centerFreq * pow (spacingRatio, -1), ampl, bandwidth, type),
//        Band (0, centerFreq * pow (spacingRatio, 0), -ampl * 0.5f, bandwidth, type),
//        Band (0, centerFreq * pow (spacingRatio, 1) , ampl * 1.5f, bandwidth, type),
//        Band (0, centerFreq * pow (spacingRatio, 2), -ampl, bandwidth, type),
//    };
//    
//    
//    if (listener != nullptr)
//    {
//        listener->setBands (provisionalBands);
//        bands = provisionalBands;
//    }
//}

//void KnobView::updateBands()
//{
//    // Start with the default parameters
//    float centerFreq = 800.0f * std::pow(2.0f, pitchSlider.getValue());
//    float spacingFactor = 0.5f; // octaves
//    float spacingRatio = std::pow(2.0f, spacingFactor * spacingSlider.getValue());
//
//    float ampl = gainSlider.getValue();
//    float bandwidthParam = randomSlider.getValue(); // Now from 0 to 1
//
//    // Ensure permutations are generated
//    if (permutations.empty())
//    {
//        generatePermutations();
//    }
//
//    // Map bandwidthParam to interpolate between permutations
//    const int numPermutations = static_cast<int>(permutations.size());
//    float t = bandwidthParam * (numPermutations - 1);
//
//    int index = static_cast<int>(std::floor(t));
//    float alpha = t - index;
//
//    // Handle edge cases
//    if (index >= numPermutations - 1)
//    {
//        index = numPermutations - 2;
//        alpha = 1.0f;
//    }
//
//    const std::vector<Band>& perm1 = permutations[index];
//    const std::vector<Band>& perm2 = permutations[index + 1];
//
//    const int numBands = static_cast<int>(perm1.size());
//
//    std::vector<Band> interpolatedBands;
//    
//    const float bandwidthFactor = bandwidthSlider.getValue();
//
//    for (int b = 0; b < numBands; ++b)
//    {
//        const Band& band1 = perm1[b];
//        const Band& band2 = perm2[b];
//
//        // Interpolate normalized parameters
//        float normFreqOffset = juce::jmap(alpha, band1.freq, band2.freq);
//        float normGain = juce::jmap(alpha, band1.ampl, band2.ampl);
//        float normBandwidth = juce::jmap(alpha, band1.bandwidth, band2.bandwidth) * bandwidthFactor;
//
//        // Calculate actual frequency
//        float freq = centerFreq * std::pow(spacingRatio, normFreqOffset);
//
//        // Calculate actual gain (scaled by ampl)
//        float gain = normGain * ampl; // Adjust gain scaling as needed
//
//        // Map normalized bandwidth to actual bandwidth (in octaves)
//        float minBandwidth = 1.0f; // Minimum bandwidth in octaves
//        float maxBandwidth = 4.0f; // Maximum bandwidth in octaves
//        float bandwidth = juce::jmap(normBandwidth, minBandwidth, maxBandwidth);
//
//        // Create the interpolated band
//        Band interpolatedBand(0, freq, gain, bandwidth, Band::Type::both);
//        interpolatedBands.push_back(interpolatedBand);
//    }
//
//    if (listener != nullptr)
//    {
//        listener->setBands(interpolatedBands);
//        bands = interpolatedBands;
//    }
//}

void KnobView::updateBands()
{
    // Calculate parameters
    float centerFreq = 800.0f * std::pow(2.0f, pitchSlider.getValue());
    float spacingFactor = 0.5f; // octaves
    float spacingRatio = std::pow(2.0f, spacingFactor * spacingSlider.getValue());

    float gainMultiplier = gainSlider.getValue(); // Adjust as needed
    float bandwidthParam = randomSlider.getValue(); // Now from 0 to 1

    // Ensure permutations are generated
    if (permutations.empty() || finePermutations.empty())
    {
        generatePermutations();
    }

    // Map bandwidthParam to interpolate between permutations
    const int numPermutations = static_cast<int>(permutations.size());
    float t = bandwidthParam * (numPermutations - 1);
    float fineT = fineSlider.getValue() * (numPermutations - 1);

    int index = static_cast<int>(std::floor(t));
    int fineIdx = static_cast<int>(std::floor(fineT));
    float alpha = t - index;
    float beta = fineT - fineIdx;

    // Handle edge cases
    if (index >= numPermutations - 1)
    {
        index = numPermutations - 2;
        alpha = 1.0f;
    }
    
    if (fineIdx >= numPermutations - 1)
    {
        fineIdx = numPermutations - 2;
        beta = 1.0f;
    }

    // Get main permutations
    const std::vector<Band>& perm1 = permutations[index];
    const std::vector<Band>& perm2 = permutations[index + 1];

    // Get fine permutations
    const std::vector<Band>& finePerm1 = finePermutations[fineIdx];
    const std::vector<Band>& finePerm2 = finePermutations[fineIdx + 1];

    const int numBands = static_cast<int>(perm1.size());

    std::vector<Band> interpolatedBands;

    for (int b = 0; b < numBands; ++b)
    {
        const Band& band1 = perm1[b];
        const Band& band2 = perm2[b];

        // Interpolate main permutation parameters
        float normFreqOffset = juce::jmap(alpha, band1.freq, band2.freq);
        float normGain = juce::jmap(alpha, band1.ampl, band2.ampl);
        float normBandwidth = juce::jmap(alpha, band1.bandwidth, band2.bandwidth) * bandwidthSlider.getValue();

        // Interpolate fine permutation parameters
        const Band& fineBand1 = finePerm1[b];
        const Band& fineBand2 = finePerm2[b];

        // Interpolate fine frequency multiplier and gain change
        float fineFreqMultiplier = juce::jmap(beta, fineBand1.freq, fineBand2.freq); // frequency used as multiplier here
        float fineGainChange = juce::jmap(beta, fineBand1.ampl, fineBand2.ampl);

        // Apply changes to actual frequency
        float baseFreq = centerFreq * std::pow(spacingRatio, normFreqOffset);
        float freq = baseFreq * fineFreqMultiplier; // Apply frequency shift

        // Apply gain changes
        float gain = (normGain) * gainMultiplier * fineGainChange; // Gains are negative

        // Bandwidth remains the same (no change from fine permutations)
        float bandwidth = normBandwidth;

        // Create the interpolated band
        Band interpolatedBand(0, freq, gain, bandwidth, Band::Type::both);
        interpolatedBands.push_back(interpolatedBand);
    }

    if (listener != nullptr)
    {
        listener->setBands(interpolatedBands);
        bands = interpolatedBands;
    }
}

void KnobView::setIsOn (bool isOn)
{
    if (listener != nullptr)
    {
        isOn = isOn;
        listener->setIsOn (isOn);
        onButton.setButtonText (isOn ? "ON" : "OFF");
    }
}

//void KnobView::generatePermutations()
//{
//    // Clear existing permutations
//    permutations.clear();
//
//    const int numPermutations = 7;
//    const int numBands = 6;
//    juce::Random random;
//
//    for (int p = 0; p < numPermutations; ++p)
//    {
//        std::vector<Band> permBands;
//
//        for (int b = 0; b < numBands; ++b)
//        {
//            // Random normalized frequency offset (-2 to +2)
//            float randomSpacing = random.nextFloat() * 4.0f - 2.0f;
//
//            // Random normalized gain (-1 to +1)
//            float randomGain = random.nextFloat() * 2.0f - 1.0f;
//
//            // Random normalized bandwidth (0 to 1)
//            float randomBandwidth = random.nextFloat() * 0.3f;
//
//            // Store normalized values
//            Band band(0, randomSpacing, randomGain, randomBandwidth, Band::Type::both);
//            permBands.push_back(band);
//        }
//
//        permutations.push_back(permBands);
//    }
//}

void KnobView::generatePermutations()
{
    // Clear existing permutations
    permutations.clear();
    finePermutations.clear();

    const int numPermutations = 7;
    const int numBands = 6;
    juce::Random random;

    for (int p = 0; p < numPermutations; ++p)
    {
        std::vector<Band> permBands;
        std::vector<Band> finePermBands;

        for (int b = 0; b < numBands; ++b)
        {
            // Main permutations (as before)
            // Random normalized frequency offset (-2 to +2)
            float randomSpacing = random.nextFloat() * 4.0f - 2.0f;

            // Random negative gain (dips)
            float minGain = -12.0f; // in dB
            float maxGain = 12.0f;  // in dB
            float randomGain = random.nextFloat() * (maxGain - minGain) + minGain;

            // Random narrow bandwidth (sharp dips)
            float minBandwidth = 0.05f; // in octaves
            float maxBandwidth = 0.2f;  // in octaves
            float randomBandwidth = random.nextFloat() * (maxBandwidth - minBandwidth) + minBandwidth;

            // Store main permutation bands
            Band mainBand(0, randomSpacing, randomGain, randomBandwidth, Band::Type::both);
            permBands.push_back(mainBand);

            // Fine permutations (additional changes)
            // For the first permutation (p == 0), no change
            float fineFreqMultiplier = (p == 0) ? 1.0f : 1.0f;//random.nextFloat() * 0.2f + 0.9f; // Multiplier between 0.8 and 1.2

            float fineGainChange = (p == 0) ? 1.0f : random.nextFloat() * 1.0f + 0.5f; // Gain change between -3 dB and +3 dB

            // Store fine permutation bands
            Band fineBand(0, fineFreqMultiplier, fineGainChange, 0.0f, Band::Type::both); // Bandwidth change is zero
            finePermBands.push_back(fineBand);
        }

        permutations.push_back(permBands);
        finePermutations.push_back(finePermBands);
    }
}
