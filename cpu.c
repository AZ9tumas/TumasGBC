
#include "cpu.h"
#include "debug.h"

#define LOAD_16B_RR(e, r1, r2) set_reg16(e, read2Bytes(e), r1, r2)
#define LOAD_8R_16BRR(e, r1, r2, r3) write_to_reg(e, r1, read_address(e, get_reg16(e, r2, r3)))
#define LOAD_16RB_R(e, r1, r2, r3) write_address(e, get_reg16(e, r1, r2), get_reg_byte(e, r3))
#define LOAD_R8_R8(e, r1, r2) write_to_reg(e, r1, get_reg_byte(e, r2))
#define LOAD_8B_R(e, r) write_to_reg(e, r, readByte(e))
// get 16b address and write that with u8 byte from read address
#define LOAD_u8_addr_u16(e, r1, r2) write_address(e, get_reg16(e, r1, r2), readByte(e)) 

#define INC_R1_R2(e, reg1, reg2) set_reg16(e, get_reg16(e, reg1, reg2) + 1, reg1, reg2)
#define DEC_R1_R2(e, reg1, reg2) set_reg16(e, get_reg16(e, reg1, reg2) - 1, reg1, reg2)
#define INC_R(e,r) write_to_reg(e, r, get_reg_byte(e,r) + 1)
#define DEC_R(e,r) write_to_reg(e, r, get_reg_byte(e,r) - 1)

#define SET_FLAG_Z(e, v) set_flag(e, FLAG_Z, v == 0 ? 1 : 0)
#define SET_FLAG_H_ADD(e, v1, v2) set_flag(e, FLAG_H, (((uint32_t)v1 & 0xf) + ((uint32_t)v2 & 0xf) > 0xf) ? 1 : 0)
#define SET_FLAG_H_SUB(e, v1, v2) set_flag(e, FLAG_H, ((v1 & 0xf) - (v2 & 0xf) & 0x10) ? 1 : 0)
#define SET_FLAG_H_ADD16(e, v1, v2) set_flag(e, FLAG_H, (((uint32_t)v1 & 0xfff) + ((uint32_t)v2 & 0xfff) > 0xfff) ? 0 : 1)
#define SET_FLAG_C_ADD16(e, v1, v2) set_flag(e, FLAG_C, ((uint32_t)(v1) + (uint32_t)(v2)) > 0xFFFF ? 1 : 0)
#define SET_FLAG_C_ADD(e, v1, v2) set_flag(e, FLAG_C, ((uint16_t)(v1) + (uint16_t)(v2)) > 0xFF ? 1 : 0)
#define SET_FLAG_C_SUB16(e, v1, v2) set_flag(e, FLAG_C, ((int)(v1) - (int)(v2)) < 0 ? 1 : 0)
#define SET_FLAG_C_SUB(e, v1, v2) set_flag(e, FLAG_C, ((int)(v1) - (int)(v2)) < 0 ? 1 : 0)

#define JUMP(e, r, b) write_to_reg(e, r, get_reg_byte(e, r) + b)
#define JUMP_u16(e, hex) e->rPC=hex
#define JUMP_RR(e,r1,r2) e->rPC=get_reg16(e,r1,r2)

#define POP_RR(e, r1, r2) set_reg16(e, pop16(e), r1, r2)
#define PUSH_RR(e, r1, r2) push16(e, get_reg16(e, r1, r2))
#define RST(e, hex) call(e, hex)


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

    int i = 0;
    for (;;){
        i++;
        printf(" == Dispatch no. %d == \n\n", i);
        dispatch_emulator(emulator);

        //if (i > 15) break;
    }
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

static uint8_t read_address(Emulator* emulator, uint16_t address){
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
    printf("Incrementing register\n");
    uint8_t old_val = get_reg_byte(emulator, reg);
    INC_R(emulator, reg);
    
    SET_FLAG_Z(emulator, reg);
    set_flag(emulator, FLAG_N, 0);
    SET_FLAG_H_ADD(emulator, old_val, 1);

    printf("| old -> 0x%02x | new -> 0x%02x \n\n", old_val, get_reg_byte(emulator, reg));
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

static void ccf(Emulator* emulator){
    set_flag(emulator, FLAG_C, get_flag(emulator, FLAG_C) ^ 1);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 0);
}

