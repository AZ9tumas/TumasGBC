#ifndef gbc_emu
#define gbc_emu

#include "common.h"
#include "cartridge.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>

#include <time.h>
#include <sys/time.h>

typedef struct {
    uint8_t *code;

    // CPU
    // - Registers
    uint8_t rA;
    uint8_t rF;

    uint8_t rB;
    uint8_t rC;

    uint8_t rD;
    uint8_t rE;
    
    uint8_t rH;
    uint8_t rL;

    uint16_t rPC;
    uint16_t rSP;

    // SDL stuff ...
    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    unsigned long start_time_ticks;
    unsigned long last_render_time;

    // - Other Stuff
    bool schedule_interrupt_enable;
    bool haltMode;
    bool schedule_halt_bug;  // imagine implementing bugs

    // Memory
    uint8_t VRAM[0x2000];
    uint8_t WRAM1[0x1000];
    uint8_t WRAM2[0x1000];
    uint8_t HRAM[0x7f];
    uint8_t IO[0x80];

    uint8_t* wramBanks;
    uint8_t* vramBanks;
    void* memController;
    
    uint8_t joypad_direction_buffer;
    uint64_t joypad_action_buffer;

    // Emulator
    Cartridge cartridge;
    
    bool IME;
    uint8_t IE;
    uint8_t IF;
    bool clock;
    bool run;

} Emulator;

Emulator* initEmulator(Emulator* emulator);
void initCartridgeEmulator(Emulator* emulator, Cartridge cartridge);
int initSDL(Emulator* emulator);
unsigned long getTime();

#endif