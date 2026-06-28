
#include "SynthVoice.h"


/* DEBUG */
#include "../Logger/logger.h"
#define DEBUG_ENABLE 		1

#if DEBUG_ENABLE
	#define DEBUG(fmt, ...)   	Logger::Format("[voi] " fmt, ##__VA_ARGS__)
#else
	#define DEBUG(fmt, ...)
#endif


SynthVoice::SynthVoice() : gains{0.5f, 0.5f, 0.0f}, filter_cutoff(1.0f)
{
    //SetWaveType(1, Oscillator::SINE);
    SetGain(1, 0.7);

    //SetWaveType(2, Oscillator::TRIANGLE);
    SetGain(2, 0.7);

    //SetWaveType(3, 3);
    SetGain(3, 0.7);

    filter.SetCutoff(filter_cutoff);
}

void SynthVoice::NoteOn(int midi_note, float freq) 
{
    note = midi_note;

    for (auto& osc : oscillators) 
    {
        osc.SetFrequency(freq);
        osc.NoteOn();
    }
    
    active = true;
}

void SynthVoice::NoteOff() 
{
    for (auto& osc : oscillators) 
    {
        osc.NoteOff();
    }
    
    note = -1;
    active = false;
}

float SynthVoice::Process() 
{
    if (!active) return 0.0f;

    float out = 0.0f;
    float total_gain = 0.0f;

    for (size_t i = 0; i < NUM_OSC; ++i) 
    {
        if (gains[i] > 0.0f) 
        {
            out += gains[i] * oscillators[i].Process();
            total_gain += gains[i];
        }
    }

    if (total_gain == 0.0f) total_gain = 1.0f;
    
    out /= total_gain;

    // Aplica filtro (stateful para áudio realtime)
    out = filter.Process(out);

    return out;
}

bool SynthVoice::IsActive() const 
{ 
    return active;
}

int SynthVoice::GetMidiNote() const 
{ 
    return note; 
}


void SynthVoice::SetWaveType(int osc_num, Oscillator::WaveType wave_type)
{
    if (osc_num >= 1 && osc_num <= static_cast<int>(NUM_OSC)) 
    {
        oscillators[osc_num - 1].SetWaveType(wave_type);
    }

    // error handling (e.g., log or ignore invalid osc_num)
}

void SynthVoice::SetGain(int osc_num, float gain)
{
    if (osc_num >= 1 && osc_num <= static_cast<int>(NUM_OSC)) 
    {
        gains[osc_num - 1] = std::max(0.0f, std::min(1.0f, gain));  // Clamp entre 0 e 1 para proporções
    }

    // Optionally add error handling
}


void SynthVoice::SetFilterCutoff(float cutoff) 
{
    filter_cutoff = std::max(0.0f, std::min(1.0f, cutoff));
    filter.SetCutoff(filter_cutoff);
}


const std::array<float, TABLE_SIZE>& SynthVoice::GetWaveform(int osc_num) const 
{
    if (osc_num >= 1 && osc_num <= static_cast<int>(NUM_OSC)) 
    {
        return oscillators[osc_num - 1].GetWaveform();
    }
    
    static const std::array<float, TABLE_SIZE> empty{};
    
    return empty;
}

std::array<float, TABLE_SIZE> SynthVoice::GetMixedWaveform() const 
{
    std::array<float, TABLE_SIZE> mixed{};
    float total_gain = 0.0f;

    for (const auto& g : gains) 
    {
        if (g > 0.0f) total_gain += g;
    }
    
    if (total_gain == 0.0f) total_gain = 1.0f;

    // Gera mistura raw estática
    for (size_t j = 0; j < TABLE_SIZE; ++j) 
    {
        for (size_t i = 0; i < NUM_OSC; ++i) 
        {
            if (gains[i] > 0.0f) 
            {
                const auto& wf = oscillators[i].GetWaveform();
                float norm_gain = gains[i] / total_gain;
                mixed[j] += norm_gain * wf[j % wf.size()];  // % para repetir se necessário
            }
        }
    }

    // Aplica filtro para preview (usa cópia temp do filtro para não afetar áudio)
    LowPassFilter preview_filter = filter;  // cópia (estado reset implícito se novo)
    preview_filter.Reset();  // garante estado limpo

    for (size_t j = 0; j < TABLE_SIZE; ++j) 
    {
        mixed[j] = preview_filter.Process(mixed[j]);
    }

    return mixed;
}