static void halt(Emulator* emulator){

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

    set_reg16(emulator, final_val, reg1, reg2);

    set_flag(emulator, FLAG_N, 0);
    SET_FLAG_H_ADD16(emulator, reg_16B_1_2, reg_16B_3_4);
    SET_FLAG_C_ADD16(emulator, reg_16B_1_2, reg_16B_3_4);
}

static void add_R(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2){
    uint8_t byte1 = get_reg_byte(emulator, r1);
    uint8_t byte2 = get_reg_byte(emulator, r2);

    uint8_t final_val = byte1 + byte2;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    SET_FLAG_H_ADD(emulator, byte1, byte2);
    SET_FLAG_C_ADD(emulator, byte1, byte2);
}

static void add_u8(Emulator* emulator, REGISTER_TYPE r1){
    uint8_t byte1 = get_reg_byte(emulator, r1);
    uint8_t byte2 = readByte(emulator);

    uint8_t final_val = byte1 + byte2;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    SET_FLAG_H_ADD(emulator, byte1, byte2);
    SET_FLAG_C_ADD(emulator, byte1, byte2);
}

static void addR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address(emulator, get_reg16(emulator, r2, r3));
    uint8_t final = __r1_val + __r2_3_val;

    write_to_reg(emulator, r1, final);

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 0);
    SET_FLAG_H_ADD(emulator, __r1_val, __r2_3_val);
    SET_FLAG_C_ADD(emulator, __r1_val, __r2_3_val);
}

static void test_adc_hcflags(Emulator* emulator, uint8_t oldval, uint8_t newval, uint8_t final, uint8_t c_flag_val){
    bool half_carry = false;
    bool carry = false;

    /* Half Carry occurs if:
     -> The old val is added to the new val
     -> This result is added to carry flag
    */

    half_carry = ((oldval & 0xF) + (newval & 0xF)) > 0xF || ((final & 0xF) + (c_flag_val & 0xF)) > 0xF ? true : false ;

    /* Carry Flag - If there is an integer overflow for the 8 bit addition:
      -> set the carry flag, calculate overflow separately (like half carry) 
    */

    carry = ((uint16_t)oldval + (uint16_t)newval) > 0xFF || ((uint16_t)final + (uint16_t)c_flag_val) > 0xFF ? true : false ;

    set_flag(emulator, FLAG_H, half_carry);
    set_flag(emulator, FLAG_C, carry);
}

static void test_sbc_hcflags(Emulator* emulator, uint8_t oldval, uint8_t newval, uint8_t final, uint8_t c_flag_val){
    bool half_carry = false;
    bool carry = false;

    uint8_t oldval_low = oldval & 0xF;
    uint8_t final_low = final & 0xF;

    half_carry = (uint8_t)(oldval_low - (newval & 0xF)) > oldval || (uint8_t)(final_low - carry) > final_low ? true : false;
    carry = (uint8_t)((uint16_t)oldval - (uint16_t)newval) > oldval || ((uint8_t)((uint16_t)final - carry) > final) ? true : false;

    set_flag(emulator, FLAG_H, half_carry);
    set_flag(emulator, FLAG_C, carry);
}

static void adc_R(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t r2_val = get_reg_byte(emulator, r2);
    uint8_t flag_c_val = get_flag(emulator, FLAG_C);
    uint8_t final = r1_val + r2_val;
    uint8_t result = final + flag_c_val;

    write_to_reg(emulator, r1, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_N, 0);
    // test adc hc flags ... 
    test_adc_hcflags(emulator, r1_val, r2_val, final, flag_c_val);
}

static void adc_u8(Emulator* emulator, REGISTER_TYPE r1){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t byte = readByte(emulator);
    uint8_t flag_c_val = get_flag(emulator, FLAG_C);
    uint8_t final = r1_val + byte;
    uint8_t result = final + flag_c_val;

    write_to_reg(emulator, r1, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_N, 0);
    // test adc hc flags ... 
    test_adc_hcflags(emulator, r1_val, byte, final, flag_c_val);
}

