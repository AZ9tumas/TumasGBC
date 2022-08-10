#include "emulator.h"
#include <sys/time.h>

void initCartridgeEmulator(Emulator* emulator, Cartridge cartridge){
    emulator->cartridge = cartridge;
    emulator->cartridge.inserted = true;
}

Emulator* initEmulator(Emulator* emulator){
    emulator->rA = 0x11;
    emulator->rF = 0b10000000;
    emulator->rB = 0x00;
    emulator->rC = 0x00;
    emulator->rD = 0xFF;
    emulator->rE = 0x56;
    emulator->rH = 0x00;
    emulator->rL = 0x0D;
    emulator->rSP = 0xfffe;
    emulator->rPC = 0x0101;

    emulator->schedule_halt_bug = false;
    emulator->schedule_interrupt_enable = false;
    emulator->haltMode = false;

    emulator->run = false;

    emulator->clock = 0;
    emulator->start_time_ticks = 0;
    emulator->last_render_time = 0;
    emulator->ppuEnabled = true;

    emulator->cyclesFromLastFrame = 0;
    emulator->cyclesFromLastMode = 0;

    emulator->joypad_action = 0xF;
    emulator->joypad_direction = 0xF;

    emulator->IME = false;
    emulator->IE = 0x00;
    emulator->IF = 0x00;

    return emulator;
}

unsigned long getTime() {
	struct timeval current_time;
    gettimeofday(&current_time, NULL);

    return (current_time.tv_sec * 1000000) + current_time.tv_usec;
}

void endRun(Emulator* emulator){    
    /* Free SDL */
    #ifndef USE_SDL2
    SDL_DestroyRenderer(emulator->sdl_renderer);
    SDL_DestroyWindow(emulator->sdl_window);
    SDL_Quit();
    #endif
    initEmulator(emulator);
}
