#include "Filter.h"
#include <cmath>  // para std::exp se quiseres mais preciso, mas simples aqui

LowPassFilter::LowPassFilter() 
{
    // default cutoff_norm = 0.5f ou algo
    //SetCutoff(0.5f);
}

void LowPassFilter::SetCutoff(float cutoff_norm) 
{
    cutoff_norm = std::max(0.0f, std::min(1.0f, cutoff_norm));
    //alpha = cutoff_norm;  // simples; para mais preciso: 
    alpha = std::exp(-2.0f * 3.14159 * (1.0f - cutoff_norm));
    //alpha = 1.0f - std::pow(0.01f, cutoff_norm);
}

float LowPassFilter::Process(float input) 
{
    float output = alpha * input + (1.0f - alpha) * prev_output;
    prev_output = output;

    return output;
    //return output * (1.0f + 2.0f * (1.0f - alpha));  // aumenta ganho quando filtro fechado
}

void LowPassFilter::Reset() 
{
    prev_output = 0.0f;
}


