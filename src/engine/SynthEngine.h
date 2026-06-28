
#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <memory>

#include "SynthVoice.h"

class SynthEngine
{
public:
    SynthEngine(int max_voices = 8);

    // ---- UI API ----

    enum ParameterID
    {
        // --- Eventos tipo botão ---
        PARAM_NOTE_ON,
        PARAM_NOTE_OFF,

        PARAM_OSC1_WAVEFORM,
        PARAM_OSC2_WAVEFORM,
        PARAM_OSC3_WAVEFORM,

        PARAM_GLOBAL_AMPLITUDE,

        PARAM_FILTER1_CUTOFF,

        // etc ...
    };

    void SetParameter(int param_id, float value); // extensível

    // ---- AUDIO THREAD ----
    float Process();

    // ---- UI (read-only snapshot) ----
    std::vector<const std::array<float, TABLE_SIZE>*> GetWaveforms() const;

private:
    // ===============================
    // EVENT SYSTEM
    // ===============================
    struct Event
    {
        int param;
        float value;
    };

    static constexpr size_t MAX_EVENTS = 100;

    std::array<Event, MAX_EVENTS> event_buffer;
    std::atomic<uint32_t> write_index {0};   // UI escreve
    uint32_t read_index = 0;                 // Audio lê

    void PushEvent(int param, float value);
    void ProcessEvents(); // chamado no audio thread

    // ===============================
    // ENGINE STATE (AUDIO THREAD ONLY)
    // ===============================

    std::vector<SynthVoice> voices;
    float global_amplitude = 1.0f;

    void HandleParameter(int param, float value);
    void InternalNoteOn(int midi_note);
    void InternalNoteOff(int midi_note);
};


