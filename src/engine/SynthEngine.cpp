
#include "SynthEngine.h"
#include <cmath>


/* DEBUG */
#include "../Logger/logger.h"
#define DEBUG_ENABLE 		1

#if DEBUG_ENABLE
	#define DEBUG(fmt, ...)   	Logger::Format("[eng] " fmt, ##__VA_ARGS__)
#else
	#define DEBUG(fmt, ...)
#endif


// ===============================
// Constructor
// ===============================
SynthEngine::SynthEngine(int max_voices)
{
    voices.resize(max_voices);

    global_amplitude = 0.7f;  // valor inicial razoável

    for (uint8_t voice_idx = 0; voice_idx < max_voices; voice_idx++)
    {
        voices[voice_idx].SetWaveType(1, Oscillator::WAVE_SINE);
    }

    voices[0].SetWaveType(1, Oscillator::WAVE_SINE);
    
    voices[1].SetWaveType(1, Oscillator::WAVE_SAW);

    voices[2].SetWaveType(1, Oscillator::WAVE_SQUARE);

    voices[3].SetWaveType(1, Oscillator::WAVE_TRIANGLE);
}

// ===============================
// UI THREAD
// ===============================
void SynthEngine::PushEvent(int param, float value)
{
    uint32_t w = write_index.load(std::memory_order_relaxed);
    event_buffer[w % MAX_EVENTS] = { param, value };
    write_index.store(w + 1, std::memory_order_release);
}

void SynthEngine::SetParameter(int param, float value)
{
    PushEvent(param, value);
}

// ===============================
// AUDIO THREAD
// ===============================
void SynthEngine::ProcessEvents()
{
    uint32_t w = write_index.load(std::memory_order_acquire);

    while (read_index < w)
    {
        const Event& e = event_buffer[read_index % MAX_EVENTS];
        HandleParameter(e.param, e.value);
        read_index++;
    }
}

float SynthEngine::Process()
{
    ProcessEvents();

    float out = 0.0f;

    for (auto& v : voices) out += v.Process();
    
    // Curva em dB: 0.0 = -inf dB, 1.0 = 0 dB
    constexpr float MIN_GAIN_DB = -60.0f;  // ajusta: -80, -60, -40...
    float db = MIN_GAIN_DB + ( - MIN_GAIN_DB * global_amplitude );
    float master_gain = (global_amplitude <= 0.001f) ? 0.0f : std::pow(10.0f, db / 20.0f);

    return out * master_gain;
}

// ===================================
// PARAMETER HANDLER
// ===================================
void SynthEngine::HandleParameter(int param, float value)
{
    switch (param)
    {
        case PARAM_NOTE_ON:
        {
            InternalNoteOn((int)value);
            break;
        }
        
        case PARAM_NOTE_OFF:
        {
            InternalNoteOff((int)value);
            break;
        }

        case PARAM_GLOBAL_AMPLITUDE:
        {
            global_amplitude = value;
            break;
        }

        case PARAM_OSC1_WAVEFORM:
        case PARAM_OSC2_WAVEFORM:
        case PARAM_OSC3_WAVEFORM:
        {
            int osc_index = (param - PARAM_OSC1_WAVEFORM); // 0,1,2,...

            if (osc_index >= 0 && osc_index < 3) 
            {
                for (auto& voice : voices) 
                {
                    voice.SetWaveType(osc_index + 1, static_cast<Oscillator::WaveType>(value));
                }
            }

            break;
        }

        case PARAM_FILTER1_CUTOFF:
        {
            voices[0].SetFilterCutoff(value);

            break;
        }

        default:
        {
            /*
            for (auto& v : voices) 
            {
                v.SetParameter(param, value);
            }
            */

            break;
        }
    }
}

// ===============================
// Internal Voice Control
// ===============================
static float MidiToFreq(int note)
{
    return 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
}

void SynthEngine::InternalNoteOn(int midi_note)
{
    float freq = MidiToFreq(midi_note);

    for (auto& v : voices)
    {
        if (!v.IsActive())
        {
            v.NoteOn(midi_note, freq);
            return;
        }
    }

    // Voice stealing simples
    voices[0].NoteOn(midi_note, freq);
}

void SynthEngine::InternalNoteOff(int midi_note)
{
    for (auto& v : voices)
    {
        if (v.IsActive() && v.GetMidiNote() == midi_note)
        {
            v.NoteOff();
        }
    }
}

// ===============================
// UI Waveform Snapshot
// ===============================
std::vector<const std::array<float, TABLE_SIZE>*> SynthEngine::GetWaveforms() const
{
    std::vector<const std::array<float, TABLE_SIZE>*> waves;
    static std::vector<std::array<float, TABLE_SIZE>> mixed_buffers;  // static normal

    mixed_buffers.clear();
    mixed_buffers.reserve(voices.size());

    for (const auto& v : voices)
    {
        if (v.IsActive())
        {
            mixed_buffers.push_back(v.GetMixedWaveform());
            waves.push_back(&mixed_buffers.back());
        }
    }

    return waves;
}


