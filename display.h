#ifndef gbc_display_h
#define gbc_display_h

#define DISPLAY_SCALING 5.5
#define HEIGHT_PX 144
#define WIDTH_PX  160

#define FIFO_MAX_COUNT 8

#define CYCLES_PER_FRAME 70224
#define CYCLES_PER_SCANLINE 456
#define DEFAULT_FRAMERATE 63.7164

#define CYCLES_PER_MODE2 80
#define CYCLES_PER_VBLANK 4560

#ifndef USE_SDL2
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_video.h>


#include "emulator.h"

void SDL_events(Emulator* emulator);
void Sync_Display(Emulator* emulator, unsigned long cycles);
int initSDL(Emulator* emulator);
#endif

#endif
