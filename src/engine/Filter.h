#ifndef LOWPASSFILTER_H
#define LOWPASSFILTER_H

class LowPassFilter {
private:
    float alpha = 0.1f;  // coeficiente baseado em cutoff (0.0 = full cutoff, 1.0 = no filter)
    float prev_output = 0.0f;  // estado anterior (stateful)

public:
    LowPassFilter();
    void SetCutoff(float cutoff_norm);  // cutoff_norm: 0.0 (muito fechado) a 1.0 (aberto)
    float Process(float input);
    void Reset();  // limpa estado para preview
};

#endif