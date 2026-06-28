
#pragma once

#include "../engine/SynthEngine.h"

#include "SDL2/SDL.h"
#include <string>


void Renderer__Init(SynthEngine* synth);
void Renderer__Poll(void);
bool Renderer__Keep(void);
void Renderer__Quit(void);


