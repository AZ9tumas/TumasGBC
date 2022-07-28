#include "display.h"

static void lockFPS(Emulator* emulator){
    unsigned long elapsed_ticks = (getTime() - emulator->start_time_ticks);
    //usleep((1e6/DEFAULT_FRAMERATE) - elapsed_ticks);
    emulator->last_render_time = getTime() - emulator->start_time_ticks;
}

void SDL_events(Emulator* emulator){
    SDL_Event event;
    
    while (SDL_PollEvent(&event)){
        if (event.type == SDL_KEYDOWN && event.key.repeat == 0){
            switch (event.key.keysym.scancode){
                case SDL_SCANCODE_UP: printf("UP KEY down\n");       break;
                case SDL_SCANCODE_LEFT: printf("LEFT KEY down\n");   break;
                case SDL_SCANCODE_DOWN: printf("DOWN KEY down\n");   break;
				case SDL_SCANCODE_RIGHT: printf("RIGHT KEY down\n"); break;
                default: return;
            }
        } else if (event.type == SDL_KEYUP && event.key.repeat == 0){
            switch (event.key.keysym.scancode){
                case SDL_SCANCODE_UP: printf("UP KEY up\n");       break;
                case SDL_SCANCODE_LEFT: printf("LEFT KEY up\n");   break;
                case SDL_SCANCODE_DOWN: printf("DOWN KEY up\n");   break;
				case SDL_SCANCODE_RIGHT: printf("RIGHT KEY up\n"); break;
                default: return;
            }
        } else if (event.type == SDL_QUIT){
            emulator->run = false;
        }
    }
}

int initSDL(Emulator* emulator){
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WIDTH_PX * DISPLAY_SCALING, HEIGHT_PX * DISPLAY_SCALING, SDL_WINDOW_SHOWN, &emulator->sdl_window, &emulator->sdl_renderer);

    if (!emulator->sdl_window) return 1;
    SDL_SetWindowTitle(emulator->sdl_window, "Tumas Emulator");
    SDL_RenderSetScale(emulator->sdl_renderer, DISPLAY_SCALING, DISPLAY_SCALING);

    return 0;
}

static void advancePPU(Emulator* emulator){
    emulator->cyclesFromLastMode ++;
    
}

void Sync_Display(Emulator* emulator, unsigned long cycles) {
    
}
