#ifndef debug_h
#define debug_h
#include "cpu.h"

// #define DEBUG_NO_CARTRIDGE_VERIFICATION
// #define DEBUG_REALTIME_PRINTING
// #define DEBUG_PRINT_REGISTERS
// #define DEBUG_PRINT_TIMERS
// #define DEBUG_PRINT_CYCLES
// #define DEBUG_PRINT_FLAGS
// #define DEBUG_PRINT_ADDRESS
// #define DEBUG_PRINT_PPU
// #define DEBUG_PRINT_JOYPAD_REG

// #define DEBUG_PRINT_CARTRIDGE_INFO
// #define DEBUG_LOGGING
// #define DEBUG_MEM_LOGGING
#define DEBUG_PRINT_SERIAL_OUTPUT
#define DEBUG_SUPPORT_SLOW_EMULATION

/* Stops running when encounters opcode 0x40, LD B, B */
// #define DEBUG_LDBB_BREAKPOINT

#ifdef DEBUG_REALTIME_PRINTING
#define DEBUG_SUPPORT_SLOW_EMULATION
#endif

void printFlags(Emulator* emulator);
void printInstruction(Emulator* emulator);
void printRegisters(Emulator* emulator);
void printCBInstruction(Emulator* emulator, uint8_t byte);
void log_fatal(Emulator* emulator, const char* string);
void log_warning(Emulator* emulator, const char* string);
#endif