static void adcR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3) {
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address(emulator, get_reg16(emulator, r2, r3));
    uint8_t flag_c_val = get_flag(emulator, FLAG_C);
    uint8_t final = __r1_val + __r2_3_val;
    uint8_t result = final + flag_c_val;

    write_to_reg(emulator, r1, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_N, 0);
    // test adc hc flags ...
    test_adc_hcflags(emulator, __r1_val, __r2_3_val, final, flag_c_val);
}

static void sub_R(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t r2_val = get_reg_byte(emulator, r2);
    uint8_t final = r1_val - r2_val;

    write_to_reg(emulator, r1, final);

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 1);
    SET_FLAG_H_SUB(emulator, r1_val, r2_val);
    SET_FLAG_C_SUB(emulator, r1_val, r2_val);
}

static void sub_u8(Emulator* emulator, REGISTER_TYPE r1){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t byte = readByte(emulator);
    uint8_t final = r1_val - byte;

    write_to_reg(emulator, r1, final);

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 1);
    SET_FLAG_H_SUB(emulator, r1_val, byte);
    SET_FLAG_C_SUB(emulator, r1_val, byte);
}

static void subR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address(emulator, get_reg16(emulator, r2, r3));
    uint8_t final = __r1_val - __r2_3_val;

    write_to_reg(emulator, r1, final);

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 1);
    SET_FLAG_H_SUB(emulator, __r1_val, __r2_3_val);
    SET_FLAG_C_SUB(emulator, __r1_val, __r2_3_val);
}

static void sbc_R(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t r2_val = get_reg_byte(emulator, r2);
    uint8_t flag_c_val = get_flag(emulator, FLAG_C);
    uint8_t final = r1_val - r2_val;
    uint8_t result = final - flag_c_val;

    write_to_reg(emulator, r1, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_N, 0);
    // test sbc hc flags ... 
    test_adc_hcflags(emulator, r1_val, r2_val, final, flag_c_val);
}

static void sbc_u8(Emulator* emulator, REGISTER_TYPE r1){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t byte = readByte(emulator);
    uint8_t flag_c_val = get_flag(emulator, FLAG_C);
    uint8_t final = r1_val - byte;
    uint8_t result = final - flag_c_val;

    write_to_reg(emulator, r1, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_N, 0);
    // test sbc hc flags ... 
    test_adc_hcflags(emulator, r1_val, byte, final, flag_c_val);
}

static void sbcR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3) {
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address(emulator, get_reg16(emulator, r2, r3));
    uint8_t flag_c_val = get_flag(emulator, FLAG_C);
    uint8_t final = __r1_val - __r2_3_val;
    uint8_t result = final - flag_c_val;

    write_to_reg(emulator, r1, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_N, 1);
    // test sbc hc flags ...
    test_adc_hcflags(emulator, __r1_val, __r2_3_val, final, flag_c_val);
}

static void and_R(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t r2_val = get_reg_byte(emulator, r2);
    uint8_t final_val = r1_val & r2_val;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 1);
    set_flag(emulator, FLAG_C, 0);
}

static void and_u8(Emulator* emulator, REGISTER_TYPE r1){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t byte = readByte(emulator);
    uint8_t final_val = r1_val & byte;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 1);
    set_flag(emulator, FLAG_C, 0);
}

static void andR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address(emulator, get_reg16(emulator, r2, r3));
    uint8_t final = __r1_val & __r2_3_val;

    write_to_reg(emulator, r1, final);

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 1);
    set_flag(emulator, FLAG_C, 0);
}

static void xor_R(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t r2_val = get_reg_byte(emulator, r2);
    uint8_t final_val = r1_val ^ r2_val;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_C, 0);
}

static void xor_u8(Emulator* emulator, REGISTER_TYPE r1){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t byte = readByte(emulator);
    uint8_t final_val = r1_val ^ byte;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_C, 0);
}

static void xorR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address(emulator, get_reg16(emulator, r2, r3));
    uint8_t final = __r1_val ^ __r2_3_val;

    write_to_reg(emulator, r1, final);

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_C, 0);
}

static void or_R(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t r2_val = get_reg_byte(emulator, r2);
    uint8_t final_val = r1_val | r2_val;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_C, 0);
}

static void or_u8(Emulator* emulator, REGISTER_TYPE r1){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t byte = readByte(emulator);
    uint8_t final_val = r1_val | byte;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_C, 0);
}

