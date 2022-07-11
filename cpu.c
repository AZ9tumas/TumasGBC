
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
// get 16b address and write that with u8 byte from read address
#define LOAD_u8_addr_u16(e, r1, r2) write_address(e, get_reg16(e, r1, r2), readByte(e)); 
#define SET_FLAG_Z(e, v) set_flag(e, FLAG_Z, v == 0 ? 1 : 0);
#define SET_FLAG_H_ADD(e, v1, v2) set_flag(e, FLAG_H, (((uint32_t)v1 & 0xf) + ((uint32_t)v2 & 0xf) > 0xf) ? 1 : 0);
#define SET_FLAG_H_SUB(e, v1, v2) set_flag(e, FLAG_H, ((v1 & 0xf) - (v2 & 0xf) & 0x10) ? 1 : 0)
#define SET_FLAG_H_ADD16(e, v1, v2) set_flag(e, FLAG_H, (((uint32_t)v1 & 0xfff) + ((uint32_t)v2 & 0xfff) > 0xfff) ? 0 : 1)
#define SET_FLAG_C_ADD16(e, v1, v2) set_flag(e, FLAG_C, ((uint32_t)(v1) + (uint32_t)(v2)) > 0xFFFF ? 1 : 0)
#define JUMP(e, r, b) write_to_reg(e, r, get_reg_byte(e, r) + b);

// Jump Condition Check ...

#define NZ(e) (get_flag(e, FLAG_Z) != 1)
#define NC(e) (get_flag(e, FLAG_C) != 1)

#define Z(e)  (get_flag(e, FLAG_Z) == 1)
#define C(e)  (get_flag(e, FLAG_C) == 1)

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

static void decimal_adjust(Emulator* emulator){
    uint8_t value = get_reg_byte(emulator, REGISTER_A);

    if (get_flag(emulator, FLAG_N)) {
        
        // Previous operation was sub
        if (get_flag(emulator, FLAG_H)) value -= 0x06;
        if (get_flag(emulator, FLAG_C)) value -= 0x60;
    } else {
        // Previous operation was addition
        if (get_flag(emulator, FLAG_H) || (value & 0xF) > 0x9) {
            uint8_t original = value;
            value += 0x6;

            if (original > value) set_flag(emulator, FLAG_C, 1);
        }
        
        if (get_flag(emulator, FLAG_C) || value > 0x9F) value += 0x60; set_flag(emulator, FLAG_C, 1);
    }

    SET_FLAG_Z(emulator, value);
    set_flag(emulator, FLAG_H, 0);

    write_to_reg(emulator, REGISTER_A, value);
}

static void cpl(Emulator* emulator){
    write_to_reg(emulator, REGISTER_A, ~get_reg_byte(emulator, REGISTER_A));

    set_flag(emulator, FLAG_N, 1);
    set_flag(emulator, FLAG_H, 1);
}

static void JumpConditionRelative(Emulator* emulator, bool condition){
    if (condition){
        emulator->rPC += (int8_t)readByte(emulator);
    }
}

// Rotates value to left, moves bit 7 to C flag and C flag's original value to bit 0
static void RotateLeftCarryR8(Emulator* emulator, REGISTER_TYPE reg) {
    uint8_t val = get_reg_byte(emulator, reg);
    bool carry_flag = get_flag(emulator, FLAG_C);
    uint8_t bit7 = val >> 7;

    val <<= 1;
    val |= carry_flag;

    write_to_reg(emulator, reg, val);

    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit7);
}

// Rotates value to right, moves bit 0 to C flag and C flag's original value to bit 7 
static void RotateRightCarryR8(Emulator* emulator, REGISTER_TYPE reg){
    uint8_t val = get_reg_byte(emulator, reg);
    bool carry_flag = get_flag(emulator, FLAG_C);
    uint8_t bit0 = val & 1;

    val >>= 1;
    val |= carry_flag << 7;

    write_to_reg(emulator, reg, val);

    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit0);
}


