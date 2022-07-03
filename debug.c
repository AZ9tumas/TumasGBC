#include <stdio.h>
#include "debug.h"

void log_fatal(Emulator* emulator, const char* string) {
    printf("[FATAL]");
    printf(" %s", string);
    printf("\n");

    //stopEmulator(emulator);
    exit(99);
}

void log_warning(Emulator* emulator, const char* string) {
    printf("[WARNING]");
    printf(" %s", string);
    printf("\n");
}

static uint16_t read2Bytes(Emulator* emulator) {
    uint8_t b1 = emulator->cartridge.file[emulator->rPC + 1];
    uint8_t b2 = emulator->cartridge.file[emulator->rPC + 2];
    uint16_t D16 = (b2 << 8) | b1;
    return D16;
}

static void printFlags(Emulator* emulator) {
    uint8_t flagState = emulator->rF;
    
    printf("[Z%d", flagState >> 7);
    printf(" N%d", (flagState >> 6) & 1);
    printf(" H%d", (flagState >> 5) & 1);
    printf(" C%d]", (flagState >> 4) & 1);
}

static void simpleInstruction(Emulator* emulator, char* ins) {
    printf("%s\n", ins);;
}

static void d16(Emulator* emulator, char* ins) {
    printf("%s (0x%04x)\n", ins, read2Bytes(emulator));
}

static void d8(Emulator* emulator, char* ins) {
    printf("%s (0x%02x)\n", ins, emulator->cartridge.file[emulator->rPC + 1]);
}

static void a16(Emulator* emulator, char* ins) {
    printf("%s (0x%04x)\n", ins, read2Bytes(emulator));
}

static void r8(Emulator* emulator, char* ins) {
    printf("%s (%d)\n", ins, (int8_t)emulator->cartridge.file[emulator->rPC + 1]);
}