static void orR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address(emulator, get_reg16(emulator, r2, r3));
    uint8_t final = __r1_val | __r2_3_val;

    write_to_reg(emulator, r1, final);

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_C, 0);
}

static void compare_R(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t r2_val = get_reg_byte(emulator, r2);
    uint8_t final = r1_val - r2_val;

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 1);
    SET_FLAG_H_SUB(emulator, r1_val, r2_val);
    SET_FLAG_C_SUB(emulator, r1_val, r2_val);
}

static void compare_u8(Emulator* emulator, REGISTER_TYPE r1){
    uint8_t r1_val = get_reg_byte(emulator, r1);
    uint8_t byte = readByte(emulator);
    uint8_t final = r1_val - byte;

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 1);
    SET_FLAG_H_SUB(emulator, r1_val, byte);
    SET_FLAG_C_SUB(emulator, r1_val, byte);
}

static void compareR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address(emulator, get_reg16(emulator, r2, r3));
    uint8_t final = __r1_val - __r2_3_val;

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 1);
    SET_FLAG_H_SUB(emulator, __r1_val, __r2_3_val);
    SET_FLAG_C_SUB(emulator, __r1_val, __r2_3_val);
}

static inline uint16_t pop16(Emulator* emulator){
    uint16_t sp_val = emulator->rSP;

    uint8_t before_byte = read_address(emulator, sp_val);
    uint8_t after_byte = read_address(emulator, sp_val + 1);
    
    emulator->rSP = sp_val + 2;

    return (uint16_t)((after_byte << 8) | before_byte);
}

static inline void push16(Emulator* emulator, uint16_t u16byte){
    uint16_t sp = emulator->rSP;

    write_address(emulator, sp - 1, u16byte >> 8);
    write_address(emulator, sp - 2, u16byte & 0xFF);

    emulator->rSP = sp - 2;
}

static void return_condition(Emulator* emulator, bool condition){
    if (condition)emulator->rPC = pop16(emulator);
}

static inline void ret(Emulator* emulator){
    emulator->rPC = pop16(emulator);
}

static void jumpCondition(Emulator* emulator, bool condition){
    uint16_t address = read2Bytes(emulator);

    if (condition){
        emulator->rPC = address;
    }
}

// Call pushes reg PC and sets reg PC to the given address
static void call(Emulator* emulator, uint16_t address){
    push16(emulator, emulator->rPC);
    emulator->rPC = address;
}

