
#include "renderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>


/* DEBUG */
#include "../Logger/logger.h"

#define DEBUG_ENABLE 		(0)

#if DEBUG_ENABLE
	#define DEBUG(fmt, ...)   	Logger::Format("[Render] " fmt, ##__VA_ARGS__)
#else
	#define DEBUG(fmt, ...)
#endif


#define SIZE 					640
#define SQUARE_SIZE 			(SIZE/8)

#define FONT_PATH     		"src\\assets\\mono_regular.ttf"


static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static TTF_Font* font = nullptr;

static bool exit_flag = false;

static SynthEngine* synth = nullptr;


// Array de cores fortes e distintas (RGB 0-255)
// Ordem: vermelho vivo, ciano elétrico, magenta forte, verde limão, laranja queimado, roxo neon, amarelo brilhante...
static const SDL_Color voice_colors[] = {
	{255,   0,   0, 255},   // R
	{0,   255,   0, 255},   // G
	{  0,   0, 255, 255},   // B 

	{255, 255,   0, 255},   // Amarelo puro           (R + G)
    	{  0, 255, 255, 255},   // Ciano puro             (G + B)
    	{255,   0, 255, 255},   // Magenta puro           (R + B)

	{255, 128,   0, 255},   // Laranja forte          (R + metade G)
	{255,   0, 128, 255},   // Rosa/Magenta avermelhado (R + metade B)
	{128, 255,   0, 255},   // Verde-limão            (G + metade R)
	{  0, 255, 128, 255},   // Verde-água / Turquesa clara (G + metade B)
	{128,   0, 255, 255},   // Violeta/roxo forte     (B + metade R)
	{  0, 128, 255, 255},   // Azul-cobalto / Azul elétrico (B + metade G)

	{255, 180,  60, 255},   // Laranja-amarelo vivo   (R + G forte + pouco B)
	{180, 255,  60, 255},   // Verde-amarelado limão  (G + R médio)
	{255,  60, 180, 255},   // Rosa quente            (R + B médio)
};


/*
    PRIVATE
*/
static int KeyToMidi(SDL_Keycode key) 
{
	switch(key) 
	{
		case SDLK_a: return 60; // C4
		case SDLK_s: return 62;
		case SDLK_d: return 64;
		case SDLK_f: return 65;
		case SDLK_g: return 67;
		case SDLK_h: return 69;
		case SDLK_j: return 71;
		case SDLK_k: return 72; // C5
		default: return -1;
	}
}

static void DrawWaveForm()
{
	if (!renderer || !synth) return;

	auto waves = synth->GetWaveforms();

	// === Configurações centradas na janela de 640×640 ===
	const int WINDOW_WIDTH  = SIZE;               // 640
	const int WINDOW_HEIGHT = SIZE;               // 640 (assumindo quadrado)

	const int MARGIN        = 40;                 // margem esquerda/direita/topo/fundo
	const int WAVE_WIDTH    = WINDOW_WIDTH - 2 * MARGIN;   // ex: 640 - 80 = 560 pixels
	const int WAVE_X_START  = MARGIN;             // começa simetricamente à esquerda
	const int WAVE_Y_CENTER = WINDOW_HEIGHT / 2;  // 320 (centro vertical da janela)

	const float WAVE_AMPLITUDE_SCALE = 180.0f;    // mantém a tua escala de amplitude

	// Fundo opcional para o widget (para destacar)
	SDL_SetRenderDrawColor(renderer, 30, 30, 40, 180);
	SDL_Rect bg_rect = { WAVE_X_START - 10, WAVE_Y_CENTER - 220, WAVE_WIDTH + 20, 440 };
	SDL_RenderFillRect(renderer, &bg_rect);


	for (size_t voice_idx = 0; const auto* wave_ptr : waves)
	{
		if (!wave_ptr) continue;
		const auto& wave = *wave_ptr;

		const size_t table_size = wave.size();
		if (table_size < 2) continue;

		// Escolhe cor com base no índice da voz
		const SDL_Color& color = voice_colors[voice_idx % (sizeof(voice_colors) / sizeof(voice_colors[0]))];

		// Aplica a cor antes de desenhar esta waveform
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

		for (size_t i = 0; i < table_size - 1; ++i)
		{
			// Posição X proporcional à largura disponível
			int x1 = WAVE_X_START + static_cast<int>((static_cast<float>(i)       * WAVE_WIDTH) / (table_size - 1));
			int x2 = WAVE_X_START + static_cast<int>((static_cast<float>(i + 1)   * WAVE_WIDTH) / (table_size - 1));

			float s1 = wave[i];
			float s2 = wave[i + 1];

			int y1 = WAVE_Y_CENTER - static_cast<int>(s1 * WAVE_AMPLITUDE_SCALE);
			int y2 = WAVE_Y_CENTER - static_cast<int>(s2 * WAVE_AMPLITUDE_SCALE);

			SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
		}

		voice_idx++;
	}

	// Linha central (referência zero)
	SDL_SetRenderDrawColor(renderer, 80, 80, 90, 200);
	SDL_RenderDrawLine(renderer, WAVE_X_START, WAVE_Y_CENTER, WAVE_X_START + WAVE_WIDTH, WAVE_Y_CENTER);

	// Bordas do widget (opcional, fica bonito)
	SDL_SetRenderDrawColor(renderer, 120, 140, 180, 140);
	SDL_RenderDrawRect(renderer, &bg_rect);
}