void printCBInstruction(Emulator* emulator, uint8_t byte) {
#ifdef DEBUG_PRINT_ADDRESS
    printf("[0x%04x]", emulator->PC - 1);
#endif
#ifdef DEBUG_PRINT_FLAGS
    printFlags(emulator);
#endif
#ifdef DEBUG_PRINT_CYCLES
    printf("[%ld]", emulator->clock);
#endif
#ifdef DEBUG_PRINT_JOYPAD_REG
    printf("[sel:%x|", (emulator->MEM[R_P1_JOYP] >> 4) & 0x3);
    printf("sig:%x]", (emulator->MEM[R_P1_JOYP] & 0b00001111));
#endif
#ifdef DEBUG_PRINT_TIMERS
	printf("[%x|%x|%x|%x]", emulator->MEM[R_DIV], emulator->MEM[R_TIMA], emulator->MEM[R_TMA], emulator->MEM[R_TAC]);
#endif
    printf(" %5s", "");

    switch (byte) {
        case 0x00: return simpleInstruction(emulator, "RLC B");
        case 0x01: return simpleInstruction(emulator, "RLC C");
        case 0x02: return simpleInstruction(emulator, "RLC D");
        case 0x03: return simpleInstruction(emulator, "RLC E");
        case 0x04: return simpleInstruction(emulator, "RLC H");
        case 0x05: return simpleInstruction(emulator, "RLC L");
        case 0x06: return simpleInstruction(emulator, "RLC (HL)");
        case 0x07: return simpleInstruction(emulator, "RLC A");
        case 0x08: return simpleInstruction(emulator, "RRC B");
        case 0x09: return simpleInstruction(emulator, "RRC C");
        case 0x0A: return simpleInstruction(emulator, "RRC D");
        case 0x0B: return simpleInstruction(emulator, "RRC E");
        case 0x0C: return simpleInstruction(emulator, "RRC H");
        case 0x0D: return simpleInstruction(emulator, "RRC L");
        case 0x0E: return simpleInstruction(emulator, "RRC (HL)");
        case 0x0F: return simpleInstruction(emulator, "RRC A");
        case 0x10: return simpleInstruction(emulator, "RL B");
        case 0x11: return simpleInstruction(emulator, "RL C");
        case 0x12: return simpleInstruction(emulator, "RL D");
        case 0x13: return simpleInstruction(emulator, "RL E");
        case 0x14: return simpleInstruction(emulator, "RL H");
        case 0x15: return simpleInstruction(emulator, "RL L");
        case 0x16: return simpleInstruction(emulator, "RL (HL)");
        case 0x17: return simpleInstruction(emulator, "RL A");
        case 0x18: return simpleInstruction(emulator, "RR B");
        case 0x19: return simpleInstruction(emulator, "RR C");
        case 0x1A: return simpleInstruction(emulator, "RR D");
        case 0x1B: return simpleInstruction(emulator, "RR E");
        case 0x1C: return simpleInstruction(emulator, "RR H");
        case 0x1D: return simpleInstruction(emulator, "RR L");
        case 0x1E: return simpleInstruction(emulator, "RR (HL)");
        case 0x1F: return simpleInstruction(emulator, "RR A");
        case 0x20: return simpleInstruction(emulator, "SLA B");
        case 0x21: return simpleInstruction(emulator, "SLA C");
        case 0x22: return simpleInstruction(emulator, "SLA D");
        case 0x23: return simpleInstruction(emulator, "SLA E");
        case 0x24: return simpleInstruction(emulator, "SLA H");
        case 0x25: return simpleInstruction(emulator, "SLA L");
        case 0x26: return simpleInstruction(emulator, "SLA (HL)");
        case 0x27: return simpleInstruction(emulator, "SLA A");
        case 0x28: return simpleInstruction(emulator, "SRA B");
        case 0x29: return simpleInstruction(emulator, "SRA C");
        case 0x2A: return simpleInstruction(emulator, "SRA D");
        case 0x2B: return simpleInstruction(emulator, "SRA E");
        case 0x2C: return simpleInstruction(emulator, "SRA H");
        case 0x2D: return simpleInstruction(emulator, "SRA L");
        case 0x2E: return simpleInstruction(emulator, "SRA (HL)");
        case 0x2F: return simpleInstruction(emulator, "SRA A");
        case 0x30: return simpleInstruction(emulator, "SWAP B");
        case 0x31: return simpleInstruction(emulator, "SWAP C");
        case 0x32: return simpleInstruction(emulator, "SWAP D");
        case 0x33: return simpleInstruction(emulator, "SWAP E");
        case 0x34: return simpleInstruction(emulator, "SWAP H");
        case 0x35: return simpleInstruction(emulator, "SWAP L");
        case 0x36: return simpleInstruction(emulator, "SWAP (HL)");
        case 0x37: return simpleInstruction(emulator, "SWAP A");
        case 0x38: return simpleInstruction(emulator, "SRL B");
        case 0x39: return simpleInstruction(emulator, "SRL C");
        case 0x3A: return simpleInstruction(emulator, "SRL D");
        case 0x3B: return simpleInstruction(emulator, "SRL E");
        case 0x3C: return simpleInstruction(emulator, "SRL H");
        case 0x3D: return simpleInstruction(emulator, "SRL L");
        case 0x3E: return simpleInstruction(emulator, "SRL (HL)");
        case 0x3F: return simpleInstruction(emulator, "SRL A");
        case 0x40: return simpleInstruction(emulator, "BIT 0, B");
        case 0x41: return simpleInstruction(emulator, "BIT 0, C");
        case 0x42: return simpleInstruction(emulator, "BIT 0, D");
        case 0x43: return simpleInstruction(emulator, "BIT 0, E");
        case 0x44: return simpleInstruction(emulator, "BIT 0, H");
        case 0x45: return simpleInstruction(emulator, "BIT 0, L");
        case 0x46: return simpleInstruction(emulator, "BIT 0, (HL)");
        case 0x47: return simpleInstruction(emulator, "BIT 0, A");
        case 0x48: return simpleInstruction(emulator, "BIT 1, B");
        case 0x49: return simpleInstruction(emulator, "BIT 1, C");
        case 0x4A: return simpleInstruction(emulator, "BIT 1, D");
        case 0x4B: return simpleInstruction(emulator, "BIT 1, E");
        case 0x4C: return simpleInstruction(emulator, "BIT 1, H");
        case 0x4D: return simpleInstruction(emulator, "BIT 1, L");
        case 0x4E: return simpleInstruction(emulator, "BIT 1, (HL)");
        case 0x4F: return simpleInstruction(emulator, "BIT 1, A");
        case 0x50: return simpleInstruction(emulator, "BIT 2, B");
        case 0x51: return simpleInstruction(emulator, "BIT 2, C");
        case 0x52: return simpleInstruction(emulator, "BIT 2, D");
        case 0x53: return simpleInstruction(emulator, "BIT 2, E");
        case 0x54: return simpleInstruction(emulator, "BIT 2, H");
        case 0x55: return simpleInstruction(emulator, "BIT 2, L");
        case 0x56: return simpleInstruction(emulator, "BIT 2, (HL)");
        case 0x57: return simpleInstruction(emulator, "BIT 2, A");
        case 0x58: return simpleInstruction(emulator, "BIT 3, B");
        case 0x59: return simpleInstruction(emulator, "BIT 3, C");
        case 0x5A: return simpleInstruction(emulator, "BIT 3, D");
        case 0x5B: return simpleInstruction(emulator, "BIT 3, E");
        case 0x5C: return simpleInstruction(emulator, "BIT 3, H");
        case 0x5D: return simpleInstruction(emulator, "BIT 3, L");
        case 0x5E: return simpleInstruction(emulator, "BIT 3, (HL)");
        case 0x5F: return simpleInstruction(emulator, "BIT 3, A");
        case 0x60: return simpleInstruction(emulator, "BIT 4, B");
        case 0x61: return simpleInstruction(emulator, "BIT 4, C");
        case 0x62: return simpleInstruction(emulator, "BIT 4, D");
        case 0x63: return simpleInstruction(emulator, "BIT 4, E");
        case 0x64: return simpleInstruction(emulator, "BIT 4, H");
        case 0x65: return simpleInstruction(emulator, "BIT 4, L");
        case 0x66: return simpleInstruction(emulator, "BIT 4, (HL)");
        case 0x67: return simpleInstruction(emulator, "BIT 4, A");
        case 0x68: return simpleInstruction(emulator, "BIT 5, B");
        case 0x69: return simpleInstruction(emulator, "BIT 5, C");
        case 0x6A: return simpleInstruction(emulator, "BIT 5, D");
        case 0x6B: return simpleInstruction(emulator, "BIT 5, E");
        case 0x6C: return simpleInstruction(emulator, "BIT 5, H");
        case 0x6D: return simpleInstruction(emulator, "BIT 5, L");
        case 0x6E: return simpleInstruction(emulator, "BIT 5, (HL)");
        case 0x6F: return simpleInstruction(emulator, "BIT 5, A");
        case 0x70: return simpleInstruction(emulator, "BIT 6, B");
        case 0x71: return simpleInstruction(emulator, "BIT 6, C");
        case 0x72: return simpleInstruction(emulator, "BIT 6, D");
        case 0x73: return simpleInstruction(emulator, "BIT 6, E");
        case 0x74: return simpleInstruction(emulator, "BIT 6, H");
        case 0x75: return simpleInstruction(emulator, "BIT 6, L");
        case 0x76: return simpleInstruction(emulator, "BIT 6, (HL)");
        case 0x77: return simpleInstruction(emulator, "BIT 6, A");
        case 0x78: return simpleInstruction(emulator, "BIT 7, B");
        case 0x79: return simpleInstruction(emulator, "BIT 7, C");
        case 0x7A: return simpleInstruction(emulator, "BIT 7, D");
        case 0x7B: return simpleInstruction(emulator, "BIT 7, E");
        case 0x7C: return simpleInstruction(emulator, "BIT 7, H");
        case 0x7D: return simpleInstruction(emulator, "BIT 7, L");
        case 0x7E: return simpleInstruction(emulator, "BIT 7, (HL)");
        case 0x7F: return simpleInstruction(emulator, "BIT 7, A");
        case 0x80: return simpleInstruction(emulator, "RES 0, B");
        case 0x81: return simpleInstruction(emulator, "RES 0, C");
        case 0x82: return simpleInstruction(emulator, "RES 0, D");
        case 0x83: return simpleInstruction(emulator, "RES 0, E");
        case 0x84: return simpleInstruction(emulator, "RES 0, H");
        case 0x85: return simpleInstruction(emulator, "RES 0, L");
        case 0x86: return simpleInstruction(emulator, "RES 0, (HL)");
        case 0x87: return simpleInstruction(emulator, "RES 0, A");
        case 0x88: return simpleInstruction(emulator, "RES 1, B");
        case 0x89: return simpleInstruction(emulator, "RES 1, C");
        case 0x8A: return simpleInstruction(emulator, "RES 1, D");
        case 0x8B: return simpleInstruction(emulator, "RES 1, E");
        case 0x8C: return simpleInstruction(emulator, "RES 1, H");
        case 0x8D: return simpleInstruction(emulator, "RES 1, L");
        case 0x8E: return simpleInstruction(emulator, "RES 1, (HL)");
        case 0x8F: return simpleInstruction(emulator, "RES 1, A");
        case 0x90: return simpleInstruction(emulator, "RES 2, B");
        case 0x91: return simpleInstruction(emulator, "RES 2, C");
        case 0x92: return simpleInstruction(emulator, "RES 2, D");
        case 0x93: return simpleInstruction(emulator, "RES 2, E");
        case 0x94: return simpleInstruction(emulator, "RES 2, H");
        case 0x95: return simpleInstruction(emulator, "RES 2, L");
        case 0x96: return simpleInstruction(emulator, "RES 2, (HL)");
        case 0x97: return simpleInstruction(emulator, "RES 2, A");
        case 0x98: return simpleInstruction(emulator, "RES 3, B");
        case 0x99: return simpleInstruction(emulator, "RES 3, C");
        case 0x9A: return simpleInstruction(emulator, "RES 3, D");
        case 0x9B: return simpleInstruction(emulator, "RES 3, E");
        case 0x9C: return simpleInstruction(emulator, "RES 3, H");
        case 0x9D: return simpleInstruction(emulator, "RES 3, L");
        case 0x9E: return simpleInstruction(emulator, "RES 3, (HL)");
        case 0x9F: return simpleInstruction(emulator, "RES 3, A");
        case 0xA0: return simpleInstruction(emulator, "RES 4, B");
        case 0xA1: return simpleInstruction(emulator, "RES 4, C");
        case 0xA2: return simpleInstruction(emulator, "RES 4, D");
        case 0xA3: return simpleInstruction(emulator, "RES 4, E");
        case 0xA4: return simpleInstruction(emulator, "RES 4, H");
        case 0xA5: return simpleInstruction(emulator, "RES 4, L");
        case 0xA6: return simpleInstruction(emulator, "RES 4, (HL)");
        case 0xA7: return simpleInstruction(emulator, "RES 4, A");
        case 0xA8: return simpleInstruction(emulator, "RES 5, B");
        case 0xA9: return simpleInstruction(emulator, "RES 5, C");
        case 0xAA: return simpleInstruction(emulator, "RES 5, D");
        case 0xAB: return simpleInstruction(emulator, "RES 5, E");
        case 0xAC: return simpleInstruction(emulator, "RES 5, H");
        case 0xAD: return simpleInstruction(emulator, "RES 5, L");
        case 0xAE: return simpleInstruction(emulator, "RES 5, (HL)");
        case 0xAF: return simpleInstruction(emulator, "RES 5, A");
        case 0xB0: return simpleInstruction(emulator, "RES 6, B");
        case 0xB1: return simpleInstruction(emulator, "RES 6, C");
        case 0xB2: return simpleInstruction(emulator, "RES 6, D");
        case 0xB3: return simpleInstruction(emulator, "RES 6, E");
        case 0xB4: return simpleInstruction(emulator, "RES 6, H");
        case 0xB5: return simpleInstruction(emulator, "RES 6, L");
        case 0xB6: return simpleInstruction(emulator, "RES 6, (HL)");
        case 0xB7: return simpleInstruction(emulator, "RES 6, A");
        case 0xB8: return simpleInstruction(emulator, "RES 7, B");
        case 0xB9: return simpleInstruction(emulator, "RES 7, C");
        case 0xBA: return simpleInstruction(emulator, "RES 7, D");
        case 0xBB: return simpleInstruction(emulator, "RES 7, E");
        case 0xBC: return simpleInstruction(emulator, "RES 7, H");
        case 0xBD: return simpleInstruction(emulator, "RES 7, L");
        case 0xBE: return simpleInstruction(emulator, "RES 7, (HL)");
        case 0xBF: return simpleInstruction(emulator, "RES 7, A");
        case 0xC0: return simpleInstruction(emulator, "SET 0, B");
        case 0xC1: return simpleInstruction(emulator, "SET 0, C");
        case 0xC2: return simpleInstruction(emulator, "SET 0, D");
        case 0xC3: return simpleInstruction(emulator, "SET 0, E");
        case 0xC4: return simpleInstruction(emulator, "SET 0, H");
        case 0xC5: return simpleInstruction(emulator, "SET 0, L");
        case 0xC6: return simpleInstruction(emulator, "SET 0, (HL)");
        case 0xC7: return simpleInstruction(emulator, "SET 0, A");
        case 0xC8: return simpleInstruction(emulator, "SET 1, B");
        case 0xC9: return simpleInstruction(emulator, "SET 1, C");
        case 0xCA: return simpleInstruction(emulator, "SET 1, D");
        case 0xCB: return simpleInstruction(emulator, "SET 1, E");
        case 0xCC: return simpleInstruction(emulator, "SET 1, H");
        case 0xCD: return simpleInstruction(emulator, "SET 1, L");
        case 0xCE: return simpleInstruction(emulator, "SET 1, (HL)");
        case 0xCF: return simpleInstruction(emulator, "SET 1, A");
        case 0xD0: return simpleInstruction(emulator, "SET 2, B");
        case 0xD1: return simpleInstruction(emulator, "SET 2, C");
        case 0xD2: return simpleInstruction(emulator, "SET 2, D");
        case 0xD3: return simpleInstruction(emulator, "SET 2, E");
        case 0xD4: return simpleInstruction(emulator, "SET 2, H");
        case 0xD5: return simpleInstruction(emulator, "SET 2, L");
        case 0xD6: return simpleInstruction(emulator, "SET 2, (HL)");
        case 0xD7: return simpleInstruction(emulator, "SET 2, A");
        case 0xD8: return simpleInstruction(emulator, "SET 3, B");
        case 0xD9: return simpleInstruction(emulator, "SET 3, C");
        case 0xDA: return simpleInstruction(emulator, "SET 3, D");
        case 0xDB: return simpleInstruction(emulator, "SET 3, E");
        case 0xDC: return simpleInstruction(emulator, "SET 3, H");
        case 0xDD: return simpleInstruction(emulator, "SET 3, L");
        case 0xDE: return simpleInstruction(emulator, "SET 3, (HL)");
        case 0xDF: return simpleInstruction(emulator, "SET 3, A");
        case 0xE0: return simpleInstruction(emulator, "SET 4, B");
        case 0xE1: return simpleInstruction(emulator, "SET 4, C");
        case 0xE2: return simpleInstruction(emulator, "SET 4, D");
        case 0xE3: return simpleInstruction(emulator, "SET 4, E");
        case 0xE4: return simpleInstruction(emulator, "SET 4, H");
        case 0xE5: return simpleInstruction(emulator, "SET 4, L");
        case 0xE6: return simpleInstruction(emulator, "SET 4, (HL)");
        case 0xE7: return simpleInstruction(emulator, "SET 4, A");
        case 0xE8: return simpleInstruction(emulator, "SET 5, B");
        case 0xE9: return simpleInstruction(emulator, "SET 5, C");
        case 0xEA: return simpleInstruction(emulator, "SET 5, D");
        case 0xEB: return simpleInstruction(emulator, "SET 5, E");
        case 0xEC: return simpleInstruction(emulator, "SET 5, H");
        case 0xED: return simpleInstruction(emulator, "SET 5, L");
        case 0xEE: return simpleInstruction(emulator, "SET 5, (HL)");
        case 0xEF: return simpleInstruction(emulator, "SET 5, A");
        case 0xF0: return simpleInstruction(emulator, "SET 6, B");
        case 0xF1: return simpleInstruction(emulator, "SET 6, C");
        case 0xF2: return simpleInstruction(emulator, "SET 6, D");
        case 0xF3: return simpleInstruction(emulator, "SET 6, E");
        case 0xF4: return simpleInstruction(emulator, "SET 6, H");
        case 0xF5: return simpleInstruction(emulator, "SET 6, L");
        case 0xF6: return simpleInstruction(emulator, "SET 6, (HL)");
        case 0xF7: return simpleInstruction(emulator, "SET 6, A");
        case 0xF8: return simpleInstruction(emulator, "SET 7, B");
        case 0xF9: return simpleInstruction(emulator, "SET 7, C");
        case 0xFA: return simpleInstruction(emulator, "SET 7, D");
        case 0xFB: return simpleInstruction(emulator, "SET 7, E");
        case 0xFC: return simpleInstruction(emulator, "SET 7, H");
        case 0xFD: return simpleInstruction(emulator, "SET 7, L");
        case 0xFE: return simpleInstruction(emulator, "SET 7, (HL)");
        case 0xFF: return simpleInstruction(emulator, "SET 7, A");
    }
}

