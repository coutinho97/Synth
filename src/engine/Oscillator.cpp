
#include "Oscillator.h"
#include <fstream>

#define M_PI		3.14159265358979323846


/* DEBUG */
#include "../Logger/logger.h"
#define DEBUG_ENABLE 		1

#if DEBUG_ENABLE
	#define DEBUG(fmt, ...)   	Logger::Format("[osc] " fmt, ##__VA_ARGS__)
#else
	#define DEBUG(fmt, ...)
#endif


Oscillator::Oscillator(WaveType t, float sr) : wave_type(t), sample_rate(sr)
{
     GenerateTable();
     //DumpWaveformToFile("waveform.txt");
}

void Oscillator::SetWaveType(WaveType type)
{
     if (type != wave_type) 
     {
          wave_type = type;
          GenerateTable();
     }
}

void Oscillator::SetFrequency(float f) 
{ 
     if (f != frequency)
     {
          frequency = f; 
          GenerateTable();
     }
}

void Oscillator::SetAmplitude(float a) 
{ 
     if (a != amplitude) 
     {
          amplitude = a;
          GenerateTable(); // regenera porque amplitude mudou
     }
}

void Oscillator::NoteOn() 
{ 
     gate = true;
}

void Oscillator::NoteOff() 
{ 
     gate = false;
}

float Oscillator::Process() 
{
     // 1. Calcular posição na tabela (0.0 ... TABLE_SIZE)
     float index = phase * static_cast<float>(SAMPLES_PER_CYCLE);

     // 2. Índice inteiro + fração
     size_t i0 = static_cast<size_t>(index);           // parte inteira
     float frac = index - static_cast<float>(i0);      // parte fracionária

     // 3. Wrap-around (muito importante quando TABLE_SIZE > samples de 1 ciclo)
     size_t i1 = (i0 + 1) % TABLE_SIZE;

     // 4. Interpolação linear
     float sample = waveform_table[i0] * (1.0f - frac) + waveform_table[i1] * frac;

     // 5. Aplicar amplitude / gate / envelope
     //sample *= amplitude * (gate ? 1.0f : 0.0f);
     sample *= (gate ? 1.0f : 0.0f);

     // 6. Avançar a fase — ESTA LINHA NUNCA MUDA, independentemente de TABLE_SIZE ou CYCLES_IN_TABLE
     phase += frequency / sample_rate;

     // 7. Wrap phase para [0,1)  ← mantém o conceito de "um ciclo = phase 0→1"
     phase -= std::floor(phase);

     return sample;
}

const std::array<float, TABLE_SIZE>& Oscillator::GetWaveform() const 
{
    return waveform_table;
}


/*
     Private
*/
void Oscillator::GenerateTable() 
{
     for (int i = 0; i < TABLE_SIZE; ++i) 
     {
          // phase dentro de UM ciclo (sempre 0..1)
          float cycle_phase = static_cast<float>(i) / SAMPLES_PER_CYCLE;
          float frac_phase = cycle_phase - std::floor(cycle_phase);

          float sample = 0.0f;
          
          switch (wave_type) 
          {
               case WAVE_SINE:
               {
                    sample = std::sinf(2.0f * M_PI * frac_phase);
                    break;
               }
               
               case WAVE_SAW:
               {
                    sample = 2.0f * frac_phase - 1.0f;
                    break;
               }
               
               case WAVE_SQUARE:
               {
                    sample = (frac_phase < 0.5f) ? 1.0f : -1.0f;
                    break;
               }

               case WAVE_TRIANGLE:
               {
                    sample = 1.0f - 4.0f * std::fabs(frac_phase - 0.5f);
                    break;
               }

               case WAVE_NONE:
               default:
               {
                    sample = 0;
                    break;
               }
          }

          waveform_table[i] = sample * amplitude;
     }
}

void Oscillator::DumpWaveformToFile(const std::string& filename) const 
{
     std::ofstream out(filename);

     if (!out) return;  // erro se não abrir

     for (const auto& val : waveform_table) 
     {
          out << val << "\n";
     }

     out.close();
}

// end
