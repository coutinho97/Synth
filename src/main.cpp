
#include "logger/logger.h"
#include "engine/SynthEngine.h"
#include "audio/Audio.h"

// inputs
#include "midi/MidiSerial.h"
#include <conio.h> // _getch _kbhit


/* sdl_main */
#include "SDL2/SDL.h"
#include "renderer/renderer.h"


// global variables
static LogType log_type = LogType::Console;
static SynthEngine* synth;


/*
 * functions prototypes
 */
void ProcessConsoleInput();

void MidiCallbck(int channel, int cc_number, int value);


/* 
 * main(int argc, char **argv) 
 */
int main(int argc, char **argv) 
{
     (void)argc;
     (void)argv;

     Logger::Init(log_type);

     synth = new SynthEngine();

     if (synth == NULL)  
     {
          Logger::Format("Error creating synth object");
     }

     Audio__Init(synth);

     Renderer__Init(synth);


     MidiSerial midi_serial;

     if (midi_serial.Open("COM10", 31250)) 
     {
          midi_serial.SetControlChangeCallback(MidiCallbck);
          midi_serial.StartReading();
     }


     while (Renderer__Keep()) 
     {
          Renderer__Poll();
          ProcessConsoleInput();
     }
     

     midi_serial.Close();
     Renderer__Quit();
     Audio__Quit();
     Logger::Close();
     delete synth;

     return 0;
}

/* 
 * Funct
 */
void ProcessConsoleInput()
{
     if (log_type != LogType::Console) return;

     if (!_kbhit()) return;

     int ch = _getch();

     Logger::Format("%c", ch);

     switch (ch)
     {    
          case '1': synth->SetParameter(SynthEngine::PARAM_OSC1_WAVEFORM, Oscillator::WAVE_SINE); break;
          case '2': synth->SetParameter(SynthEngine::PARAM_OSC1_WAVEFORM, Oscillator::WAVE_SAW); break;
          case '3': synth->SetParameter(SynthEngine::PARAM_OSC1_WAVEFORM, Oscillator::WAVE_SQUARE); break;
          case '4': synth->SetParameter(SynthEngine::PARAM_OSC1_WAVEFORM, Oscillator::WAVE_TRIANGLE); break;

          case ' ': /* note toggle */ break;
          //case 27:  running = false; break;
     }
}

void MidiCallbck(int channel, int cc_number, int value)
{
     (void)cc_number;
     //Logger::Format("ch: %d, cc: %d, val: %d", channel, cc_number, value);

     float normalized_value = value / 127.0f;

     switch (channel)
     {
          case 0:   // pot
          {
               Logger::Format("pot: %.2f", normalized_value);

               //synth->SetParameter(SynthEngine::PARAM_GLOBAL_AMPLITUDE, normalized_value);
               synth->SetParameter(SynthEngine::PARAM_FILTER1_CUTOFF, normalized_value);

               break;
          }

          case 1:   // btn 
          {
               synth->SetParameter(SynthEngine::PARAM_GLOBAL_AMPLITUDE, 1);

               if (normalized_value)
               {
                    synth->SetParameter(SynthEngine::PARAM_NOTE_ON, 60);
               }
               else
               {
                    synth->SetParameter(SynthEngine::PARAM_NOTE_OFF, 60);
               }

               break;
          }

          default:
          {
               break;
          }
     }
}