static void rotateRightR8(Emulator* emulator, REGISTER_TYPE reg) {
    uint8_t val = get_reg_byte(emulator, reg);
    uint8_t bitno1 = val & 1;

    val >>= 1;
    val |= bitno1 << 7;

    write_to_reg(emulator, reg, val);

    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bitno1);
}

static void rotateLeftR8(Emulator* emulator, REGISTER_TYPE reg){
    
    uint8_t val = get_reg_byte(emulator, reg);
    uint8_t bitno7 = val >> 7;

    val <<= 1;
    val |= bitno7;

    write_to_reg(emulator, reg, val);

    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bitno7);
}

static void addRR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2, REGISTER_TYPE reg3, REGISTER_TYPE reg4){
    // ADD HL,BC - 0x09
    uint16_t reg_16B_1_2 = get_reg16(emulator, reg1, reg2);
    uint16_t reg_16B_3_4 = get_reg16(emulator, reg3, reg4);

    uint16_t final_val = reg_16B_1_2 + reg_16B_3_4;

    set_reg16(emulator, reg1, reg2, final_val);

    set_flag(emulator, FLAG_N, 0);
    SET_FLAG_H_ADD16(emulator, reg_16B_1_2, reg_16B_3_4);
    SET_FLAG_C_ADD16(emulator, reg_16B_1_2, reg_16B_3_4);
}

static void addR8(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2){
    uint8_t byte1 = get_reg_byte(emulator, reg1);
    uint8_t byte2 = get_reg_byte(emulator, reg2);

    uint8_t final_val = byte1 + byte2;

    write_to_reg(emulator, reg1, final_val);
}

/*Rotate is a binary operation
* The bytes can be represented in binary so 234 in binary is 11101010
* If u do a rotate operation on 234, it's gonna rotate or move the bits in a particular direction
* For example if u rotate 234 towards the right, it's gonna move the bit that's first to the second place, second to third and so on. It will move the eighth bit back to the first place 
* That's why it's called rotate because it treats it like a circle and wraps it around
* So rotating 234 to the right will turn 11101010 to 01110101
* The bits moved to the right by 1 place and the last one moved to the place of the first. 
* 01110101 is 117 in decimal
* So if u have 234 in A, u execute RR A or rotate right A, it will store 117 in A
* RRCA is just the normal rotate instruction but it only operates on the A register, and it stores the last bit to the carry flag. So if the last bit was 1 which rotated to the first place, it's gonna do the rotation but also store the bit in the C flag
* Similarly you can rotate it to the left too
* So rotating 234 to left will do this 11101010 -> 11010101
* The bits moved to the left by one place 
* And the first bit went to the last position
* So that will give u 213 in decimal
* RLCA does the same additional stuff as RRCA
* Store the bit that wrapped around in the C flag
* Now u don't have to sit and convert this to binary and then back to decimal when implementing this
* Binary operators already exist that let u operate on individual bits
* C has a 'shift' operator which shifts the bits to right or left
* But that doesn't do the complete work as it still doesn't wrap around the bits
* It fills the new space with a 0 instead
So for example 234 << 1 will do 11101010 -> 11010100
* It shifted them to the left by 1 place but just filled the new bit at the end with a 0 
* Ull have to read the first bit manually before shifting, in that case it's 1. Then u use the shift operator. Then u insert that bit u read in the last place to complete the rotate operation.
* Additionally u would also store the bit u read in the C flag
* If it's RLCA
*/