static void callCondition(Emulator* emulator, uint16_t address, bool condition){
    if (condition) {
        call(emulator, address);
    }
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
    

    printf("| Instruction Details:");
    printInstruction(emulator);
    printf("\n\n");

    emulator->rPC ++;

    switch (opcode)
    {   
        case 0x00: break;
        case 0x01: LOAD_16B_RR(emulator, REGISTER_B, REGISTER_C); break; // LD BC,u16
        case 0x02: LOAD_16RB_R(emulator, REGISTER_B, REGISTER_C, REGISTER_A); break; // LD (BC),A
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
        case 0x39: {
            // ADD HL SP
            uint16_t hl_byte = get_reg16(emulator, REGISTER_H, REGISTER_L);
            uint16_t sp_byte = emulator->rSP;
            uint16_t final = hl_byte + sp_byte;

            set_reg16(emulator, final, REGISTER_H, REGISTER_L);

            set_flag(emulator, FLAG_N, 0);
            SET_FLAG_H_ADD16(emulator, hl_byte, sp_byte);
            SET_FLAG_C_ADD16(emulator, hl_byte, sp_byte);

            break;
        }
        case 0x3A: DEC_R1_R2(emulator, REGISTER_H, REGISTER_L); LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0x3B: emulator->rSP--;
        case 0x3C: increment_R_8(emulator, REGISTER_A); break;
        case 0x3D: decrement_R_8(emulator, REGISTER_A); break;
        case 0x3E: LOAD_8B_R(emulator, REGISTER_A); break;
        case 0x3F: ccf(emulator); break;
        
        case 0x40: LOAD_R8_R8(emulator, REGISTER_B, REGISTER_B); break;
        case 0x41: LOAD_R8_R8(emulator, REGISTER_B, REGISTER_C); break;
        case 0x42: LOAD_R8_R8(emulator, REGISTER_B, REGISTER_D); break;
        case 0x43: LOAD_R8_R8(emulator, REGISTER_B, REGISTER_E); break;
        case 0x44: LOAD_R8_R8(emulator, REGISTER_B, REGISTER_H); break;
        case 0x45: LOAD_R8_R8(emulator, REGISTER_B, REGISTER_L); break;
        case 0x46: LOAD_8R_16BRR(emulator, REGISTER_B, REGISTER_H, REGISTER_L); break;
        case 0x47: LOAD_R8_R8(emulator, REGISTER_B, REGISTER_A); break;
        case 0x48: LOAD_R8_R8(emulator, REGISTER_C, REGISTER_B); break;
        case 0x49: LOAD_R8_R8(emulator, REGISTER_C, REGISTER_C); break;
        case 0x4A: LOAD_R8_R8(emulator, REGISTER_C, REGISTER_D); break;
        case 0x4B: LOAD_R8_R8(emulator, REGISTER_C, REGISTER_E); break;
        case 0x4C: LOAD_R8_R8(emulator, REGISTER_C, REGISTER_H); break;
        case 0x4D: LOAD_R8_R8(emulator, REGISTER_C, REGISTER_L); break;
        case 0x4E: LOAD_8R_16BRR(emulator, REGISTER_C, REGISTER_H, REGISTER_L); break;
        case 0x4F: LOAD_R8_R8(emulator, REGISTER_C, REGISTER_A); break;
        
        case 0x50: LOAD_R8_R8(emulator, REGISTER_D, REGISTER_B); break;
        case 0x51: LOAD_R8_R8(emulator, REGISTER_D, REGISTER_C); break;
        case 0x52: LOAD_R8_R8(emulator, REGISTER_D, REGISTER_D); break;
        case 0x53: LOAD_R8_R8(emulator, REGISTER_D, REGISTER_E); break;
        case 0x54: LOAD_R8_R8(emulator, REGISTER_D, REGISTER_H); break;
        case 0x55: LOAD_R8_R8(emulator, REGISTER_D, REGISTER_L); break;
        case 0x56: LOAD_8R_16BRR(emulator, REGISTER_D, REGISTER_H, REGISTER_L); break;
        case 0x57: LOAD_R8_R8(emulator, REGISTER_D, REGISTER_A); break;
        case 0x58: LOAD_R8_R8(emulator, REGISTER_E, REGISTER_B); break;
        case 0x59: LOAD_R8_R8(emulator, REGISTER_E, REGISTER_C); break;
        case 0x5A: LOAD_R8_R8(emulator, REGISTER_E, REGISTER_D); break;
        case 0x5B: LOAD_R8_R8(emulator, REGISTER_E, REGISTER_E); break;
        case 0x5C: LOAD_R8_R8(emulator, REGISTER_E, REGISTER_H); break;
        case 0x5D: LOAD_R8_R8(emulator, REGISTER_E, REGISTER_L); break;
        case 0x5E: LOAD_8R_16BRR(emulator, REGISTER_E, REGISTER_H, REGISTER_L); break;
        case 0x5F: LOAD_R8_R8(emulator, REGISTER_E, REGISTER_A); break;
        
        case 0x60: LOAD_R8_R8(emulator, REGISTER_H, REGISTER_B); break;
        case 0x61: LOAD_R8_R8(emulator, REGISTER_H, REGISTER_C); break;
        case 0x62: LOAD_R8_R8(emulator, REGISTER_H, REGISTER_D); break;
        case 0x63: LOAD_R8_R8(emulator, REGISTER_H, REGISTER_E); break;
        case 0x64: LOAD_R8_R8(emulator, REGISTER_H, REGISTER_H); break;
        case 0x65: LOAD_R8_R8(emulator, REGISTER_H, REGISTER_L); break;
        case 0x66: LOAD_8R_16BRR(emulator, REGISTER_H, REGISTER_H, REGISTER_L); break;
        case 0x67: LOAD_R8_R8(emulator, REGISTER_H, REGISTER_A); break;
        case 0x68: LOAD_R8_R8(emulator, REGISTER_L, REGISTER_B); break;
        case 0x69: LOAD_R8_R8(emulator, REGISTER_L, REGISTER_C); break;
        case 0x6A: LOAD_R8_R8(emulator, REGISTER_L, REGISTER_D); break;
        case 0x6B: LOAD_R8_R8(emulator, REGISTER_L, REGISTER_E); break;
        case 0x6C: LOAD_R8_R8(emulator, REGISTER_L, REGISTER_H); break;
        case 0x6D: LOAD_R8_R8(emulator, REGISTER_L, REGISTER_L); break;
        case 0x6E: LOAD_8R_16BRR(emulator, REGISTER_L, REGISTER_H, REGISTER_L); break;
        case 0x6F: LOAD_R8_R8(emulator, REGISTER_L, REGISTER_A); break;
        
        case 0x70: LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_B); break;
        case 0x71: LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_C); break;
        case 0x72: LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_D); break;
        case 0x73: LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_E); break;
        case 0x74: LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_H); break;
        case 0x75: LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_L); break;
        case 0x76: halt(emulator); break;
        case 0x77: LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_A); break;
        case 0x78: LOAD_R8_R8(emulator, REGISTER_A, REGISTER_B); break;
        case 0x79: LOAD_R8_R8(emulator, REGISTER_A, REGISTER_C); break;
        case 0x7A: LOAD_R8_R8(emulator, REGISTER_A, REGISTER_D); break;
        case 0x7B: LOAD_R8_R8(emulator, REGISTER_A, REGISTER_E); break;
        case 0x7C: LOAD_R8_R8(emulator, REGISTER_A, REGISTER_H); break;
        case 0x7D: LOAD_R8_R8(emulator, REGISTER_A, REGISTER_L); break;
        case 0x7E: LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0x7F: LOAD_R8_R8(emulator, REGISTER_A, REGISTER_A); break;
        
        case 0x80: add_R(emulator, REGISTER_A, REGISTER_B); break;
        case 0x81: add_R(emulator, REGISTER_A, REGISTER_C); break;
        case 0x82: add_R(emulator, REGISTER_A, REGISTER_D); break;
        case 0x83: add_R(emulator, REGISTER_A, REGISTER_E); break;
        case 0x84: add_R(emulator, REGISTER_A, REGISTER_H); break;
        case 0x85: add_R(emulator, REGISTER_A, REGISTER_L); break;
        case 0x86: addR_RR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0x87: add_R(emulator, REGISTER_A, REGISTER_A); break;
        case 0x88: adc_R(emulator, REGISTER_A, REGISTER_B); break;
        case 0x89: adc_R(emulator, REGISTER_A, REGISTER_C); break;
        case 0x8A: adc_R(emulator, REGISTER_A, REGISTER_D); break;
        case 0x8B: adc_R(emulator, REGISTER_A, REGISTER_E); break;
        case 0x8C: adc_R(emulator, REGISTER_A, REGISTER_H); break;
        case 0x8D: adc_R(emulator, REGISTER_A, REGISTER_L); break;
        case 0x8E: adcR_RR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0x8F: adc_R(emulator, REGISTER_A, REGISTER_A); break;
        
        case 0x90: sub_R(emulator, REGISTER_A, REGISTER_B); break;
        case 0x91: sub_R(emulator, REGISTER_A, REGISTER_C); break;
        case 0x92: sub_R(emulator, REGISTER_A, REGISTER_D); break;
        case 0x93: sub_R(emulator, REGISTER_A, REGISTER_E); break;
        case 0x94: sub_R(emulator, REGISTER_A, REGISTER_H); break;
        case 0x95: sub_R(emulator, REGISTER_A, REGISTER_L); break;
        case 0x96: subR_RR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0x97: sub_R(emulator, REGISTER_A, REGISTER_A); break;
        case 0x98: sbc_R(emulator, REGISTER_A, REGISTER_B); break;
        case 0x99: sbc_R(emulator, REGISTER_A, REGISTER_C); break;
        case 0x9A: sbc_R(emulator, REGISTER_A, REGISTER_D); break;
        case 0x9B: sbc_R(emulator, REGISTER_A, REGISTER_E); break;
        case 0x9C: sbc_R(emulator, REGISTER_A, REGISTER_H); break;
        case 0x9D: sbc_R(emulator, REGISTER_A, REGISTER_L); break;
        case 0x9E: sbcR_RR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0x9F: sbc_R(emulator, REGISTER_A, REGISTER_A); break;

        case 0xA0: and_R(emulator, REGISTER_A, REGISTER_B); break;
        case 0xA1: and_R(emulator, REGISTER_A, REGISTER_C); break;
        case 0xA2: and_R(emulator, REGISTER_A, REGISTER_D); break;
        case 0xA3: and_R(emulator, REGISTER_A, REGISTER_E); break;
        case 0xA4: and_R(emulator, REGISTER_A, REGISTER_H); break;
        case 0xA5: and_R(emulator, REGISTER_A, REGISTER_L); break;
        case 0xA6: andR_RR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0xA7: and_R(emulator, REGISTER_A, REGISTER_A); break;
        case 0xA8: xor_R(emulator, REGISTER_A, REGISTER_B); break;
        case 0xA9: xor_R(emulator, REGISTER_A, REGISTER_C); break;
        case 0xAA: xor_R(emulator, REGISTER_A, REGISTER_D); break;
        case 0xAB: xor_R(emulator, REGISTER_A, REGISTER_E); break;
        case 0xAC: xor_R(emulator, REGISTER_A, REGISTER_H); break;
        case 0xAD: xor_R(emulator, REGISTER_A, REGISTER_L); break;
        case 0xAE: xorR_RR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0xAF: xor_R(emulator, REGISTER_A, REGISTER_A); break;

        case 0xB0: or_R(emulator, REGISTER_A, REGISTER_B); break;
        case 0xB1: or_R(emulator, REGISTER_A, REGISTER_C); break;
        case 0xB2: or_R(emulator, REGISTER_A, REGISTER_D); break;
        case 0xB3: or_R(emulator, REGISTER_A, REGISTER_E); break;
        case 0xB4: or_R(emulator, REGISTER_A, REGISTER_H); break;
        case 0xB5: or_R(emulator, REGISTER_A, REGISTER_L); break;
        case 0xB6: orR_RR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0xB7: or_R(emulator, REGISTER_A, REGISTER_A); break;
        case 0xB8: compare_R(emulator, REGISTER_A, REGISTER_B); break;
        case 0xB9: compare_R(emulator, REGISTER_A, REGISTER_C); break;
        case 0xBA: compare_R(emulator, REGISTER_A, REGISTER_D); break;
        case 0xBB: compare_R(emulator, REGISTER_A, REGISTER_E); break;
        case 0xBC: compare_R(emulator, REGISTER_A, REGISTER_H); break;
        case 0xBD: compare_R(emulator, REGISTER_A, REGISTER_L); break;
        case 0xBE: compare_R(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0xBF: compare_R(emulator, REGISTER_A, REGISTER_A); break;

        case 0xC0: return_condition(emulator, NZ(emulator)); break;
        case 0xC1: POP_RR(emulator, REGISTER_B, REGISTER_C); break;
        case 0xC2: jumpCondition(emulator, NZ(emulator)); break;
        case 0xC3: JUMP_u16(emulator, read2Bytes(emulator)); break;
        case 0xC4: callCondition(emulator, read2Bytes(emulator), NZ(emulator)); break;
        case 0xC5: PUSH_RR(emulator, REGISTER_B, REGISTER_C); break;
        case 0xC6: add_u8(emulator, REGISTER_A); break;
        case 0xC7: RST(emulator, 0x00); break;
        case 0xC8: return_condition(emulator, Z(emulator)); break;
        case 0xC9: ret(emulator); break;
        case 0xCA: jumpCondition(emulator, Z(emulator)); break;
    //  case 0xCB: prefix(emulator); break;
        case 0xCC: callCondition(emulator, read2Bytes(emulator), Z(emulator)); break;
        case 0xCD: call(emulator, read2Bytes(emulator)); break;
        case 0xCE: adc_u8(emulator, REGISTER_A); break;
        case 0xCF: RST(emulator, 0x08); break;
        
        case 0xD0: return_condition(emulator, NC(emulator)); break;
        case 0xD1: POP_RR(emulator, REGISTER_D, REGISTER_E); break;
        case 0xD2: jumpCondition(emulator, NC(emulator)); break;
        case 0xD4: callCondition(emulator, read2Bytes(emulator), NC(emulator)); break;
        case 0xD5: PUSH_RR(emulator, REGISTER_D, REGISTER_E); break;
        case 0xD6: sub_u8(emulator, REGISTER_A); break;
        case 0xD7: RST(emulator, 0x10); break;
        case 0xD8: return_condition(emulator, C(emulator)); break;
        case 0xD9: ret(emulator); break;
        case 0xDA: jumpCondition(emulator, CONDITION_C(emulator)); break;
        case 0xDC: callCondition(emulator, read2Bytes(emulator), CONDITION_C(emulator)); break;
        case 0xDE: sbc_u8(emulator, REGISTER_A); break;
        case 0xDF: RST(emulator, 0x18); break;
        
        case 0xE0: write_address(emulator, 0xFF00 + readByte(emulator), get_reg_byte(emulator, REGISTER_A)); break;
        case 0xE1: POP_RR(emulator, REGISTER_H, REGISTER_L); break;
        case 0xE2: write_address(emulator, 0xFF00 + get_reg_byte(emulator, REGISTER_C), get_reg_byte(emulator, REGISTER_A)); break;
        case 0xE5: PUSH_RR(emulator, REGISTER_H, REGISTER_L); break;
        case 0xE6: and_u8(emulator, REGISTER_A); break;
        case 0xE7: RST(emulator, 0x20); break;
        case 0xE8: {
            uint16_t old = emulator->rSP;
            int8_t byte = (int8_t)readByte(emulator);
            uint16_t result = old + byte;

            emulator->rSP = result;

            set_flag(emulator, FLAG_Z, 0);
            set_flag(emulator, FLAG_N, 0);

            old &= 0xFF;

            SET_FLAG_H_ADD(emulator, old, (uint8_t)byte);
            SET_FLAG_C_ADD(emulator, old, (uint8_t)byte);

            break;
        }
        case 0xE9: JUMP_RR(emulator, REGISTER_H, REGISTER_L); break;
        case 0xEA: write_address(emulator, read2Bytes(emulator), get_reg_byte(emulator, REGISTER_A)); break;
        case 0xEE: xor_u8(emulator, REGISTER_A); break;
        case 0xEF: RST(emulator, 0x28); break;
        
        case 0xF0: write_to_reg(emulator, REGISTER_A, 0xFF00 + readByte(emulator)); break;
        case 0xF1: POP_RR(emulator, REGISTER_A, REGISTER_F); write_to_reg(emulator, REGISTER_F, get_reg_byte(emulator, REGISTER_F & 0xF0)); break;
        case 0xF2: write_to_reg(emulator, REGISTER_A, 0xFF00 + get_reg_byte(emulator, REGISTER_C)); break;
    //  case 0xF3: INTURREPT_DISABLE(emulator); break;
        case 0xF5: PUSH_RR(emulator, REGISTER_A, REGISTER_F); break;
        case 0xF6: or_u8(emulator, REGISTER_A); break;
        case 0xF7: RST(emulator, 0x30); break;
        case 0xF8: {
            uint16_t old = emulator->rSP;
            int8_t byte = (int8_t) readByte(emulator);
            uint16_t final = set_reg16(emulator, old + byte, REGISTER_H, REGISTER_L);

            set_flag(emulator, FLAG_Z, 0);
            set_flag(emulator, FLAG_N, 0);

            old &= 0xFF;

            SET_FLAG_H_ADD(emulator, old, (uint8_t)byte);
            SET_FLAG_C_ADD(emulator, old, (uint8_t)byte);

            break;
        }
        case 0xF9: emulator->rSP = get_reg16(emulator, REGISTER_H, REGISTER_L); break;
        case 0xFA: write_to_reg(emulator, REGISTER_A, read_address(emulator, read2Bytes(emulator))); break;
    //  case 0xFB: INTURREPT_ENABLE(emulator); break;
        case 0xFE: compare_u8(emulator, REGISTER_A); break;
        case 0xFF: RST(emulator, 0x38); break;

        default:
        printf("ggs\n");
    }
}
