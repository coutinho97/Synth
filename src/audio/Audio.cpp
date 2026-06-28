
#include "Audio.h"
#include "../engine/SynthEngine.h"
#include <portaudio.h>

static PaStream* stream = nullptr;
static SynthEngine* synth = nullptr;

static int audioCallback(const void* input, void* output, unsigned long frame_count, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, void* user_data)
{
     (void)input;
     (void)time_info;
     (void)status_flags;

     float* out = (float*)output;
     SynthEngine* synth = (SynthEngine*)user_data;

     for (unsigned long i = 0; i < frame_count; i++)
     {
          float sample = synth->Process();

          (*out++) = sample; // L
          (*out++) = sample; // R
     }

     return paContinue;
}

void Audio__Init(SynthEngine* engine)
{
     synth = engine;
     Pa_Initialize();
     Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, 44100, 256, audioCallback, synth);
     Pa_StartStream(stream);
}

void Audio__Quit()
{
     Pa_StopStream(stream);
     Pa_CloseStream(stream);
     Pa_Terminate();
}