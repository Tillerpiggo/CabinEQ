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
    addSliderAndLabel (&bandwidthSlider, &bandwidthLabel, "Bandwidth", 0.1f, 12.0f, 1.0f);
    addSliderAndLabel(&spacingSlider, &spacingLabel, "Spacing", 0.1f, 2.0f, 0.5f); // spacing in octaves per band
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
    addButton (&randomizeButton);
    
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
    addButtonAction (&randomizeButton, [this](juce::Button*) {
        generatePermutations();
        updateBands();
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
    layout.addRow ({ Space (&randomizeButton), Space (&addBandsButton), Space (&onButton, 80) });
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

//void KnobView::updateBands()
//{
//    // Calculate parameters
//    float centerFreq = 800.0f * std::pow(2.0f, pitchSlider.getValue());
//    float spacingFactor = 0.5f; // octaves
//    float spacingRatio = std::pow(2.0f, spacingFactor * spacingSlider.getValue());
//
//    float gainMultiplier = gainSlider.getValue(); // Adjust as needed
//    float bandwidthParam = randomSlider.getValue(); // Now from 0 to 1
//
//    // Ensure permutations are generated
//    if (permutations.empty() || finePermutations.empty())
//    {
//        generatePermutations();
//    }
//
//    // Map bandwidthParam to interpolate between permutations
//    const int numPermutations = static_cast<int>(permutations.size());
//    float t = bandwidthParam * (numPermutations - 1);
//    float fineT = fineSlider.getValue() * (numPermutations - 1);
//
//    int index = static_cast<int>(std::floor(t));
//    int fineIdx = static_cast<int>(std::floor(fineT));
//    float alpha = t - index;
//    float beta = fineT - fineIdx;
//
//    // Handle edge cases
//    if (index >= numPermutations - 1)
//    {
//        index = numPermutations - 2;
//        alpha = 1.0f;
//    }
//    
//    if (fineIdx >= numPermutations - 1)
//    {
//        fineIdx = numPermutations - 2;
//        beta = 1.0f;
//    }
//
//    // Get main permutations
//    const std::vector<Band>& perm1 = permutations[index];
//    const std::vector<Band>& perm2 = permutations[index + 1];
//
//    // Get fine permutations
//    const std::vector<Band>& finePerm1 = finePermutations[fineIdx];
//    const std::vector<Band>& finePerm2 = finePermutations[fineIdx + 1];
//
//    const int numBands = static_cast<int>(perm1.size());
//
//    std::vector<Band> interpolatedBands;
//
//    for (int b = 0; b < numBands; ++b)
//    {
//        const Band& band1 = perm1[b];
//        const Band& band2 = perm2[b];
//
//        // Interpolate main permutation parameters
//        float normFreqOffset = juce::jmap(alpha, band1.freq, band2.freq);
//        float normGain = juce::jmap(alpha, band1.ampl, band2.ampl);
//        float normBandwidth = juce::jmap(alpha, band1.bandwidth, band2.bandwidth) * bandwidthSlider.getValue();
//
//        // Interpolate fine permutation parameters
//        const Band& fineBand1 = finePerm1[b];
//        const Band& fineBand2 = finePerm2[b];
//
//        // Interpolate fine frequency multiplier and gain change
//        float fineFreqMultiplier = juce::jmap(beta, fineBand1.freq, fineBand2.freq); // frequency used as multiplier here
//        float fineGainChange = juce::jmap(beta, fineBand1.ampl, fineBand2.ampl);
//
//        // Apply changes to actual frequency
//        float baseFreq = centerFreq * std::pow(spacingRatio, normFreqOffset);
//        float freq = normFreqOffset;//baseFreq * fineFreqMultiplier; // Apply frequency shift
//
//        // Apply gain changes
//        float gain = (normGain) * gainMultiplier * fineGainChange; // Gains are negative
//
//        // Bandwidth remains the same (no change from fine permutations)
//        float bandwidth = normBandwidth;
//
//        // Create the interpolated band
//        Band interpolatedBand(0, std::min (freq, 18000.0f), gain, bandwidth, Band::Type::both);
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

    // Spacing controls the per-band frequency spacing (in octaves per band)
    float spacingValue = spacingSlider.getValue(); // Define the range in your slider initialization

    // Ensure permutations are generated
    if (permutations.empty())
    {
        generatePermutations();
    }

    // Map randomSlider to interpolate between permutations
    const int numPermutations = static_cast<int>(permutations.size());
    float t = randomSlider.getValue() * (numPermutations - 1);

    int index = static_cast<int>(std::floor(t));
    float alpha = t - index;

    // Handle edge cases
    if (index >= numPermutations - 1)
    {
        index = numPermutations - 2;
        alpha = 1.0f;
    }

    const std::vector<Band>& perm1 = permutations[index];
    const std::vector<Band>& perm2 = permutations[index + 1];

    const int numBands = static_cast<int>(perm1.size());

    std::vector<Band> interpolatedBands;

    for (int b = 0; b < numBands; ++b)
    {
        const Band& band1 = perm1[b];
        const Band& band2 = perm2[b];

        // Interpolate normalized parameters
        float freqOffset = juce::jmap(alpha, band1.freq, band2.freq); // in octaves
        float gain = juce::jmap(alpha, band1.ampl, band2.ampl) * gainSlider.getValue();
        float bandwidth = juce::jmap(alpha, band1.bandwidth, band2.bandwidth) * bandwidthSlider.getValue();

        // Calculate actual frequency
        float bandPosition = (static_cast<float>(b) - (numBands - 1) / 2.0f); // Center bands around zero
        float freqInOctaves = bandPosition * spacingValue + freqOffset;
        float freq = centerFreq * std::pow(2.0f, freqInOctaves);

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

//void KnobView::generatePermutations()
//{
//    // Clear existing permutations
//    permutations.clear();
//    finePermutations.clear();
//
//    const int numPermutations = 7;
//    const int numBands = 4;
//    juce::Random random;
//
//    for (int p = 0; p < numPermutations; ++p)
//    {
//        std::vector<Band> permBands;
//        std::vector<Band> finePermBands;
//
//        // Prepare to mute 5 bands in fine permutations (except for the first one)
//        std::set<int> bandsToMute;
//        if (p != 0)
//        {
//            // Select 5 unique random indices to mute
//            while (bandsToMute.size() < 3)
//            {
//                int randomIndex = random.nextInt(numBands); // Random index between 0 and numBands - 1
//                bandsToMute.insert(randomIndex);
//            }
//        }
//
//        for (int b = 0; b < numBands; ++b)
//        {
//            // Main permutations (gain in dB)
//            float randomSpacing = random.nextFloat() * 4.0f - 2.0f;
//
//            float minGainDB = -12.0f; // in dB
//            float maxGainDB = 12.0f;  // in dB
//            float randomGainDB = random.nextFloat() * (maxGainDB - minGainDB) + minGainDB;
//
//            // Random narrow bandwidth
//            float minBandwidth = 0.05f; // in octaves
//            float maxBandwidth = 0.2f;  // in octaves
//            float randomBandwidth = random.nextFloat() * (maxBandwidth - minBandwidth) + minBandwidth;
//
//            // Create the main permutation band (gain in dB)
//            Band mainBand(0, randomSpacing, randomGainDB, randomBandwidth, Band::Type::both);
//            permBands.push_back(mainBand);
//
//            // Fine permutations (gain as a multiplier)
//            float fineFreqMultiplier = 1.0f; // No change in frequency
//            float fineGainMultiplier;
//
//            if (p == 0)
//            {
//                fineGainMultiplier = 1.0f; // For the first fine permutation, no change
//            }
//            else
//            {
//                if (bandsToMute.count(b) > 0)
//                {
//                    // This band is muted in fine permutation
//                    fineGainMultiplier = 0.0f; // Mute the band
//                }
//                else
//                {
//                    fineGainMultiplier = 1.0f; // No change to this band
//                }
//            }
//
//            // Store fine permutation bands (gain as multiplier)
//            Band fineBand(0, fineFreqMultiplier, fineGainMultiplier, 0.0f, Band::Type::both); // Bandwidth change is zero
//            finePermBands.push_back(fineBand);
//        }
//
//        // Add the permutations to the lists
//        permutations.push_back(permBands);
//        finePermutations.push_back(finePermBands);
//    }
//}

//void KnobView::generatePermutations()
//{
//    // Clear existing permutations
//    permutations.clear();
//    finePermutations.clear();
//
//    const int numPermutations = 50; // Number of permutations
//    const int numBands = 10;        // Number of frequency bands
//    const float minFreq = 40.0f;    // Minimum frequency (Hz)
//    const float maxFreq = 15000.0f; // Maximum frequency (Hz)
//
//    juce::Random random;
//
//    // Calculate logarithmically spaced frequencies
//    std::vector<float> frequencies;
//    for (int i = 0; i < numBands; ++i)
//    {
//        float freq = minFreq * std::pow(maxFreq / minFreq, static_cast<float>(i) / (numBands - 1));
//        frequencies.push_back(freq);
//    }
//
//    for (int p = 0; p < numPermutations; ++p)
//    {
//        std::vector<Band> permBands;
//        std::vector<Band> finePermBands;
//
//        for (int b = 0; b < numBands; ++b)
//        {
//            // Toggle band ON (+12 dB) or OFF (-12 dB) randomly
//            float gainDB = random.nextFloat() * 24.0f - 12.0f;//(random.nextBool() ? 12.0f : -12.0f);
//
//            // Bandwidth for each band (in octaves)
//            float minBandwidth = 0.1f; // Narrower bandwidth
//            float maxBandwidth = 0.5f; // Wider bandwidth
//            float randomBandwidth = 0.25f;//random.nextFloat() * (maxBandwidth - minBandwidth) + minBandwidth;
//
//            // Create the main permutation band
//            Band mainBand(0, frequencies[b], gainDB, randomBandwidth, Band::Type::both);
//            permBands.push_back(mainBand);
//
//            // Fine permutation: Same as main permutation for now (can be modified if needed)
//            Band fineBand(0, 1.0f, 1.0f, 0.0f, Band::Type::both);
//            finePermBands.push_back(fineBand);
//        }
//
//        // Add the permutations to the lists
//        permutations.push_back(permBands);
//        finePermutations.push_back(finePermBands);
//    }
//}

void KnobView::generatePermutations()
{
    // Clear existing permutations
    permutations.clear();

    const int numPermutations = 10; // Generate 10 permutations
    const int numBands = 8;         // Each permutation has 3 bands
    juce::Random random;

    for (int p = 0; p < numPermutations; ++p)
    {
        std::vector<Band> permBands;

        for (int b = 0; b < numBands; ++b)
        {
            // Random frequency offset in octaves (-1 to +1)
            float randomFreqOffset = random.nextFloat() * 2.0f - 1.0f;

            // Random gain in dB (-12 dB to +12 dB)
            float randomGain = random.nextFloat() * 24.0f - 12.0f;

            // Random bandwidth in octaves (0.1 to 2.0)
            float randomBandwidth = random.nextFloat() * (2.0f - 0.1f) + 0.1f;

            // Store normalized values
            Band band(0, randomFreqOffset, randomGain, randomBandwidth, Band::Type::both);
            permBands.push_back(band);
        }

        permutations.push_back(permBands);
    }
}