void dispatch_emulator(Emulator* emulator){
    uint8_t opcode = read_address(emulator, emulator->rPC);
    emulator->rPC ++;

    printf("OpCode: %d, 0x%02x\n Instruction: ", opcode, opcode);
    printInstruction(emulator);
    printf("\n\n");

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
        case 0x0F: rotateRightR8(emulator, REGISTER_A); break;
        case 0x10: break; // STOP (stops cpu)
        case 0x11: LOAD_16B_RR(emulator, REGISTER_D, REGISTER_E);break;
        case 0x12: LOAD_16RB_R(emulator, REGISTER_D, REGISTER_E, REGISTER_A); break;
        case 0x13: INC_R1_R2(emulator, REGISTER_B, REGISTER_C); break;
        case 0x14: increment_R_8(emulator, REGISTER_D); break;
        case 0x15: decrement_R_8(emulator, REGISTER_D); break;
        case 0x16: LOAD_8B_R(emulator, REGISTER_D); break;
        case 0x17: RotateLeftCarryR8(emulator, REGISTER_A); break;
        case 0x18: emulator->rPC += (uint8_t)readByte(emulator); break; // JR i8
        case 0x19: addRR16(emulator, REGISTER_H, REGISTER_L, REGISTER_D, REGISTER_E); break;
        case 0x1A: LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_D, REGISTER_E); break;
        case 0x1B: DEC_R1_R2(emulator, REGISTER_D, REGISTER_E); break;
        case 0x1C: increment_R_8(emulator, REGISTER_E); break;
        case 0x1D: decrement_R_8(emulator, REGISTER_E); break;
        case 0x1E: LOAD_8B_R(emulator, REGISTER_E); break;
        case 0x1F: RotateRightCarryR8(emulator, REGISTER_A); break;
        case 0x20: JumpConditionRelative(emulator, NZ(emulator)); break; // JR NZ i8
        case 0x21: LOAD_16B_RR(emulator, REGISTER_H, REGISTER_L); break;
        case 0x22: LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_A); INC_R1_R2(emulator, REGISTER_H, REGISTER_L); break;
        case 0x23: INC_R1_R2(emulator, REGISTER_H, REGISTER_L); break;
        case 0x24: increment_R_8(emulator, REGISTER_H); break;
        case 0x25: decrement_R_8(emulator, REGISTER_H); break;
        case 0x26: LOAD_8B_R(emulator, REGISTER_H); break;
        case 0x27: decimal_adjust(emulator); break;
        case 0x28: JumpConditionRelative(emulator, Z(emulator)); break; // JR Z i8
        case 0x29: addRR16(emulator, REGISTER_H, REGISTER_L, REGISTER_H, REGISTER_L); break;
        case 0x2A: INC_R1_R2(emulator, REGISTER_H, REGISTER_H); LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0x2B: DEC_R1_R2(emulator, REGISTER_H, REGISTER_L); break;
        case 0x2C: increment_R_8(emulator, REGISTER_L); break;
        case 0x2D: decrement_R_8(emulator, REGISTER_L); break;
        case 0x2E: LOAD_8B_R(emulator,  REGISTER_L); break;
        case 0x2F: cpl(emulator); break; // CPL
        case 0x30: JumpConditionRelative(emulator, NC(emulator)); // JR NC i8
        case 0x31: emulator->rSP = read2Bytes(emulator); break;
        case 0x32: DEC_R1_R2(emulator, REGISTER_H, REGISTER_L); LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0x33: emulator->rSP++; break;
        
        case 0x34: {
            // increment address in HL
            uint16_t address = get_reg16(emulator, REGISTER_H, REGISTER_L);
            uint8_t old = read_address(emulator, address);
            uint8_t new = old + 1;

            SET_FLAG_Z(emulator, new);
            SET_FLAG_H_ADD16(emulator, old, 1);
            set_flag(emulator, FLAG_N, 0);
            write_address(emulator, address, new);

            break;
        }
        
        case 0x35: {
            // opposite of 0x34 (decrement)
            uint16_t address = get_reg16(emulator, REGISTER_H, REGISTER_L);
            uint8_t old = read_address(emulator, address);
            uint8_t new = old - 1;

            SET_FLAG_Z(emulator, new);
            SET_FLAG_H_ADD16(emulator, old, 1);
            set_flag(emulator, FLAG_N, 0);
            write_address(emulator, address, new);

            break;
        }
        
        case 0x36: LOAD_u8_addr_u16(emulator, REGISTER_H, REGISTER_L); break;
        case 0x37: {
            set_flag(emulator, FLAG_C, 1);
            set_flag(emulator, FLAG_N, 0);
            set_flag(emulator, FLAG_H, 0);
            break;
        }
        case 0x38: JumpConditionRelative(emulator, C(emulator)); // JR C i8
        
    }
}
