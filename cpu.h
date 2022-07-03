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

typedef struct {
    uint8_t *code;

    /* Registers */
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

    Cartridge cartridge;

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