void printInstruction(Emulator* emulator) {
#ifdef DEBUG_PRINT_ADDRESS
    printf("[0x%04x]", emulator->PC);
#endif
#ifdef DEBUG_PRINT_FLAGS
    printFlags(emulator);
#endif
#ifdef DEBUG_PRINT_CYCLES
	/* We print t-cycles */
    printf("[%ld]", emulator->clock * 4);
#endif
#ifdef DEBUG_PRINT_JOYPAD_REG
    printf("[sel:%x|", (emulator->MEM[R_P1_JOYP] >> 4) & 0x3);
    printf("sig:%x]", (emulator->MEM[R_P1_JOYP] & 0b00001111));
#endif
#ifdef DEBUG_PRINT_TIMERS
	printf("[%x|%x|%x|%x]", emulator->MEM[R_DIV], emulator->MEM[R_TIMA], emulator->MEM[R_TMA], emulator->MEM[R_TAC]);
#endif
    printf(" %5s", "");
  
    switch (emulator->cartridge.file[emulator->rPC]) {
        case 0x00: return simpleInstruction(emulator, "NOP");
        case 0x01: return d16(emulator, "LD BC, d16");
        case 0x02: return simpleInstruction(emulator, "LD (BC), A");
        case 0x03: return simpleInstruction(emulator, "INC BC");
        case 0x04: return simpleInstruction(emulator, "INC B");
        case 0x05: return simpleInstruction(emulator, "DEC B");
        case 0x06: return d8(emulator, "LD B, d8");
        case 0x07: return simpleInstruction(emulator, "RLCA");
        case 0x08: return a16(emulator, "LD a16, SP");
        case 0x09: return simpleInstruction(emulator, "ADD HL, BC");
        case 0x0A: return simpleInstruction(emulator, "LD A, (BC)");
        case 0x0B: return simpleInstruction(emulator, "DEC BC");
        case 0x0C: return simpleInstruction(emulator, "INC C");
        case 0x0D: return simpleInstruction(emulator, "DEC C");
        case 0x0E: return d8(emulator, "LD C, d8");
        case 0x0F: return simpleInstruction(emulator, "RRCA");
        case 0x10: return simpleInstruction(emulator, "STOP");
        case 0x11: return d16(emulator, "LD DE, d16");
        case 0x12: return simpleInstruction(emulator, "LD (DE), A");
        case 0x13: return simpleInstruction(emulator, "INC DE");
        case 0x14: return simpleInstruction(emulator, "INC D");
        case 0x15: return simpleInstruction(emulator, "DEC D");
        case 0x16: return d8(emulator, "LD D, d8");
        case 0x17: return simpleInstruction(emulator, "RLA");
        case 0x18: return r8(emulator, "JR r8");
        case 0x19: return simpleInstruction(emulator, "ADD HL, DE");
        case 0x1A: return simpleInstruction(emulator, "LD A, (DE)");
        case 0x1B: return simpleInstruction(emulator, "DEC DE");
        case 0x1C: return simpleInstruction(emulator, "INC E");
        case 0x1D: return simpleInstruction(emulator, "DEC E");
        case 0x1E: return d8(emulator, "LD E, D8");
        case 0x1F: return simpleInstruction(emulator, "RRA");
        case 0x20: return r8(emulator, "JR NZ, r8");
        case 0x21: return d16(emulator, "LD HL, d16");
        case 0x22: return simpleInstruction(emulator, "LD (HL+), A");
        case 0x23: return simpleInstruction(emulator, "INC HL");
        case 0x24: return simpleInstruction(emulator, "INC H");
        case 0x25: return simpleInstruction(emulator, "DEC H");
        case 0x26: return d8(emulator, "LD H, d8");
        case 0x27: return simpleInstruction(emulator, "DAA");
        case 0x28: return r8(emulator, "JR Z, r8");
        case 0x29: return simpleInstruction(emulator, "ADD HL, HL");
        case 0x2A: return simpleInstruction(emulator, "LD A, (HL+)");
        case 0x2B: return simpleInstruction(emulator, "DEC HL");
        case 0x2C: return simpleInstruction(emulator, "INC L");
        case 0x2D: return simpleInstruction(emulator, "DEC L");
        case 0x2E: return d8(emulator, "LD L, d8");
        case 0x2F: return simpleInstruction(emulator, "CPL");
        case 0x30: return r8(emulator, "JR NC, r8");
        case 0x31: return d16(emulator, "LD SP,d16");
        case 0x32: return simpleInstruction(emulator, "LD (HL-), A");
        case 0x33: return simpleInstruction(emulator, "INC SP");
        case 0x34: return simpleInstruction(emulator, "INC (HL)");
        case 0x35: return simpleInstruction(emulator, "DEC (HL)");
        case 0x36: return d8(emulator, "LD (HL), d8");
        case 0x37: return simpleInstruction(emulator, "SCF");
        case 0x38: return r8(emulator, "JR C, r8");
        case 0x39: return simpleInstruction(emulator, "ADD HL, SP");
        case 0x3A: return simpleInstruction(emulator, "LD A, (HL-)");
        case 0x3B: return simpleInstruction(emulator, "DEC SP");
        case 0x3C: return simpleInstruction(emulator, "INC A");
        case 0x3D: return simpleInstruction(emulator, "DEC A");
        case 0x3E: return d8(emulator, "LD A, d8");
        case 0x3F: return simpleInstruction(emulator, "CCF");
        case 0x40: return simpleInstruction(emulator, "LD B, B");
        case 0x41: return simpleInstruction(emulator, "LD B, C");
        case 0x42: return simpleInstruction(emulator, "LD B, D");
        case 0x43: return simpleInstruction(emulator, "LD B, E");
        case 0x44: return simpleInstruction(emulator, "LD B, H");
        case 0x45: return simpleInstruction(emulator, "LD B, L");
        case 0x46: return simpleInstruction(emulator, "LD B, (HL)");
        case 0x47: return simpleInstruction(emulator, "LD B, A");
        case 0x48: return simpleInstruction(emulator, "LD C, B");
        case 0x49: return simpleInstruction(emulator, "LD C, C");
        case 0x4A: return simpleInstruction(emulator, "LD C, D");
        case 0x4B: return simpleInstruction(emulator, "LD C, E");
        case 0x4C: return simpleInstruction(emulator, "LD C, H");
        case 0x4D: return simpleInstruction(emulator, "LD C, L");
        case 0x4E: return simpleInstruction(emulator, "LD C, (HL)");
        case 0x4F: return simpleInstruction(emulator, "LD C, A");
        case 0x50: return simpleInstruction(emulator, "LD D, B");
        case 0x51: return simpleInstruction(emulator, "LD D, C");
        case 0x52: return simpleInstruction(emulator, "LD D, D");
        case 0x53: return simpleInstruction(emulator, "LD D, E");
        case 0x54: return simpleInstruction(emulator, "LD D, H");
        case 0x55: return simpleInstruction(emulator, "LD D, L");
        case 0x56: return simpleInstruction(emulator, "LD D, (HL)");
        case 0x57: return simpleInstruction(emulator, "LD D, A");
        case 0x58: return simpleInstruction(emulator, "LD E, B");
        case 0x59: return simpleInstruction(emulator, "LD E, C");
        case 0x5A: return simpleInstruction(emulator, "LD E, D");
        case 0x5B: return simpleInstruction(emulator, "LD E, E");
        case 0x5C: return simpleInstruction(emulator, "LD E, H");
        case 0x5D: return simpleInstruction(emulator, "LD E, L");
        case 0x5E: return simpleInstruction(emulator, "LD E, (HL)");
        case 0x5F: return simpleInstruction(emulator, "LD E, A");
        case 0x60: return simpleInstruction(emulator, "LD H, B");
        case 0x61: return simpleInstruction(emulator, "LD H, C");
        case 0x62: return simpleInstruction(emulator, "LD H, D");
        case 0x63: return simpleInstruction(emulator, "LD H, E");
        case 0x64: return simpleInstruction(emulator, "LD H, H");
        case 0x65: return simpleInstruction(emulator, "LD H, L");
        case 0x66: return simpleInstruction(emulator, "LD H, (HL)");
        case 0x67: return simpleInstruction(emulator, "LD H, A");
        case 0x68: return simpleInstruction(emulator, "LD L, B");
        case 0x69: return simpleInstruction(emulator, "LD L, C");
        case 0x6A: return simpleInstruction(emulator, "LD L, D");
        case 0x6B: return simpleInstruction(emulator, "LD L, E");
        case 0x6C: return simpleInstruction(emulator, "LD L, H");
        case 0x6D: return simpleInstruction(emulator, "LD L, L");
        case 0x6E: return simpleInstruction(emulator, "LD L, (HL)");
        case 0x6F: return simpleInstruction(emulator, "LD L, A");
        case 0x70: return simpleInstruction(emulator, "LD (HL), B");
        case 0x71: return simpleInstruction(emulator, "LD (HL), C");
        case 0x72: return simpleInstruction(emulator, "LD (HL), D");
        case 0x73: return simpleInstruction(emulator, "LD (HL), E");
        case 0x74: return simpleInstruction(emulator, "LD (HL), H");
        case 0x75: return simpleInstruction(emulator, "LD (HL), L");
        case 0x76: return simpleInstruction(emulator, "HALT");
        case 0x77: return simpleInstruction(emulator, "LD (HL), A");
        case 0x78: return simpleInstruction(emulator, "LD A, B");
        case 0x79: return simpleInstruction(emulator, "LD A, C");
        case 0x7A: return simpleInstruction(emulator, "LD A, D");
        case 0x7B: return simpleInstruction(emulator, "LD A, E");
        case 0x7C: return simpleInstruction(emulator, "LD A, H");
        case 0x7D: return simpleInstruction(emulator, "LD A, L");
        case 0x7E: return simpleInstruction(emulator, "LD A, (HL)");
        case 0x7F: return simpleInstruction(emulator, "LD A, A");
        case 0x80: return simpleInstruction(emulator, "ADD A, B");
        case 0x81: return simpleInstruction(emulator, "ADD A, C");
        case 0x82: return simpleInstruction(emulator, "ADD A, D");
        case 0x83: return simpleInstruction(emulator, "ADD A, E");
        case 0x84: return simpleInstruction(emulator, "ADD A, H");
        case 0x85: return simpleInstruction(emulator, "ADD A, L");
        case 0x86: return simpleInstruction(emulator, "ADD A, (HL)");
        case 0x87: return simpleInstruction(emulator, "ADD A, A");
        case 0x88: return simpleInstruction(emulator, "ADC A, B");
        case 0x89: return simpleInstruction(emulator, "ADC A, C");
        case 0x8A: return simpleInstruction(emulator, "ADC A, D");
        case 0x8B: return simpleInstruction(emulator, "ADC A, E");
        case 0x8C: return simpleInstruction(emulator, "ADC A, H");
        case 0x8D: return simpleInstruction(emulator, "ADC A, L");
        case 0x8E: return simpleInstruction(emulator, "ADC A, (HL)");
        case 0x8F: return simpleInstruction(emulator, "ADC A, A");
        case 0x90: return simpleInstruction(emulator, "SUB B");
        case 0x91: return simpleInstruction(emulator, "SUB C");
        case 0x92: return simpleInstruction(emulator, "SUB D");
        case 0x93: return simpleInstruction(emulator, "SUB E");
        case 0x94: return simpleInstruction(emulator, "SUB H");
        case 0x95: return simpleInstruction(emulator, "SUB L");
        case 0x96: return simpleInstruction(emulator, "SUB (HL)");
        case 0x97: return simpleInstruction(emulator, "SUB A");
        case 0x98: return simpleInstruction(emulator, "SBC A, B");
        case 0x99: return simpleInstruction(emulator, "SBC A, C");
        case 0x9A: return simpleInstruction(emulator, "SBC A, D");
        case 0x9B: return simpleInstruction(emulator, "SBC A, E");
        case 0x9C: return simpleInstruction(emulator, "SBC A, H");
        case 0x9D: return simpleInstruction(emulator, "SBC A, L");
        case 0x9E: return simpleInstruction(emulator, "SBC A, (HL)");
        case 0x9F: return simpleInstruction(emulator, "SBC A, A");
        case 0xA0: return simpleInstruction(emulator, "AND B");
        case 0xA1: return simpleInstruction(emulator, "AND C");
        case 0xA2: return simpleInstruction(emulator, "AND D");
        case 0xA3: return simpleInstruction(emulator, "AND E");
        case 0xA4: return simpleInstruction(emulator, "AND H");
        case 0xA5: return simpleInstruction(emulator, "AND L");
        case 0xA6: return simpleInstruction(emulator, "AND (HL)");
        case 0xA7: return simpleInstruction(emulator, "AND A");
        case 0xA8: return simpleInstruction(emulator, "XOR B");
        case 0xA9: return simpleInstruction(emulator, "XOR C");
        case 0xAA: return simpleInstruction(emulator, "XOR D");
        case 0xAB: return simpleInstruction(emulator, "XOR E");
        case 0xAC: return simpleInstruction(emulator, "XOR H");
        case 0xAD: return simpleInstruction(emulator, "XOR L");
        case 0xAE: return simpleInstruction(emulator, "XOR (HL)");
        case 0xAF: return simpleInstruction(emulator, "XOR A");
        case 0xB0: return simpleInstruction(emulator, "OR B");
        case 0xB1: return simpleInstruction(emulator, "OR C");
        case 0xB2: return simpleInstruction(emulator, "OR D");
        case 0xB3: return simpleInstruction(emulator, "OR E");
        case 0xB4: return simpleInstruction(emulator, "OR H");
        case 0xB5: return simpleInstruction(emulator, "OR L");
        case 0xB6: return simpleInstruction(emulator, "OR (HL)");
        case 0xB7: return simpleInstruction(emulator, "OR A");
        case 0xB8: return simpleInstruction(emulator, "CP B");
        case 0xB9: return simpleInstruction(emulator, "CP C");
        case 0xBA: return simpleInstruction(emulator, "CP D");
        case 0xBB: return simpleInstruction(emulator, "CP E");
        case 0xBC: return simpleInstruction(emulator, "CP H");
        case 0xBD: return simpleInstruction(emulator, "CP L");
        case 0xBE: return simpleInstruction(emulator, "CP (HL)");
        case 0xBF: return simpleInstruction(emulator, "CP A");
        case 0xC0: return simpleInstruction(emulator, "RET NZ");
        case 0xC1: return simpleInstruction(emulator, "POP BC");
        case 0xC2: return a16(emulator, "JP NZ, a16");
        case 0xC3: return a16(emulator, "JP a16");
        case 0xC4: return a16(emulator, "CALL NZ, a16");
        case 0xC5: return simpleInstruction(emulator, "PUSH BC");
        case 0xC6: return d8(emulator, "ADD A, d8");
        case 0xC7: return simpleInstruction(emulator, "RST 0x00");
        case 0xC8: return simpleInstruction(emulator, "RET Z");
        case 0xC9: return simpleInstruction(emulator, "RET");
        case 0xCA: return a16(emulator, "JP Z, a16");
        case 0xCB: return simpleInstruction(emulator, "PREFIX CB");
        case 0xCC: return a16(emulator, "CALL Z, a16");
        case 0xCD: return a16(emulator, "CALL a16");
        case 0xCE: return d8(emulator, "ADC A, d8");
        case 0xCF: return simpleInstruction(emulator, "RST 0x08");
        case 0xD0: return simpleInstruction(emulator, "RET NC");
        case 0xD1: return simpleInstruction(emulator, "POP DE");
        case 0xD2: return a16(emulator, "JP NC, a16");
        case 0xD4: return a16(emulator, "CALL NC, a16");
        case 0xD5: return simpleInstruction(emulator, "PUSH DE");
        case 0xD6: return d8(emulator, "SUB d8");
        case 0xD7: return simpleInstruction(emulator, "RST 0x10");
        case 0xD8: return simpleInstruction(emulator, "REC C");
        case 0xD9: return simpleInstruction(emulator, "RETI");
        case 0xDA: return a16(emulator, "JP C, a16");
        case 0xDC: return a16(emulator, "CALL C, a16");
        case 0xDE: return d8(emulator, "SBC A, d8");
        case 0xDF: return simpleInstruction(emulator, "RST 0x18");
        case 0xE0: return d8(emulator, "LD (0xFF00 + d8), A");
        case 0xE1: return simpleInstruction(emulator, "POP HL");
        case 0xE2: return simpleInstruction(emulator, "LD (0xFF00 + C), A");
        case 0xE5: return simpleInstruction(emulator, "PUSH HL");
        case 0xE6: return d8(emulator, "AND d8");
        case 0xE7: return simpleInstruction(emulator, "RST 0x20");
        case 0xE8: return r8(emulator, "ADD SP, r8");
        case 0xE9: return simpleInstruction(emulator, "JP (HL)");
        case 0xEA: return a16(emulator, "LD (a16), A");
        case 0xEE: return d8(emulator, "XOR d8");
        case 0xEF: return simpleInstruction(emulator, "RST 0x28");
        case 0xF0: return d8(emulator, "LD A, (0xFF00 + d8)");
        case 0xF1: return simpleInstruction(emulator, "POP AF");
        case 0xF2: return simpleInstruction(emulator, "LD A, (0xFF00 + C)");
        case 0xF3: return simpleInstruction(emulator, "DI");
        case 0xF5: return simpleInstruction(emulator, "PUSH AF");
        case 0xF6: return d8(emulator, "OR d8");
        case 0xF7: return simpleInstruction(emulator, "RST 0x30");
        case 0xF8: return r8(emulator, "LD HL, SP + r8");
        case 0xF9: return simpleInstruction(emulator, "LD SP, HL");
        case 0xFA: return a16(emulator, "LD A, (a16)");
        case 0xFB: return simpleInstruction(emulator, "EI");
        case 0xFE: return d8(emulator, "CP d8");
        case 0xFF: return simpleInstruction(emulator, "RST 0x38");
        default: return simpleInstruction(emulator, "????");
    }
}

void printRegisters(Emulator* emulator) {
    printf("Registers: [A %02x|B %02x|C %02x|D %02x|E %02x|H %02x|L %02x|SP %04x]\n\n", 
            emulator->rA, emulator->rB, emulator->rC,
            emulator->rD, emulator->rE, emulator->rH,
            emulator->rL, (uint16_t)(emulator->rSP << 8) + emulator->rSP);
}
