
#include "cpu.h"
#include "debug.h"

#define LOAD_16B_RR(e, r1, r2) set_reg16(e, read2Bytes(e), r1, r2);
#define LOAD_8R_16BRR(e, r1, r2, r3) set_reg16(e, r2, r3, read_address(e, get_reg_byte(e, r1)));
#define LOAD_16RB_R(e, r1, r2, r3) write_to_reg(e, read_address(e, get_reg16(e, r1, r2)), r3);
#define INC_R1_R2(e, reg1, reg2) set_reg16(e, get_reg16(e, reg1, reg2) + 1, reg1, reg2);
#define DEC_R1_R2(e, reg1, reg2) set_reg16(e, get_reg16(e, reg1, reg2) - 1, reg1, reg2);
#define INC_R(e,r) write_to_reg(e, r, get_reg_byte(e,r) + 1);
#define DEC_R(e,r) write_to_reg(e, r, get_reg_byte(e,r) - 1);
#define LOAD_8B_R(e, r) write_to_reg(e, r, readByte(e));
#define SET_FLAG_Z(e, r) set_flag(e, FLAG_Z, r==0?1:0);
#define SET_FLAG_H_ADD(e, v1, v2) set_flag(emulator, FLAG_H, (((uint32_t)v1 & 0xf) + ((uint32_t)v2 & 0xf) > 0xf) ? 1 : 0);
#define SET_FLAG_H_SUB(e, v1, v2) set_flag(emulator, FLAG_H, (((uint32_t)v1 & 0xf) + ((uint32_t)v2 & 0xf) > 0xf) ? 1 : 0);

Emulator* initEmulator(Emulator* emulator){
    emulator->rA = 0x01;
    emulator->rF = 0xF0;
    emulator->rB = 0x00;
    emulator->rC = 0x13;
    emulator->rD = 0x00;
    emulator->rE = 0xd8;
    emulator->rH = 0x01;
    emulator->rL = 0x4d;
    emulator->rPC = 0x0100;
    emulator->rSP = 0xfffe;

    return emulator;
}

void startEmulator(Cartridge* cartridge, Emulator* emulator){
    printf("Dispatching Emulator\n\n");

    for (int i = 0; i < 6; i++){
        printf("Dispatch no. %d, %02x\n\n", i, i);
        dispatch_emulator(emulator);
    }
    printf("Dispatching finished\n\n");
}

static void write_to_reg(Emulator* emulator, REGISTER_TYPE reg, uint8_t byte){
    switch (reg){
        case REGISTER_A: emulator->rA = byte;
        case REGISTER_F: emulator->rF = byte;

        case REGISTER_B: emulator->rB = byte;
        case REGISTER_C: emulator->rC = byte;

        case REGISTER_D: emulator->rD = byte;
        case REGISTER_E: emulator->rE = byte;

        case REGISTER_H: emulator->rH = byte;
        case REGISTER_L: emulator->rL = byte;
    }
}

static uint8_t get_reg_byte(Emulator* emulator, REGISTER_TYPE reg){
    switch (reg){
        case REGISTER_A: return emulator->rA;
        case REGISTER_F: return emulator->rF;

        case REGISTER_B: return emulator->rB;
        case REGISTER_C: return emulator->rC;

        case REGISTER_D: return emulator->rD;
        case REGISTER_E: return emulator->rE;

        case REGISTER_H: return emulator->rH;
        case REGISTER_L: return emulator->rL;
    }
}

static inline void set_flag(Emulator* emulator, FLAG flag, uint8_t bit){
    if (bit) emulator->rF |= 1 << (flag + 4);
    else emulator->rF &= ~(1 << (flag + 4));
}

static inline uint8_t get_flag(Emulator* emulator, FLAG flag){
    return (emulator->rF >> (flag + 4) & 1);
}

static uint16_t read_address(Emulator* emulator, uint16_t address){
    if (address >= 0x0000 && address <= 0x3FFF) {
        return emulator->cartridge.file[address];
    }
}

static void write_address(Emulator* emulator, uint16_t address, uint8_t byte){
    if (address >= 0x0000 && address <= 0x3FFF){
        emulator->cartridge.file = &byte;
    }
}

static uint16_t set_reg16(Emulator* emu, uint16_t byte, REGISTER_TYPE reg1, REGISTER_TYPE reg2) {
    write_to_reg(emu, reg1, byte >> 8);
    write_to_reg(emu, reg2, byte & 0xFF);
    return byte;
}

static uint16_t get_reg16(Emulator* emu, REGISTER_TYPE reg1, REGISTER_TYPE reg2){
    
    return ((get_reg_byte(emu, reg1) << 8) | get_reg_byte(emu, reg2));
}

static inline uint8_t readByte(Emulator* emu){
    return (uint8_t)(read_address(emu, emu->rPC++));
}

static inline uint16_t read2Bytes(Emulator* emu) {
    return (uint16_t)(read_address(emu, emu->rPC++) | (read_address(emu, emu->rPC++) << 8));
}

static void increment_R_8(Emulator* emulator, REGISTER_TYPE reg){
    uint8_t old_val = get_reg_byte(emulator, reg);
    INC_R(emulator, reg);
    
    SET_FLAG_Z(emulator, reg);
    set_flag(emulator, FLAG_N, 0);
    SET_FLAG_H_ADD(emulator, old_val, 1);
}

static void decrement_R_8(Emulator* emulator, REGISTER_TYPE reg){
    uint8_t old_val = get_reg_byte(emulator, reg);
    uint8_t new = --old_val;
    DEC_R(emulator, reg);

    set_flag(emulator, FLAG_Z, reg == 0 ? 1 : 0);
    set_flag(emulator, FLAG_N, 1);
    SET_FLAG_H_SUB(emulator, old_val, 1);
}

