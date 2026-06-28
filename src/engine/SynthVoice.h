
#pragma once

#include "Oscillator.h"
#include "Filter.h"

static constexpr size_t NUM_OSC = 4;  // Configurable number of oscillators (more than one)

class SynthVoice 
{
public:
     SynthVoice();
     void NoteOn(int midi_note, float freq);
     void NoteOff();
     float Process();
     bool IsActive() const;
     int GetMidiNote() const;
     
     void SetWaveType(int osc_num, Oscillator::WaveType wave_type);   // osc_num starts from 1
     void SetGain(int osc_num, float gain);                           // New method to set gain for each oscillator
     
     void SetFilterCutoff(float cutoff);
     
     const std::array<float, TABLE_SIZE>& GetWaveform(int osc_num) const;
     std::array<float, TABLE_SIZE> GetMixedWaveform() const;

private:
     bool active = false;
     int note = -1;

     std::array<Oscillator, NUM_OSC> oscillators;
     std::array<float, NUM_OSC> gains;

     LowPassFilter filter;         // um por voice
     float filter_cutoff = 1.0f;   // default: no filter (aberto)
};

