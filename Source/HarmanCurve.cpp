/*
  ==============================================================================

    HarmanCurve.cpp
    Created: 27 Jun 2024 3:43:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "HarmanCurve.h"

const std::complex<float> HarmanCurve::valueAtFrequency (float frequency) const
{
    int numPoints = frequencies.size();

    // Windowing to handle out-of-bounds frequencies
    if (frequency < frequencies.at(0))
    {
        return - gains.at(0);
    }

    if (frequency > frequencies.at(numPoints - 1))
    {
        return - gains.at(numPoints - 1);
    }

    float setPointFreq1;
    float setPointGain1;
    float setPointFreq2;
    float setPointGain2;

    float setPointGain0;
    float setPointGain3;

    for (int i = 0; i < numPoints; ++i)
    {
        // If the frequency is the same, return the value of the set point
        if (frequency == frequencies.at(i))
        {
            return - gains.at(i);
        }

        if (frequency < frequencies.at(i))
        {
            setPointFreq1 = frequencies.at(i - 1); // There should always be a previous set point. The only way for there not to be one is if freq <= frequencies.at(0), but we already check those cases.
            setPointFreq2 = frequencies.at(i);
            setPointGain1 = gains.at(i - 1); // Same reasoning as above.
            setPointGain2 = gains.at(i);

            if (i > 1)
            {
                setPointGain0 = gains.at(i - 2);
            }
            else
            {
                setPointGain0 = setPointGain1;
            }

            if (i < numPoints - 1)
            {
                setPointGain3 = gains.at(i+1);
            }
            else
            {
                setPointGain3 = setPointGain2;
            }

            break;
        }
    }

    // Interpolate using catmull-rom
    float t = (frequency - setPointFreq1) / (setPointFreq2 - setPointFreq1);
    float gainAtFrequency = catmullRom (t, setPointGain0, setPointGain1, setPointGain2, setPointGain3);

    return - gainAtFrequency;
}

const float HarmanCurve::catmullRom (float t, float y0, float y1, float y2, float y3) const
{
    float y = 0.5 * ((2.f * y1) + (-y0 + y2) * t + (2.f * y0 - 5.f * y1 + 4.f * y2 - y3) * pow(t, 2) + (-y0 + 3 * y1 - 3 * y2 + y3) * pow(t, 3));
    
    return y;
}
