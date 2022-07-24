#ifndef gbc_cpu
#define gbc_cpu

#include "emulator.h"
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

void freeEmulator(Emulator* emulator);
void dispatch_emulator(Emulator* emulator);
void startEmulator(Cartridge* Cartridge, Emulator* emulator);

uint8_t read_address(Emulator* emulator, uint16_t address);

#endif