// função de texto (adiciona em qualquer sitio do ficheiro)
static void Renderer__DrawText(const char* text, int x, int y, SDL_Color color, TTF_Font* font) 
{
	if (!font || !text) return;

	SDL_Surface* surf = TTF_RenderText_Blended(font, text, color);
	
	if (!surf) return;
	
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_Rect dst = {x, y, surf->w, surf->h};
	SDL_RenderCopy(renderer, tex, nullptr, &dst);
	SDL_DestroyTexture(tex);
	SDL_FreeSurface(surf);
}



/*
    PUBLIC
*/
void Renderer__Init(SynthEngine* s)
{
	synth = s;

	if(SDL_Init(SDL_INIT_VIDEO)) 
	{
		DEBUG("There was a problem initializing SDL: %s", SDL_GetError());
		exit(1);
	}

	window = SDL_CreateWindow("Synth", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SIZE, SIZE, SDL_WINDOW_SHOWN);

	if(!window) 
	{
		DEBUG("There was an error creating the window: %s", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if(!renderer) 
	{
		DEBUG("There was an error creating the window: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(1);
	}


	if(TTF_Init())
	{
		DEBUG("There was an error on TTF Init: %s", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(1);
	}

	font = TTF_OpenFont(FONT_PATH, 16);
	
	if (!font)
	{
		DEBUG("There was an error opening font");
		DEBUG("Couldn't load %s | TTF_OpenFont error: %s", FONT_PATH, TTF_GetError());
	}
}

void Renderer__Poll()
{	
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
			{
				exit_flag = true;
				break;
			}

			case SDL_KEYDOWN:
			{
				if (!event.key.repeat)
				{
					int note = KeyToMidi(event.key.keysym.sym);

					if (note != -1) 
					{
						synth->SetParameter(SynthEngine::PARAM_NOTE_ON, note);
						DEBUG("NoteOn %d", note);
					}
				}

				break;
			}

			case SDL_KEYUP:
			{
				int note = KeyToMidi(event.key.keysym.sym);

				if (note != -1) 
				{
					synth->SetParameter(SynthEngine::PARAM_NOTE_OFF, note);
					DEBUG("NoteOff %d", note);
				}

				break;
			}

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
			{

				break;
			}

			default:
			{
				break;
			}
		}
	}

	// ---------- Render ----------
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	DrawWaveForm();

	SDL_RenderPresent(renderer);
}

bool Renderer__Keep(void)
{
	return !exit_flag;
}

void Renderer__Quit()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


