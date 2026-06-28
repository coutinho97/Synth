
#pragma once

#include <array>
#include <cmath>
#include <string>

static constexpr int CYCLES_IN_TABLE = 3;         // ← controlo centralizado
static constexpr int SAMPLES_PER_CYCLE = 256;     // resolução por ciclo
static constexpr int TABLE_SIZE = SAMPLES_PER_CYCLE * CYCLES_IN_TABLE;

class Oscillator 
{
public:

     typedef enum {
          WAVE_NONE      = 0,

          WAVE_SINE      = 1,
          WAVE_SAW       = 2,
          WAVE_SQUARE    = 3,
          WAVE_TRIANGLE  = 4,

          WAVE_TOTAL
     } WaveType;

     Oscillator(WaveType type = WAVE_NONE, float sr = 44100);

     void SetWaveType(WaveType type);
     void SetFrequency(float freq);
     void SetAmplitude(float amp);

     void NoteOn();
     void NoteOff();
     float Process();

     const std::array<float, TABLE_SIZE>& GetWaveform() const; // 2 ciclos fixos

private:

     WaveType wave_type = WAVE_NONE;
     float frequency = 440.0f;
     float amplitude = 1.0f;
     float sample_rate = 44100;
     float phase = 0.0f;
     bool gate = false;

     std::array<float, TABLE_SIZE> waveform_table;    // tabela pré-computada (um ciclo)
     void GenerateTable();  // gera/regera a tabela quando muda wave_type ou amplitude
     void DumpWaveformToFile(const std::string& filename) const;
};