static void rotateLeftR8(Emulator* emulator, REGISTER_TYPE reg){
    
    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, get_reg_byte(emulator, reg) >> 7);
}

static void addRR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2, REGISTER_TYPE reg3, REGISTER_TYPE reg4){
    // ADD HL,BC - 0x09
    uint16_t reg_16B_1_2 = get_reg16(emulator, reg1, reg2);
    uint16_t reg_16B_3_4 = get_reg16(emulator, reg3, reg4);

    uint16_t final_val = reg_16B_1_2 + reg_16B_3_4;

    set_reg16(emulator, reg1, reg2, final_val);

}

static void addR8(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2){
    uint8_t byte1 = get_reg_byte(emulator, reg1);
    uint8_t byte2 = get_reg_byte(emulator, reg2);

    uint8_t final_val = byte1 + byte2;

    write_to_reg(emulator, reg1, final_val);
}

void dispatch_emulator(Emulator* emulator){
    uint8_t opcode = read_address(emulator, emulator->rPC);
    emulator->rPC ++;

    printf("OpCode: %d, %02x\n Instruction: ", opcode, opcode);
    printInstruction(emulator);
    printf("\n\n");

    printRegisters(emulator);

    switch (opcode)
    {   
        case 0x00: break;
        case 0x01: LOAD_16B_RR(emulator, REGISTER_B, REGISTER_C) break; // LD BC,u16
        case 0x02: LOAD_16RB_R(emulator, REGISTER_B, REGISTER_C, REGISTER_A) break; // LD (BC),A
        case 0x03: INC_R1_R2(emulator, REGISTER_B, REGISTER_C); break; // INC BC
        case 0x04: increment_R_8(emulator, REGISTER_B); break; // INC B
        case 0x05: decrement_R_8(emulator, REGISTER_B); break; // DEC B
        case 0x06: LOAD_8B_R(emulator, REGISTER_B); break; // LD B,u8
        case 0x07: rotateLeftR8(emulator, REGISTER_A); break; // RLC_ REG A
        case 0x08: {
            uint16_t two_bytes = read2Bytes(emulator);
            uint16_t SP_VAL = emulator->rSP;

            write_address(emulator, two_bytes + 1, SP_VAL >> 8);
            write_address(emulator, two_bytes, SP_VAL & 0xFF);
            break;
        }
        case 0x09: addRR16(emulator, REGISTER_H, REGISTER_L, REGISTER_B, REGISTER_C);break;
        case 0x0A: LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_B, REGISTER_C);break;
        case 0x0B: DEC_R1_R2(emulator, REGISTER_B, REGISTER_C);break;
        case 0x0C: increment_R_8(emulator, REGISTER_C);break;
        case 0x0D: decrement_R_8(emulator, REGISTER_C); break;
        case 0x0E: LOAD_8B_R(emulator, REGISTER_C); break;
        // ignoring case 0x0F: RRCA
        // ignoring case 0x10: STOP
        case 0x11: LOAD_16B_RR(emulator, REGISTER_D, REGISTER_E);break;
        case 0x12: LOAD_16RB_R(emulator, REGISTER_D, REGISTER_E, REGISTER_A); break;
        case 0x13: INC_R1_R2(emulator, REGISTER_B, REGISTER_C); break;
        case 0x14: increment_R_8(emulator, REGISTER_D); break;
        case 0x15: decrement_R_8(emulator, REGISTER_D); break;
        case 0x16: LOAD_8B_R(emulator, REGISTER_D); break;
        // ignoring case 0x17: RLA
        // ignoring case 0x18: JR i8 
        case 0x19: addRR16(emulator, REGISTER_H, REGISTER_L, REGISTER_D, REGISTER_E); break;
        case 0x1A: LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_D, REGISTER_E); break;
        case 0x1B: DEC_R1_R2(emulator, REGISTER_D, REGISTER_E); break;
        case 0x1C: increment_R_8(emulator, REGISTER_E); break;
        case 0x1D: decrement_R_8(emulator, REGISTER_E); break;
        case 0x1E: LOAD_8B_R(emulator, REGISTER_E); break;
        // ignoring case 0x1F: RRA
        // ignoring case 0x20: JR NZ i8
        case 0x21: LOAD_16B_RR(emulator, REGISTER_H, REGISTER_L); break;
        case 0x22: INC_R1_R2(emulator, REGISTER_H, REGISTER_L); LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_A); break;
        case 0x23: INC_R1_R2(emulator, REGISTER_H, REGISTER_L); break;
        case 0x24: increment_R_8(emulator, REGISTER_H); break;
        case 0x25: decrement_R_8(emulator, REGISTER_H); break;
        case 0x26: LOAD_8B_R(emulator, REGISTER_H); break;
        // ignoring case 0x27: DAA
        // ignoring case 0x28: JR Z i8
        case 0x29: addRR16(emulator, REGISTER_H, REGISTER_L, REGISTER_H, REGISTER_L); break;
        case 0x2A: INC_R1_R2(emulator, REGISTER_H, REGISTER_H); LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0x2B: DEC_R1_R2(emulator, REGISTER_H, REGISTER_L); break;
        case 0x2C: increment_R_8(emulator, REGISTER_L); break;
        case 0x2D: decrement_R_8(emulator, REGISTER_L); break;
        case 0x2E: LOAD_8B_R(emulator,  REGISTER_L); break;
        // ignoring case 0x2F: CPL
        // ignoring case 0x30: JR NC, i8
        case 0x31: emulator->rSP = read2Bytes(emulator); break;
    }

    printRegisters(emulator);
}
