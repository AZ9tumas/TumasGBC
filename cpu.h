#ifndef gbc_cpu
#define gbc_cpu

#include "cartridge.h"
#include "common.h"

typedef enum {
    INTERRUPT_VBLANK,
    INTERRUPT_LCD_STAT,
    INTERRUPT_TIMER,
    INTERRUPT_SERIAL,
    INTERRUPT_JOYPAD,

    INTERRUPT_COUNT
} INTERRUPT;

typedef enum {
    FLAG_C,
    FLAG_H,
    FLAG_N,
    FLAG_Z
} FLAG;

typedef enum {
    ROM_N0_16KB = 0x0000,                       // 16KB ROM Bank number 0 (from cartridge)
    ROM_N0_16KB_END = 0x3FFF,

    ROM_NN_16KB = 0x4000,                       // 16KB Switchable ROM Bank area (from cartridge)
    ROM_NN_16KB_END = 0x7FFF,

    VRAM_N0_8KB = 0x8000,                       // 8KB Switchable vram
    VRAM_N0_8KB_END = 0x9FFF,

    RAM_NN_8KB = 0xA000,                        // 8KB Switchable RAM Bank area (from cartridge)
    RAM_NN_8KB_END = 0xBFFF,  

    WRAM_N0_4KB = 0xC000,                       // 4KB Work RAM
    WRAM_N0_4KB_END = 0xCFFF,

    WRAM_NN_4KB = 0xD000,                       // 4KB Switchable Work RAM (!cartridge)
    WRAM_NN_4KB_END = 0xDFFF,

    ECHO_N0_8KB = 0xE000,                       
    ECHO_N0_8KB_END = 0xFDFF,

    OAM_N0_160B = 0xFE00,                       // Where sprites (or screen objects) are stored
    OAM_N0_160B_END = 0xFE9F,
    
    UNUSABLE_N0 = 0xFEA0,                       
    UNUSABLE_N0_END = 0xFEFF,

    IO_REG = 0xFF00,                            // IO register
    IO_REG_END = 0xFF7F,

    HRAM_N0 = 0xFF80,                           // High Ram
    HRAM_N0_END = 0xFFFE, 

    INTERRUPT_ENABLE = 0xFFFF                   // interrupts register ...
} MEMORY_ADDRESS;

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

    // SDL stuff
    unsigned long start_time_ticks;
    unsigned long last_render_time;

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

typedef enum {
    REGISTER_A,
    REGISTER_F,

    REGISTER_B,
    REGISTER_C,

    REGISTER_D,
    REGISTER_E,
    
    REGISTER_H,
    REGISTER_L
} REGISTER_TYPE;

Emulator* initEmulator(Emulator* emulator);
void freeEmulator(Emulator* emulator);
void dispatch_emulator(Emulator* emulator);
void startEmulator(Cartridge* Cartridge, Emulator* emulator);

#endif
