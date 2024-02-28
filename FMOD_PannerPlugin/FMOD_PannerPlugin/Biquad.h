#pragma once
#define _USE_MATH_DEFINES
#include <math.h>



enum BiquadType
{
    Lowpass,
    Highpass,
    Bandpass,
    Bandstop,
    Peak,
    Lowshelf,
    Highshelf
};


void CalculateBiquadCoefficients(float* coeffs, BiquadType type, float fc, float Q, float pG) {

    float a0 = 0, a1 = 0, a2 = 0, b1 = 0, b2 = 0;

    float norm;
    float V = powf(10, abs(pG) / 20.0f);
    float K = tanf(M_PI * fc * 0.5f);
    switch (type)
    {
    case Lowpass:
        norm = 1 / (1 + K / Q + K * K);
        a0 = K * K * norm;
        a1 = 2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    case Highpass:
        norm = 1 / (1 + K / Q + K * K);
        a0 = 1 * norm;
        a1 = -2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    case Bandpass:
        norm = 1 / (1 + K / Q + K * K);
        a0 = K / Q * norm;
        a1 = 0;
        a2 = -a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    case Bandstop:
        norm = 1 / (1 + K / Q + K * K);
        a0 = (1 + K * K) * norm;
        a1 = 2 * (K * K - 1) * norm;
        a2 = a0;
        b1 = a1;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    case Peak:
        if (pG >= 0)
        {    // boost
            norm = 1 / (1 + 1 / Q * K + K * K);
            a0 = (1 + V / Q * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - V / Q * K + K * K) * norm;
            b1 = a1;
            b2 = (1 - 1 / Q * K + K * K) * norm;
        }
        else
        {    // cut
            norm = 1 / (1 + V / Q * K + K * K);
            a0 = (1 + 1 / Q * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - 1 / Q * K + K * K) * norm;
            b1 = a1;
            b2 = (1 - V / Q * K + K * K) * norm;
        }
        break;
    case Lowshelf:
        if (pG >= 0)
        {    // boost
            norm = 1 / (1 + sqrtf(2) * K + K * K);
            a0 = (1 + sqrtf(2 * V) * K + V * K * K) * norm;
            a1 = 2 * (V * K * K - 1) * norm;
            a2 = (1 - sqrtf(2 * V) * K + V * K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - sqrtf(2) * K + K * K) * norm;
        }
        else
        {    // cut
            norm = 1 / (1 + sqrtf(2 * V) * K + V * K * K);
            a0 = (1 + sqrtf(2) * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - sqrtf(2) * K + K * K) * norm;
            b1 = 2 * (V * K * K - 1) * norm;
            b2 = (1 - sqrtf(2 * V) * K + V * K * K) * norm;
        }
        break;
    case Highshelf:
        if (pG >= 0)
        {    // boost
            norm = 1 / (1 + sqrtf(2) * K + K * K);
            a0 = (V + sqrtf(2 * V) * K + K * K) * norm;
            a1 = 2 * (K * K - V) * norm;
            a2 = (V - sqrtf(2 * V) * K + K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - sqrtf(2) * K + K * K) * norm;
        }
        else
        {    // cut
            norm = 1 / (V + sqrtf(2 * V) * K + K * K);
            a0 = (1 + sqrtf(2) * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - sqrtf(2) * K + K * K) * norm;
            b1 = 2 * (K * K - V) * norm;
            b2 = (V - sqrtf(2 * V) * K + K * K) * norm;
        }
        break;


    }

    coeffs[0] = a0;
    coeffs[1] = a1;
    coeffs[2] = a2;
    coeffs[3] = b1;
    coeffs[4] = b2;
}


class Biquad {
public:
    float ProcessSample(float input) 
    {

        float output = input * a0 + z1;
        z1 = input * a1 + z2 - b1 * output;
        z2 = input * a2 - b2 * output;

        return output;
    }

    void SetCoefficients(float* coeffs) {
        a0 = coeffs[0];
        a1 = coeffs[1];
        a2 = coeffs[2];
        b1 = coeffs[3];
        b2 = coeffs[4];
    }

    void CalculateCoefficients(BiquadType type, float fc, float q, float pG) {
        float coeffs[5] = {};
        CalculateBiquadCoefficients(coeffs, type, fc, q, pG);

        SetCoefficients(coeffs);

    }

private:
    BiquadType type;

    float a0=0, a1=0, a2=0, b1=0, b2=0;

    float z1=0, z2=0;

};
