/*
  ==============================================================================

    InverseFletcherMunsonCurve.cpp
    Created: 25 Jun 2024 12:18:19pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "InverseFletcherMunsonCurve.h"

const std::vector<float> InverseFletcherMunsonCurve::iso226(float L_n, std::vector<float> &frequencies) const
{
    // Define the constants
    std::vector<float> f = {
        20, 25, 31.5, 40, 50, 63, 80,
        100, 125, 160, 200, 250, 315, 400, 500,
        630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000,
        10000, 12500
    };

    std::vector<float> a_f = {
        0.532, 0.506, 0.480, 0.455, 0.432, 0.409, 0.387,
        0.367, 0.349, 0.330,
        0.315, 0.301, 0.288, 0.276, 0.267, 0.259, 0.253, 0.250, 0.246, 0.244,
        0.243, 0.243, 0.243, 0.242, 0.242, 0.245, 0.254, 0.271, 0.301
    };

    std::vector<float> L_u = {
        -31.6, -27.2, -23.0, -19.1, -15.9, -13.0, -10.3, -8.1, -6.2, -4.5,
        -3.1, -2.0, -1.1, -0.4, 0.0, 0.3, 0.5, 0.0, -2.7, -4.1, -1.0, 1.7,
        2.5, 1.2, -2.1, -7.1, -11.2, -10.7, -3.1
    };

    std::vector<float> T_f = {
        78.5, 68.7, 59.5, 51.1, 44.0, 37.5, 31.5, 26.5, 22.1, 17.9, 14.4,
        11.4, 8.6, 6.2, 4.4, 3.0, 2.2, 2.4, 3.5, 1.7, -1.3, -4.2, -6.0,
        -5.4, -1.5, 6.0, 12.6, 13.9, 12.3
    };

    // Calculate A_F
    std::vector<float> A_F(f.size());
    for (size_t i = 0; i < f.size(); ++i) {
        A_F[i] = 4.47e-3 * (pow(10, 0.025 * L_n) - 1.15) +
                 pow((0.4 * pow(10, (T_f[i] + L_u[i]) / 10 - 9)), a_f[i]);
    }

    // Calculate L_p
    std::vector<float> L_p(f.size());
    for (size_t i = 0; i < f.size(); ++i) {
        L_p[i] = (10 / a_f[i] * log10(A_F[i])) - L_u[i] + 94;
    }

    // Return the frequencies as well
    frequencies = f;
    return L_p;
}

// TODO: This code is kinda reused from Curve... there's probably a clever way to combine them, but for
// TODO: now, this is what it'll be...
const float InverseFletcherMunsonCurve::valueAtFrequency (float frequency, float L_n) const
{
    std::vector<float> freqs;
    std::vector<float> gains = iso226(L_n, freqs);
    
    int numPoints = freqs.size();
    
    // Windowing (kinda poorly tho)
    
    if (frequency < freqs.at(0))
    {
        return gains.at(0);
    }
    
    if (frequency > freqs.at(numPoints - 1))
    {
        return gains.at(numPoints - 1); // TODO: Make this also roll offs
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
        if (frequency == freqs.at(i))
        {
            return gains.at(i);
        }
        
        if (frequency < freqs.at(i))
        {
            setPointFreq1 = freqs.at(i - 1); // There should always be a previous set point. The only way for there not to be one is if freq <= setPoints.at(0), but we already check those cases.
            setPointFreq2 = freqs.at(i);
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
    
    return gainAtFrequency;
}

// TODO: This is actually reused from Curve... we should make a class that stores this kind of math and does it for us.
const float InverseFletcherMunsonCurve::catmullRom (float t, float y0, float y1, float y2, float y3) const
{
    float y = 0.5 * ((2.f * y1) + (-y0 + y2) * t + (2.f * y0 - 5.f * y1 + 4.f * y2 - y3) * pow(t, 2) + (-y0 + 3 * y1 - 3 * y2 + y3) * pow(t, 3));
    
    return y;
}
