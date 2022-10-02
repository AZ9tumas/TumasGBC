
#include "cpu.h"
#include "debug.h"

#include <time.h>
#include <sys/time.h>

#define LOAD_16B_RR(e, r1, r2) set_reg16_8C(e, read2Bytes(e), r1, r2)
#define LOAD_8R_16BRR(e, r1, r2, r3) write_to_reg(e, r1, read_address(e, get_reg16(e, r2, r3))); cyclesSync(e)
#define LOAD_16RB_R(e, r1, r2, r3) write_address_4C(e, get_reg16(e, r1, r2), get_reg_byte(e, r3))
#define LOAD_R8_R8(e, r1, r2) write_to_reg(e, r1, get_reg_byte(e, r2))
#define LOAD_8B_R(e, r) write_to_reg(e, r, readByte(e)); cyclesSync(e)
// get 16b address and write that with u8 byte from read address
#define LOAD_u8_addr_u16(e, r1, r2) write_address_4C(e, get_reg16(e, r1, r2), readByte(e)) 

#define INC_R1_R2(e, reg1, reg2) set_reg16(e, get_reg16(e, reg1, reg2) + 1, reg1, reg2)
#define DEC_R1_R2(e, reg1, reg2) set_reg16(e, get_reg16(e, reg1, reg2) - 1, reg1, reg2)
#define INC_R(e,r) write_to_reg(e, r, get_reg_byte(e,r) + 1);
#define DEC_R(e,r) write_to_reg(e, r, get_reg_byte(e,r) - 1);

#define SET_FLAG_Z(e, v) set_flag(e, FLAG_Z, v == 0 ? 1 : 0)
#define SET_FLAG_H_ADD(e, v1, v2) set_flag(e, FLAG_H, (((uint32_t)v1 & 0xf) + ((uint32_t)v2 & 0xf) > 0xf) ? 1 : 0)
#define SET_FLAG_H_SUB(e, v1, v2) set_flag(e, FLAG_H, ((v1 & 0xf) - (v2 & 0xf) & 0x10) ? 1 : 0)
#define SET_FLAG_H_ADD16(e, v1, v2) set_flag(e, FLAG_H, (((uint32_t)v1 & 0xfff) + ((uint32_t)v2 & 0xfff) > 0xfff) ? 0 : 1)
#define SET_FLAG_C_ADD16(e, v1, v2) set_flag(e, FLAG_C, ((uint32_t)(v1) + (uint32_t)(v2)) > 0xFFFF ? 1 : 0)
#define SET_FLAG_C_ADD(e, v1, v2) set_flag(e, FLAG_C, ((uint16_t)(v1) + (uint16_t)(v2)) > 0xFF ? 1 : 0)
#define SET_FLAG_C_SUB16(e, v1, v2) set_flag(e, FLAG_C, ((int)(v1) - (int)(v2)) < 0 ? 1 : 0)
#define SET_FLAG_C_SUB(e, v1, v2) set_flag(e, FLAG_C, ((int)(v1) - (int)(v2)) < 0 ? 1 : 0)

#define JUMP(e, r, b) write_to_reg(e, r, get_reg_byte(e, r) + b)
#define JUMP_u16(e, hex) e->rPC=hex; cyclesSync(e)
#define JUMP_RR(e,r1,r2) e->rPC=get_reg16(e,r1,r2)

#define POP_RR(e, r1, r2) set_reg16(e, pop16(e), r1, r2)
#define PUSH_RR(e, r1, r2) cyclesSync(e); push16(e, get_reg16(e, r1, r2))
#define RST(e, hex) call(e, hex)
#define IME(e) e->schedule_interrupt_enable = true
#define IMD(e) e->IME = false

// Jump Condition Check ...

#define NZ(e) (get_flag(e, FLAG_Z) != 1)
#define NC(e) (get_flag(e, FLAG_C) != 1)

#define Z(e)  (get_flag(e, FLAG_Z) == 1)
#define C(e)  (get_flag(e, FLAG_C) == 1)

#define INTERRUPT_REQ(IE, IF) ((IE & IF & 0x1F) != 0)

static void stop_halt(Emulator* emualtor);

bool wrong = false;

static void write_address(Emulator* emulator, uint16_t address, uint8_t byte);
uint8_t read_address(Emulator* emulator, uint16_t address);

#define NO_INF_RUN

static void run(Emulator* emulator){
    emulator->start_time_ticks = getTime();

    int dispatch_count = 0;
    while (emulator->run == true && wrong == false){
        
        #ifndef USE_SDL2
         SDL_events(emulator);
        #endif

        for (int i = 0; i < 2000; i ++){
            dispatch_emulator(emulator);
            dispatch_count ++;

            #ifdef NO_INF_RUN
                if (dispatch_count >= 300000) {
                    emulator->run = false;
                    return;
                }
            #endif
        }
    }
}

void startEmulator(Cartridge* cartridge, Emulator* emulator){
    initCartridgeEmulator(emulator, *cartridge);
    int success_result = 0; //initSDL(emulator);

    if (success_result != 0) {
        printf("Unable to initialise SDL 2.0\n");
        return;
    }

    write_address(emulator, 0xFF44, 0x00);

    emulator->run = true;
    run(emulator);
    endRun(emulator);
}

void cyclesSync(Emulator* emulator){
    emulator->clock += 4;

    /* if clock % 456 == 0, and then u increment LY by 1
    * LY goes till 153
    * then it resets to 0
    * also on LY = 153
    it resets to 0 early
    6 cycles early
    

    if (read_address(emulator, 0xFF44) > 153){
        write_address(emulator, 0xFF44, 0x00);
    }

    if ((emulator->clock + 4) % 456 == 0) {
        write_address(emulator, 0xFF44, read_address(emulator, 0xFF44) + 1);
    }*/

    #ifndef USE_SDL2
        Sync_Display(emulator, 4);
    #endif
}

static uint8_t read_address_4C(Emulator* emulator, uint16_t address){
    uint8_t g = read_address(emulator, address);
    cyclesSync(emulator);
    return g;
}

static void write_address_4C(Emulator* emulator, uint16_t address, uint8_t byte){
    write_address(emulator, address, byte);
    cyclesSync(emulator);
}

static void write_to_reg(Emulator* emulator, REGISTER_TYPE reg, uint8_t byte){
    switch (reg){
        case REGISTER_A: emulator->rA = byte; break;
        case REGISTER_F: emulator->rF = byte; break;

        case REGISTER_B: emulator->rB = byte; break;
        case REGISTER_C: emulator->rC = byte; break;

        case REGISTER_D: emulator->rD = byte; break;
        case REGISTER_E: emulator->rE = byte; break;

        case REGISTER_H: emulator->rH = byte; break;
        case REGISTER_L: emulator->rL = byte; break;
    }
}

static uint8_t get_reg_byte(Emulator* emulator, REGISTER_TYPE reg){
    switch (reg){
        case REGISTER_A: return emulator->rA; break;
        case REGISTER_F: return emulator->rF; break;

        case REGISTER_B: return emulator->rB; break;
        case REGISTER_C: return emulator->rC; break;

        case REGISTER_D: return emulator->rD; break;
        case REGISTER_E: return emulator->rE; break;

        case REGISTER_H: return emulator->rH; break;
        case REGISTER_L: return emulator->rL; break;
    }
}

static inline void set_flag(Emulator* emulator, FLAG flag, uint8_t bit){
    if (bit) emulator->rF |= 1 << (flag + 4);
    else emulator->rF &= ~(1 << (flag + 4));
}

static inline uint8_t get_flag(Emulator* emulator, FLAG flag){
    return (emulator->rF >> (flag + 4) & 1);
}

uint8_t read_address(Emulator* emulator, uint16_t address){
    /* Debugging stuff */
    //if (address >= VRAM_N0_8KB && address <= VRAM_N0_8KB_END)   printf ("VRAM -> 0x%02x\n", emulator->VRAM[address - VRAM_N0_8KB]);

    /* Main Stuff */
    if (address >= 0x0000 && address <= 0x3FFF)                 return emulator->cartridge.file[address];
    if (address >= 0x4000 && address <= 0x7FFF)                 return emulator->cartridge.file[address];
    if (address >= VRAM_N0_8KB && address <= VRAM_N0_8KB_END)   return emulator->VRAM   [address - VRAM_N0_8KB];
    if (address >= WRAM_N0_4KB && address <= WRAM_N0_4KB_END)   return emulator->WRAM1  [address - WRAM_N0_4KB];
    if (address >= WRAM_NN_4KB && address <= WRAM_NN_4KB_END)   return emulator->WRAM2  [address - WRAM_NN_4KB];
    if (address >= HRAM_N0 && address <= HRAM_N0_END)           return emulator->HRAM   [address - HRAM_N0];
    if (address >= IO_REG && address <= IO_REG_END)             return emulator->IO     [address - IO_REG];
    
    return 0xFF;
}

static void write_address(Emulator* emulator, uint16_t address, uint8_t byte){
    if ((address >= ECHO_N0_8KB && address <= ECHO_N0_8KB_END) || (address >= UNUSABLE_N0 && address <= UNUSABLE_N0_END)) {
        //printf("\n\t<== WARNING ==> ATTEMPT TO WRITE TO READ ONLY AREA\n");
        //emulator->run = false;
        return;
    }

    if (address >= VRAM_N0_8KB && address <= VRAM_N0_8KB_END) {
        emulator->VRAM[address - VRAM_N0_8KB] = byte;

    } else if (address >= WRAM_N0_4KB && address <= WRAM_N0_4KB_END){
        emulator->WRAM1[address - WRAM_N0_4KB] = byte;

    } else if (address >= WRAM_NN_4KB && address <= WRAM_NN_4KB_END){
        emulator->WRAM2[address - WRAM_NN_4KB] = byte;

    } else if (address >= HRAM_N0 && address <= HRAM_N0_END){
        emulator->HRAM[address - HRAM_N0] = byte;

    } else if (address >= IO_REG && address <= IO_REG_END){ // 0xff44 issue: 0xff44 is 65348
        //printf("\tIO WRITE REGISTERS\n");
        //printf("[%c]", read_address(emulator, 0xFF01));
        switch (address) {
            /*case 0xFF05: break; // R_TIMA
            case 0xFF06: break; // R_TMA
            case 0xFF07: return; // R_TAC
            case 0xFF04: return; // R_DIV*/
            case 0xFF02: { // R_SC
                if (byte == 0x81){
                    printf("%c", read_address(emulator, 0xFF01));
                    write_address(emulator, address, 0x00);
                }
                break;
            }
        }

        emulator->IO[address - IO_REG] = byte;
    }
    return;
}

static uint16_t set_reg16(Emulator* emu, uint16_t byte, REGISTER_TYPE reg1, REGISTER_TYPE reg2) {
    write_to_reg(emu, reg1, byte >> 8);
    write_to_reg(emu, reg2, byte & 0xFF);
    return byte;
}

static uint16_t set_reg16_8C(Emulator* emu, uint16_t byte, REGISTER_TYPE reg1, REGISTER_TYPE reg2){
    uint16_t val = set_reg16(emu, byte, reg1, reg2);
    cyclesSync(emu);
    cyclesSync(emu);
    return val;
}

static uint16_t get_reg16(Emulator* emu, REGISTER_TYPE reg1, REGISTER_TYPE reg2){
    
    return ((get_reg_byte(emu, reg1) << 8) | get_reg_byte(emu, reg2));
}

static inline uint8_t readByte(Emulator* emu){
    return (uint8_t)(read_address(emu, emu->rPC++));
}

static inline uint8_t readByte4C(Emulator* emu){
    uint8_t byte = readByte(emu);
    cyclesSync(emu);
    return byte;
}

static inline uint16_t read2Bytes(Emulator* emu) {
    return (uint16_t)(read_address(emu, emu->rPC++) | (read_address(emu, emu->rPC++) << 8));
}

static inline uint16_t read2Bytes_8C(Emulator* emu) {
    uint16_t entireThing = read2Bytes(emu);
    cyclesSync(emu);
    cyclesSync(emu);

    return entireThing;
}

///////////////////////////////////
/////// INNER CPU FUNCTIONS //////
/////////////////////////////////

static void increment_R_8(Emulator* emulator, REGISTER_TYPE reg){
    uint8_t old_val = get_reg_byte(emulator, reg);
    INC_R(emulator, reg);
    
    SET_FLAG_Z(emulator, get_reg_byte(emulator, reg));
    set_flag(emulator, FLAG_N, 0);
    SET_FLAG_H_ADD(emulator, old_val, 1);
}

static void decrement_R_8(Emulator* emulator, REGISTER_TYPE reg){
    uint8_t old_val = get_reg_byte(emulator, reg);
    DEC_R(emulator, reg);
    
    SET_FLAG_Z(emulator, get_reg_byte(emulator, reg));
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

static void JumpConditionRelative(Emulator* emulator, bool condition){
    int8_t byte = (int8_t)readByte4C(emulator);
    if (condition == true){
        emulator->rPC += byte;
        
        // update clock by +4c
        cyclesSync(emulator);
    }
    
}

  ////////////////////////////////////
 ///////// ROTATE FUNCTIONS /////////
////////////////////////////////////

static void rotateLeftR8(Emulator* emulator, REGISTER_TYPE reg, bool zflag){
    
    uint8_t val = get_reg_byte(emulator, reg);
    uint8_t bitno7 = val >> 7;

    val <<= 1;
    val |= bitno7;

    write_to_reg(emulator, reg, val);

    if (zflag) {
        SET_FLAG_Z(emulator, val);
    } else {
        set_flag(emulator, FLAG_Z, 0);
    }

    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bitno7);
}

// Rotates value to left, moves bit 7 to C flag and C flag's original value to bit 0
static void RotateLeftCarryR8(Emulator* emulator, REGISTER_TYPE reg, bool zflag) {
    uint8_t val = get_reg_byte(emulator, reg);
    bool carry_flag = get_flag(emulator, FLAG_C);
    uint8_t bit7 = val >> 7;

    val <<= 1;
    val |= carry_flag;

    write_to_reg(emulator, reg, val);

    if (zflag) {
        SET_FLAG_Z(emulator, val);
    } else {
        set_flag(emulator, FLAG_Z, 0);
    }

    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit7);
}

static void RotateLeftCarryR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2, bool zflag) {
    uint16_t address = get_reg16(emulator, reg1, reg2);
    uint8_t val = read_address_4C(emulator, address);
    bool carry_flag = get_flag(emulator, FLAG_C);
    uint8_t bit7 = val >> 7;

    val <<= 1;
    val |= carry_flag;

    write_address_4C(emulator, address, val);

    if (zflag) {
        SET_FLAG_Z(emulator, val);
    } else {
        set_flag(emulator, FLAG_Z, 0);
    }

    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit7);
}

static void rotateLeftR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2, bool zflag){
    
    uint16_t address = get_reg16(emulator, reg1, reg2);
    uint8_t val = read_address_4C(emulator, address);
    uint8_t bitno7 = val >> 7;

    val <<= 1;
    val |= bitno7;

    write_address_4C(emulator, address, val);

    if (zflag) {
        SET_FLAG_Z(emulator, val);
    } else {
        set_flag(emulator, FLAG_Z, 0);
    }

    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bitno7);
}

static void rotateRightR8(Emulator* emulator, REGISTER_TYPE reg, bool zflag) {
    uint8_t val = get_reg_byte(emulator, reg);
    uint8_t bitno1 = val & 1;

    val >>= 1;
    val |= bitno1 << 7;

    write_to_reg(emulator, reg, val);

    if (zflag) {
        SET_FLAG_Z(emulator, val);
    } else {
        set_flag(emulator, FLAG_Z, 0);
    }

    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bitno1);
}

// Rotates value to right, moves bit 0 to C flag and C flag's original value to bit 7 
static void RotateRightCarryR8(Emulator* emulator, REGISTER_TYPE reg, bool zflag){
    uint8_t val = get_reg_byte(emulator, reg);
    bool carry_flag = get_flag(emulator, FLAG_C);
    uint8_t bit0 = val & 1;

    val >>= 1;
    val |= carry_flag << 7;

    write_to_reg(emulator, reg, val);

    if (zflag) {
        SET_FLAG_Z(emulator, val);
    } else {
        set_flag(emulator, FLAG_Z, 0);
    }

    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit0);
}

static void RotateRightCarryR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2, bool zflag){
    uint16_t address = get_reg16(emulator, reg1, reg2);
    uint8_t val = read_address_4C(emulator, address);
    bool carry_flag = get_flag(emulator, FLAG_C);
    uint8_t bit0 = val & 1;

    val >>= 1;
    val |= carry_flag << 7;

    write_address_4C(emulator, address, val);

    if (zflag) {
        SET_FLAG_Z(emulator, val);
    } else {
        set_flag(emulator, FLAG_Z, 0);
    }

    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit0);
}

static void rotateRightR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2, bool zflag) {
    uint16_t address = get_reg16(emulator, reg1, reg2);
    uint8_t val = read_address_4C(emulator, address);
    uint8_t bitno1 = val & 1;

    val >>= 1;
    val |= bitno1 << 7;

    write_address_4C(emulator, address, val);

    if (zflag) {
        SET_FLAG_Z(emulator, val);
    } else {
        set_flag(emulator, FLAG_Z, 0);
    }

    set_flag(emulator, FLAG_Z, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bitno1);
}

  //////////////////////////////////////////////
 ///////// SHIFT ARITHMETIC FUNCTIONS /////////
//////////////////////////////////////////////

static void shiftLeftArithmeticR8(Emulator* emulator, REGISTER_TYPE reg){
    uint8_t reg_val = get_reg_byte(emulator, reg);
    uint8_t bit7 = reg_val >> 7;
    uint8_t result = reg_val << 1;

    write_to_reg(emulator, reg, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit7);
}

static void shiftLeftArithmeticR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2){
    uint16_t address = get_reg16(emulator, reg1, reg2);
    uint8_t reg_val = read_address_4C(emulator, address);
    uint8_t bit7 = reg_val >> 7;
    uint8_t result = reg_val << 1;

    write_address_4C(emulator, address, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit7);
}

static void shiftRightArithmeticR8(Emulator* emulator, REGISTER_TYPE reg){
    uint8_t value = get_reg_byte(emulator, reg);
    uint8_t bit7 = value >> 7;
    uint8_t bit0 = value & 0x1;
    uint8_t result = value >> 1;

    result |= bit7 << 7;
    write_to_reg(emulator, reg, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit0);
}

static void shiftRightArithmeticR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2){
    uint16_t address = get_reg16(emulator, reg1, reg2);
    uint8_t value = read_address_4C(emulator, address);
    uint8_t bit7 = value >> 7;
    uint8_t bit0 = value & 0x1;
    uint8_t result = value >> 1;

    result |= bit7 << 7;
    write_address_4C(emulator, address, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit0);
}

  /////////////////////////////////////////////////
 ///////// SHIFT RIGHT LOGICAL FUNCTIONS /////////
/////////////////////////////////////////////////

static void shiftRightLogicalR8(Emulator* emulator, REGISTER_TYPE reg){
    uint8_t value = get_reg_byte(emulator, reg);
    uint8_t bit1 = value & 0x1;
    uint8_t result = value >> 1;

    write_to_reg(emulator, reg, result);
    
    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit1);
}

static void shiftRightLogicalR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2){
    uint16_t address = get_reg16(emulator, reg1, reg2);
    uint8_t value = read_address_4C(emulator, address);
    uint8_t bit1 = value & 0x1;
    uint8_t result = value >> 1;

    write_address_4C(emulator, address, result);

    SET_FLAG_Z(emulator, result);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, bit1);
}

  //////////////////////////////////
 ///////// SWAP FUNCTIONS /////////
//////////////////////////////////

static void swapR8(Emulator* emulator, REGISTER_TYPE reg){
    uint8_t value = get_reg_byte(emulator, reg);
    uint8_t high = value >> 4;
    uint8_t low = value & 0xF;

    uint8_t new = (low << 4) | high;

    write_to_reg(emulator, reg, new);

    SET_FLAG_Z(emulator, new);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, 0);
}

static void swapR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2){
    uint16_t address = get_reg16(emulator, reg1, reg2);
    uint8_t value = read_address_4C(emulator, address);
    uint8_t high = value >> 4;
    uint8_t low = value & 0xF;

    uint8_t new = (low << 4) | high;

    write_address_4C(emulator, address, value);

    SET_FLAG_Z(emulator, new);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_C, 0);
}

static void BitR8(Emulator* emulator, REGISTER_TYPE reg, uint8_t bit){
    uint8_t value = get_reg_byte(emulator, reg);
    uint8_t bitvalue = (value >> bit) & 0x1;

    SET_FLAG_Z(emulator, bitvalue);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 1);
}

static void BitR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2, uint8_t bit){
    uint8_t value = read_address_4C(emulator, get_reg16(emulator, reg1, reg2));
    uint8_t bitvalue = (value >> bit) & 0x1;

    SET_FLAG_Z(emulator, bitvalue);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 1);
}

static void setR8(Emulator* emulator, REGISTER_TYPE reg, uint8_t bit){
    uint8_t value = get_reg_byte(emulator, reg);
    uint8_t orvalue = 1 << bit;
    uint8_t result = value | orvalue;

    write_to_reg(emulator, reg, result);
}

static void setR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2, uint8_t bit){
    uint16_t address = get_reg16(emulator, reg1, reg2);
    uint8_t value = read_address_4C(emulator, address);
    uint8_t orvalue = 1 << bit;
    uint8_t result = value | orvalue;

    write_address_4C(emulator, address, result);
}

static void resetR8(Emulator* emulator, REGISTER_TYPE reg, uint8_t bit){
    uint8_t value = get_reg_byte(emulator, reg);
    uint8_t andvalue = ~(1 << bit);
    uint8_t result = value & andvalue;

    write_to_reg(emulator, reg, result);
}

static void resetR16(Emulator* emulator, REGISTER_TYPE reg1, REGISTER_TYPE reg2, uint8_t bit){
    uint16_t address = get_reg16(emulator, reg1, reg2);
    uint8_t value = read_address_4C(emulator, address);
    uint8_t andvalue = ~(1 << bit);
    uint8_t result = value & andvalue;

    write_address_4C(emulator, address, result);
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

    cyclesSync(emulator);
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
    uint8_t byte2 = readByte4C(emulator);

    uint8_t final_val = byte1 + byte2;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    SET_FLAG_H_ADD(emulator, byte1, byte2);
    SET_FLAG_C_ADD(emulator, byte1, byte2);
}

static void addR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address_4C(emulator, get_reg16(emulator, r2, r3));
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
    uint8_t byte = readByte4C(emulator);
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
    uint8_t __r2_3_val = read_address_4C(emulator, get_reg16(emulator, r2, r3));
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
    uint8_t byte = readByte4C(emulator);
    uint8_t final = r1_val - byte;

    write_to_reg(emulator, r1, final);

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 1);
    SET_FLAG_H_SUB(emulator, r1_val, byte);
    SET_FLAG_C_SUB(emulator, r1_val, byte);
}

static void subR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address_4C(emulator, get_reg16(emulator, r2, r3));
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
    uint8_t byte = readByte4C(emulator);
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
    uint8_t __r2_3_val = read_address_4C(emulator, get_reg16(emulator, r2, r3));
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
    uint8_t byte = readByte4C(emulator);
    uint8_t final_val = r1_val & byte;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 1);
    set_flag(emulator, FLAG_C, 0);
}

static void andR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address_4C(emulator, get_reg16(emulator, r2, r3));
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
    uint8_t byte = readByte4C(emulator);
    uint8_t final_val = r1_val ^ byte;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_C, 0);
}

static void xorR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address_4C(emulator, get_reg16(emulator, r2, r3));
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
    uint8_t byte = readByte4C(emulator);
    uint8_t final_val = r1_val | byte;

    write_to_reg(emulator, r1, final_val);
    
    SET_FLAG_Z(emulator, final_val);
    set_flag(emulator, FLAG_N, 0);
    set_flag(emulator, FLAG_H, 0);
    set_flag(emulator, FLAG_C, 0);
}

static void orR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address_4C(emulator, get_reg16(emulator, r2, r3));
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
    uint8_t byte = readByte4C(emulator);
    uint8_t final = r1_val - byte;

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 1);
    SET_FLAG_H_SUB(emulator, r1_val, byte);
    SET_FLAG_C_SUB(emulator, r1_val, byte);
}

static void compareR_RR(Emulator* emulator, REGISTER_TYPE r1, REGISTER_TYPE r2, REGISTER_TYPE r3){
    uint8_t __r1_val = get_reg_byte(emulator, r1);
    uint8_t __r2_3_val = read_address_4C(emulator, get_reg16(emulator, r2, r3));
    uint8_t final = __r1_val - __r2_3_val;

    SET_FLAG_Z(emulator, final);
    set_flag(emulator, FLAG_N, 1);
    SET_FLAG_H_SUB(emulator, __r1_val, __r2_3_val);
    SET_FLAG_C_SUB(emulator, __r1_val, __r2_3_val);
}

static inline uint16_t pop16(Emulator* emulator){
    uint16_t sp_val = emulator->rSP;

    uint8_t before_byte = read_address_4C(emulator, sp_val);
    uint8_t after_byte = read_address_4C(emulator, sp_val + 1);
    
    emulator->rSP = sp_val + 2;

    return (uint16_t)((after_byte << 8) | before_byte);
}

static inline void push16(Emulator* emulator, uint16_t u16byte){
    uint16_t sp = emulator->rSP;

    write_address_4C(emulator, sp - 1, u16byte >> 8);
    write_address_4C(emulator, sp - 2, u16byte & 0xFF);

    emulator->rSP = sp - 2;
}

static inline void ret(Emulator* emulator){
    emulator->rPC = pop16(emulator);

    cyclesSync(emulator);
}

static void return_condition(Emulator* emulator, bool condition){
    cyclesSync(emulator);
    if (condition == true)ret(emulator);
}

static void jumpCondition(Emulator* emulator, bool condition){
    uint16_t address = read2Bytes(emulator);

    if (condition == true){
        cyclesSync(emulator);
        emulator->rPC = address;
    }
}

// Call pushes reg PC and sets reg PC to the given address
static void call(Emulator* emulator, uint16_t address){
    cyclesSync(emulator);
    push16(emulator, emulator->rPC);
    emulator->rPC = address;
}

static void callCondition(Emulator* emulator, uint16_t address, bool condition){
    if (condition == true) {
        call(emulator, address);
    }
}

static bool checkIME(Emulator* emulator, uint8_t IE, uint8_t IF){
    if (!INTERRUPT_REQ(IE, IF)) {
        //printf("=== HALTING EMULATOR ===\n");
        emulator->haltMode = true;
        
        return true;
    }
    return false;
}

static void dispatch_interrupt(Emulator* emulator, INTERRUPT interrupt){
    /* This does 8 cycles */
    
    IMD(emulator);

    emulator->IF &= ~(1 << interrupt);

    cyclesSync(emulator);
    cyclesSync(emulator);

    switch (interrupt)
    {
        case INTERRUPT_VBLANK:      printf("\t-> INTERRUPT_VBLANK\n");    call(emulator, 0x40); break;
        case INTERRUPT_LCD_STAT:    printf("\t-> INTERRUPT_LCD_STAT\n");  call(emulator, 0x48); break;
        case INTERRUPT_TIMER:       printf("\t-> INTERRUPT_TIMER\n");     call(emulator, 0x50); break;
        case INTERRUPT_SERIAL:      printf("\t-> INTERRUPT_SERIAL\n");    call(emulator, 0x58); break;
        case INTERRUPT_JOYPAD:      printf("\t-> INTERRUPT_JOYPAD\n");    call(emulator, 0x60); break;
        default:                                                                                break;
    }
}

static void halt(Emulator* emulator){
    uint8_t IE = emulator->IE;
    uint8_t IF = emulator->IF;

    if (emulator->IME){
        checkIME(emulator, IE, IF);
        return;
    } else {
        if (!checkIME(emulator, IE, IF)){
            emulator->schedule_halt_bug = true;
        }
    }
}

static void stop_halt(Emulator* emulator){
    //printf("=== +SUCCESSFULLY EXITED HALT MODE+ ===\n");
    emulator->haltMode = false;
}

static void handleInterrupts(Emulator* emulator){
    //printf("=> Handling interrupts ");
    uint8_t IE = emulator->IE; // enabled interrupts
    uint8_t IF = emulator->IF; // requested interrupts

    if (emulator->IME){
        //printf("IME ");

        if (INTERRUPT_REQ(IE, IF)){ // If interrupt was requested

            stop_halt(emulator);

            // There has been atleast 1 interrupt enabled

            //printf("Interrupt no. ");
            for (int i = 0; i < 5; i++){
                //printf("%d ", i);

                uint8_t requestBit = (IF >> i) & 0x1;
                uint8_t enabledBit = (IE >> i) & 0x1;

                if (requestBit && enabledBit) {
                    // dispatch the highest priorty interrupt 'i'
                    dispatch_interrupt(emulator, i);
                    return;
                }
            }
            //printf("\n");
        } else {
            //printf("XX\n");
        }
    } else {
        if (INTERRUPT_REQ(IE, IF)) {
            stop_halt(emulator);
        }
        //printf("\n");
    }
}

static void prefixCB(Emulator* emulator){
    uint8_t byte = readByte(emulator);
    cyclesSync(emulator);

    //printRegisters(emulator);
    //printCBInstruction(emulator, byte);

    switch (byte)
    {
        case 0x00: rotateLeftR8(emulator, REGISTER_B, true); break;
        case 0x01: rotateLeftR8(emulator, REGISTER_C, true); break;
        case 0x02: rotateLeftR8(emulator, REGISTER_D, true); break;
        case 0x03: rotateLeftR8(emulator, REGISTER_E, true); break;
        case 0x04: rotateLeftR8(emulator, REGISTER_H, true); break;
        case 0x05: rotateLeftR8(emulator, REGISTER_L, true); break;
        case 0x06: rotateLeftR16(emulator, REGISTER_H, REGISTER_L, true); break;
        case 0x07: rotateLeftR8(emulator, REGISTER_A, true); break;
        case 0x08: rotateRightR8(emulator, REGISTER_B, true); break;
        case 0x09: rotateRightR8(emulator, REGISTER_C, true); break;
        case 0x0A: rotateRightR8(emulator, REGISTER_D, true); break;
        case 0x0B: rotateRightR8(emulator, REGISTER_E, true); break;
        case 0x0C: rotateRightR8(emulator, REGISTER_H, true); break;
        case 0x0D: rotateRightR8(emulator, REGISTER_L, true); break;
        case 0x0E: rotateRightR16(emulator, REGISTER_H, REGISTER_L, true); break;
        case 0x0F: rotateRightR8(emulator, REGISTER_A, true); break;
        
        case 0x10: RotateLeftCarryR8(emulator, REGISTER_B, true); break;
        case 0x11: RotateLeftCarryR8(emulator, REGISTER_C, true); break;
        case 0x12: RotateLeftCarryR8(emulator, REGISTER_D, true); break;
        case 0x13: RotateLeftCarryR8(emulator, REGISTER_E, true); break;
        case 0x14: RotateLeftCarryR8(emulator, REGISTER_H, true); break;
        case 0x15: RotateLeftCarryR8(emulator, REGISTER_L, true); break;
        case 0x16: RotateLeftCarryR16(emulator, REGISTER_H, REGISTER_L, true); break;
        case 0x17: RotateLeftCarryR8(emulator, REGISTER_A, true); break;
        case 0x18: RotateRightCarryR8(emulator, REGISTER_B, true); break;
        case 0x19: RotateRightCarryR8(emulator, REGISTER_C, true); break;
        case 0x1A: RotateRightCarryR8(emulator, REGISTER_D, true); break;
        case 0x1B: RotateRightCarryR8(emulator, REGISTER_E, true); break;
        case 0x1C: RotateRightCarryR8(emulator, REGISTER_H, true); break;
        case 0x1D: RotateRightCarryR8(emulator, REGISTER_L, true); break;
        case 0x1E: RotateRightCarryR16(emulator, REGISTER_H, REGISTER_L, true); break;
        case 0x1F: RotateRightCarryR8(emulator, REGISTER_A, true); break;
        
        case 0x20: shiftLeftArithmeticR8(emulator, REGISTER_B); break;
        case 0x21: shiftLeftArithmeticR8(emulator, REGISTER_C); break;
        case 0x22: shiftLeftArithmeticR8(emulator, REGISTER_D); break;
        case 0x23: shiftLeftArithmeticR8(emulator, REGISTER_E); break;
        case 0x24: shiftLeftArithmeticR8(emulator, REGISTER_H); break;
        case 0x25: shiftLeftArithmeticR8(emulator, REGISTER_L); break;
        case 0x26: shiftLeftArithmeticR16(emulator, REGISTER_H, REGISTER_L); break;
        case 0x27: shiftLeftArithmeticR8(emulator, REGISTER_A); break;
        case 0x28: shiftRightArithmeticR8(emulator, REGISTER_B); break;
        case 0x29: shiftRightArithmeticR8(emulator, REGISTER_C); break;
        case 0x2A: shiftRightArithmeticR8(emulator, REGISTER_D); break;
        case 0x2B: shiftRightArithmeticR8(emulator, REGISTER_E); break;
        case 0x2C: shiftRightArithmeticR8(emulator, REGISTER_H); break;
        case 0x2D: shiftRightArithmeticR8(emulator, REGISTER_L); break;
        case 0x2E: shiftRightArithmeticR16(emulator, REGISTER_H, REGISTER_L); break;
        case 0x2F: shiftRightArithmeticR8(emulator, REGISTER_A); break;

        case 0x30: swapR8(emulator, REGISTER_B); break;
        case 0x31: swapR8(emulator, REGISTER_C); break;
        case 0x32: swapR8(emulator, REGISTER_D); break;
        case 0x33: swapR8(emulator, REGISTER_E); break;
        case 0x34: swapR8(emulator, REGISTER_H); break;
        case 0x35: swapR8(emulator, REGISTER_L); break;
        case 0x36: swapR16(emulator, REGISTER_H, REGISTER_L); break;
        case 0x37: swapR8(emulator, REGISTER_A); break;
        case 0x38: shiftRightLogicalR8(emulator, REGISTER_B); break;
        case 0x39: shiftRightLogicalR8(emulator, REGISTER_C); break;
        case 0x3A: shiftRightLogicalR8(emulator, REGISTER_D); break;
        case 0x3B: shiftRightLogicalR8(emulator, REGISTER_E); break;
        case 0x3C: shiftRightLogicalR8(emulator, REGISTER_H); break;
        case 0x3D: shiftRightLogicalR8(emulator, REGISTER_L); break;
        case 0x3E: shiftRightLogicalR16(emulator, REGISTER_H, REGISTER_L); break;
        case 0x3F: shiftRightLogicalR8(emulator, REGISTER_A); break;

        case 0x40: BitR8(emulator, REGISTER_B, 0); break;
        case 0x41: BitR8(emulator, REGISTER_C, 0); break;
        case 0x42: BitR8(emulator, REGISTER_D, 0); break;
        case 0x43: BitR8(emulator, REGISTER_E, 0); break;
        case 0x44: BitR8(emulator, REGISTER_H, 0); break;
        case 0x45: BitR8(emulator, REGISTER_L, 0); break;
        case 0x46: BitR16(emulator, REGISTER_H, REGISTER_L, 0); break;
        case 0x47: BitR8(emulator, REGISTER_A, 0); break;
        case 0x48: BitR8(emulator, REGISTER_B, 1); break;
        case 0x49: BitR8(emulator, REGISTER_C, 1); break;
        case 0x4A: BitR8(emulator, REGISTER_D, 1); break;
        case 0x4B: BitR8(emulator, REGISTER_E, 1); break;
        case 0x4C: BitR8(emulator, REGISTER_H, 1); break;
        case 0x4D: BitR8(emulator, REGISTER_L, 1); break;
        case 0x4E: BitR16(emulator, REGISTER_H, REGISTER_L, 1); break;
        case 0x4F: BitR8(emulator, REGISTER_A, 1); break;

        case 0x50: BitR8(emulator, REGISTER_B, 2); break;
        case 0x51: BitR8(emulator, REGISTER_C, 2); break;
        case 0x52: BitR8(emulator, REGISTER_D, 2); break;
        case 0x53: BitR8(emulator, REGISTER_E, 2); break;
        case 0x54: BitR8(emulator, REGISTER_H, 2); break;
        case 0x55: BitR8(emulator, REGISTER_L, 2); break;
        case 0x56: BitR16(emulator, REGISTER_H, REGISTER_L, 2); break;
        case 0x57: BitR8(emulator, REGISTER_A, 2); break;
        case 0x58: BitR8(emulator, REGISTER_B, 3); break;
        case 0x59: BitR8(emulator, REGISTER_C, 3); break;
        case 0x5A: BitR8(emulator, REGISTER_D, 3); break;
        case 0x5B: BitR8(emulator, REGISTER_E, 3); break;
        case 0x5C: BitR8(emulator, REGISTER_H, 3); break;
        case 0x5D: BitR8(emulator, REGISTER_L, 3); break;
        case 0x5E: BitR16(emulator, REGISTER_H, REGISTER_L, 3); break;
        case 0x5F: BitR8(emulator, REGISTER_A, 3); break;

        case 0x60: BitR8(emulator, REGISTER_B, 4); break;
        case 0x61: BitR8(emulator, REGISTER_C, 4); break;
        case 0x62: BitR8(emulator, REGISTER_D, 4); break;
        case 0x63: BitR8(emulator, REGISTER_E, 4); break;
        case 0x64: BitR8(emulator, REGISTER_H, 4); break;
        case 0x65: BitR8(emulator, REGISTER_L, 4); break;
        case 0x66: BitR16(emulator, REGISTER_H, REGISTER_L, 4); break;
        case 0x67: BitR8(emulator, REGISTER_A, 4); break;
        case 0x68: BitR8(emulator, REGISTER_B, 5); break;
        case 0x69: BitR8(emulator, REGISTER_C, 5); break;
        case 0x6A: BitR8(emulator, REGISTER_D, 5); break;
        case 0x6B: BitR8(emulator, REGISTER_E, 5); break;
        case 0x6C: BitR8(emulator, REGISTER_H, 5); break;
        case 0x6D: BitR8(emulator, REGISTER_L, 5); break;
        case 0x6E: BitR16(emulator, REGISTER_H, REGISTER_L, 5); break;
        case 0x6F: BitR8(emulator, REGISTER_A, 5); break;

        case 0x70: BitR8(emulator, REGISTER_B, 6); break;
        case 0x71: BitR8(emulator, REGISTER_C, 6); break;
        case 0x72: BitR8(emulator, REGISTER_D, 6); break;
        case 0x73: BitR8(emulator, REGISTER_E, 6); break;
        case 0x74: BitR8(emulator, REGISTER_H, 6); break;
        case 0x75: BitR8(emulator, REGISTER_L, 6); break;
        case 0x76: BitR16(emulator, REGISTER_H, REGISTER_L, 6); break;
        case 0x77: BitR8(emulator, REGISTER_A, 6); break;
        case 0x78: BitR8(emulator, REGISTER_B, 7); break;
        case 0x79: BitR8(emulator, REGISTER_C, 7); break;
        case 0x7A: BitR8(emulator, REGISTER_D, 7); break;
        case 0x7B: BitR8(emulator, REGISTER_E, 7); break;
        case 0x7C: BitR8(emulator, REGISTER_H, 7); break;
        case 0x7D: BitR8(emulator, REGISTER_L, 7); break;
        case 0x7E: BitR16(emulator, REGISTER_H, REGISTER_L, 7); break;
        case 0x7F: BitR8(emulator, REGISTER_A, 7); break;
        
        case 0x80: resetR8(emulator, REGISTER_B, 0); break;
        case 0x81: resetR8(emulator, REGISTER_C, 0); break;
        case 0x82: resetR8(emulator, REGISTER_D, 0); break;
        case 0x83: resetR8(emulator, REGISTER_E, 0); break;
        case 0x84: resetR8(emulator, REGISTER_H, 0); break;
        case 0x85: resetR8(emulator, REGISTER_L, 0); break;
        case 0x86: resetR16(emulator, REGISTER_H, REGISTER_L, 0); break;
        case 0x87: resetR8(emulator, REGISTER_A, 0); break;
        case 0x88: resetR8(emulator, REGISTER_B, 1); break;
        case 0x89: resetR8(emulator, REGISTER_C, 1); break;
        case 0x8A: resetR8(emulator, REGISTER_D, 1); break;
        case 0x8B: resetR8(emulator, REGISTER_E, 1); break;
        case 0x8C: resetR8(emulator, REGISTER_H, 1); break;
        case 0x8D: resetR8(emulator, REGISTER_L, 1); break;
        case 0x8E: resetR16(emulator, REGISTER_H, REGISTER_L, 1); break;
        case 0x8F: resetR8(emulator, REGISTER_A, 1); break;
        
        case 0x90: resetR8(emulator, REGISTER_B, 2); break;
        case 0x91: resetR8(emulator, REGISTER_C, 2); break;
        case 0x92: resetR8(emulator, REGISTER_D, 2); break;
        case 0x93: resetR8(emulator, REGISTER_E, 2); break;
        case 0x94: resetR8(emulator, REGISTER_H, 2); break;
        case 0x95: resetR8(emulator, REGISTER_L, 2); break;
        case 0x96: resetR16(emulator, REGISTER_H, REGISTER_L, 2); break;
        case 0x97: resetR8(emulator, REGISTER_A, 2); break;
        case 0x98: resetR8(emulator, REGISTER_B, 3); break;
        case 0x99: resetR8(emulator, REGISTER_C, 3); break;
        case 0x9A: resetR8(emulator, REGISTER_D, 3); break;
        case 0x9B: resetR8(emulator, REGISTER_E, 3); break;
        case 0x9C: resetR8(emulator, REGISTER_H, 3); break;
        case 0x9D: resetR8(emulator, REGISTER_L, 3); break;
        case 0x9E: resetR16(emulator, REGISTER_H, REGISTER_L, 3); break;
        case 0x9F: resetR8(emulator, REGISTER_A, 3); break;
        
        case 0xA0: resetR8(emulator, REGISTER_B, 4); break;
        case 0xA1: resetR8(emulator, REGISTER_C, 4); break;
        case 0xA2: resetR8(emulator, REGISTER_D, 4); break;
        case 0xA3: resetR8(emulator, REGISTER_E, 4); break;
        case 0xA4: resetR8(emulator, REGISTER_H, 4); break;
        case 0xA5: resetR8(emulator, REGISTER_L, 4); break;
        case 0xA6: resetR16(emulator, REGISTER_H, REGISTER_L, 4); break;
        case 0xA7: resetR8(emulator, REGISTER_A, 4); break;
        case 0xA8: resetR8(emulator, REGISTER_B, 5); break;
        case 0xA9: resetR8(emulator, REGISTER_C, 5); break;
        case 0xAA: resetR8(emulator, REGISTER_D, 5); break;
        case 0xAB: resetR8(emulator, REGISTER_E, 5); break;
        case 0xAC: resetR8(emulator, REGISTER_H, 5); break;
        case 0xAD: resetR8(emulator, REGISTER_L, 5); break;
        case 0xAE: resetR16(emulator, REGISTER_H, REGISTER_L, 5); break;
        case 0xAF: resetR8(emulator, REGISTER_A, 5); break;
        
        case 0xB0: resetR8(emulator, REGISTER_B, 6); break;
        case 0xB1: resetR8(emulator, REGISTER_C, 6); break;
        case 0xB2: resetR8(emulator, REGISTER_D, 6); break;
        case 0xB3: resetR8(emulator, REGISTER_E, 6); break;
        case 0xB4: resetR8(emulator, REGISTER_H, 6); break;
        case 0xB5: resetR8(emulator, REGISTER_L, 6); break;
        case 0xB6: resetR16(emulator, REGISTER_H, REGISTER_L, 6); break;
        case 0xB7: resetR8(emulator, REGISTER_A, 6); break;
        case 0xB8: resetR8(emulator, REGISTER_B, 7); break;
        case 0xB9: resetR8(emulator, REGISTER_C, 7); break;
        case 0xBA: resetR8(emulator, REGISTER_D, 7); break;
        case 0xBB: resetR8(emulator, REGISTER_E, 7); break;
        case 0xBC: resetR8(emulator, REGISTER_H, 7); break;
        case 0xBD: resetR8(emulator, REGISTER_L, 7); break;
        case 0xBE: resetR16(emulator, REGISTER_H, REGISTER_L, 7); break;
        case 0xBF: resetR8(emulator, REGISTER_A, 7); break;

        case 0xC0: setR8(emulator, REGISTER_B, 0); break;
        case 0xC1: setR8(emulator, REGISTER_C, 0); break;
        case 0xC2: setR8(emulator, REGISTER_D, 0); break;
        case 0xC3: setR8(emulator, REGISTER_E, 0); break;
        case 0xC4: setR8(emulator, REGISTER_H, 0); break;
        case 0xC5: setR8(emulator, REGISTER_L, 0); break;
        case 0xC6: setR16(emulator, REGISTER_H, REGISTER_L, 0); break;
        case 0xC7: setR8(emulator, REGISTER_A, 0); break;
        case 0xC8: setR8(emulator, REGISTER_B, 1); break;
        case 0xC9: setR8(emulator, REGISTER_C, 1); break;
        case 0xCA: setR8(emulator, REGISTER_D, 1); break;
        case 0xCB: setR8(emulator, REGISTER_E, 1); break;
        case 0xCC: setR8(emulator, REGISTER_H, 1); break;
        case 0xCD: setR8(emulator, REGISTER_L, 1); break;
        case 0xCE: setR16(emulator, REGISTER_H, REGISTER_L, 1); break;
        case 0xCF: setR8(emulator, REGISTER_A, 1); break;

        case 0xD0: setR8(emulator, REGISTER_B, 2); break;
        case 0xD1: setR8(emulator, REGISTER_C, 2); break;
        case 0xD2: setR8(emulator, REGISTER_D, 2); break;
        case 0xD3: setR8(emulator, REGISTER_E, 2); break;
        case 0xD4: setR8(emulator, REGISTER_H, 2); break;
        case 0xD5: setR8(emulator, REGISTER_L, 2); break;
        case 0xD6: setR16(emulator, REGISTER_H, REGISTER_L, 2); break;
        case 0xD7: setR8(emulator, REGISTER_A, 2); break;
        case 0xD8: setR8(emulator, REGISTER_B, 3); break;
        case 0xD9: setR8(emulator, REGISTER_C, 3); break;
        case 0xDA: setR8(emulator, REGISTER_D, 3); break;
        case 0xDB: setR8(emulator, REGISTER_E, 3); break;
        case 0xDC: setR8(emulator, REGISTER_H, 3); break;
        case 0xDD: setR8(emulator, REGISTER_L, 3); break;
        case 0xDE: setR16(emulator, REGISTER_H, REGISTER_L, 3); break;
        case 0xDF: setR8(emulator, REGISTER_A, 3); break;

        case 0xE0: setR8(emulator, REGISTER_B, 4); break;
        case 0xE1: setR8(emulator, REGISTER_C, 4); break;
        case 0xE2: setR8(emulator, REGISTER_D, 4); break;
        case 0xE3: setR8(emulator, REGISTER_E, 4); break;
        case 0xE4: setR8(emulator, REGISTER_H, 4); break;
        case 0xE5: setR8(emulator, REGISTER_L, 4); break;
        case 0xE6: setR16(emulator, REGISTER_H, REGISTER_L, 4); break;
        case 0xE7: setR8(emulator, REGISTER_A, 4); break;
        case 0xE8: setR8(emulator, REGISTER_B, 5); break;
        case 0xE9: setR8(emulator, REGISTER_C, 5); break;
        case 0xEA: setR8(emulator, REGISTER_D, 5); break;
        case 0xEB: setR8(emulator, REGISTER_E, 5); break;
        case 0xEC: setR8(emulator, REGISTER_H, 5); break;
        case 0xED: setR8(emulator, REGISTER_L, 5); break;
        case 0xEE: setR16(emulator, REGISTER_H, REGISTER_L, 5); break;
        case 0xEF: setR8(emulator, REGISTER_A, 5); break;

        case 0xF0: setR8(emulator, REGISTER_B, 6); break;
        case 0xF1: setR8(emulator, REGISTER_C, 6); break;
        case 0xF2: setR8(emulator, REGISTER_D, 6); break;
        case 0xF3: setR8(emulator, REGISTER_E, 6); break;
        case 0xF4: setR8(emulator, REGISTER_H, 6); break;
        case 0xF5: setR8(emulator, REGISTER_L, 6); break;
        case 0xF6: setR16(emulator, REGISTER_H, REGISTER_L, 6); break;
        case 0xF7: setR8(emulator, REGISTER_A, 6); break;
        case 0xF8: setR8(emulator, REGISTER_B, 7); break;
        case 0xF9: setR8(emulator, REGISTER_C, 7); break;
        case 0xFA: setR8(emulator, REGISTER_D, 7); break;
        case 0xFB: setR8(emulator, REGISTER_E, 7); break;
        case 0xFC: setR8(emulator, REGISTER_H, 7); break;
        case 0xFD: setR8(emulator, REGISTER_L, 7); break;
        case 0xFE: setR16(emulator, REGISTER_H, REGISTER_L, 7); break;
        case 0xFF: setR8(emulator, REGISTER_A, 7); break;
        default: break;
    }
}

void dispatch_emulator(Emulator* emulator){
    uint8_t opcode = 0;
    
    if (emulator->schedule_interrupt_enable == true){
        emulator->IME = true;
        emulator->schedule_interrupt_enable = false;
        //printf("INTURREPTS ENABLED");
    }

    if (emulator->haltMode == true){
        printf("HALTING EMULATOR\n");
        cyclesSync(emulator);
        goto skip;
    } else if (emulator->schedule_halt_bug == true) {
        printf("HALT BUG\n");
        opcode = readByte(emulator);
        emulator->rPC--;
        cyclesSync(emulator);
    } else {
        opcode = read_address_4C(emulator, emulator->rPC);
    }

    //printf("\t0x%02x| Instruction Details:", opcode);
    //printRegisters(emulator);
    //printInstruction(emulator);

    emulator->rPC ++;

    switch (opcode)
    {   
        case 0x00: break;
        case 0x01: LOAD_16B_RR(emulator, REGISTER_B, REGISTER_C); break; // LD BC,u16
        case 0x02: LOAD_16RB_R(emulator, REGISTER_B, REGISTER_C, REGISTER_A); break; // LD (BC),A
        case 0x03: INC_R1_R2(emulator, REGISTER_B, REGISTER_C); cyclesSync(emulator); break; // INC BC
        case 0x04: increment_R_8(emulator, REGISTER_B); break; // INC B
        case 0x05: decrement_R_8(emulator, REGISTER_B); break; // DEC B
        case 0x06: LOAD_8B_R(emulator, REGISTER_B); break; // LD B,u8
        case 0x07: rotateLeftR8(emulator, REGISTER_A, true); break; // RLC_ REG A
        case 0x08: {
            uint16_t two_bytes = read2Bytes_8C(emulator);
            uint16_t SP_VAL = emulator->rSP;

            write_address_4C(emulator, two_bytes + 1, SP_VAL >> 8);
            write_address_4C(emulator, two_bytes, SP_VAL & 0xFF);
            break;
        }
        case 0x09: addRR16(emulator, REGISTER_H, REGISTER_L, REGISTER_B, REGISTER_C);break;
        case 0x0A: LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_B, REGISTER_C);break;
        case 0x0B: DEC_R1_R2(emulator, REGISTER_B, REGISTER_C);break;
        case 0x0C: increment_R_8(emulator, REGISTER_C);break;
        case 0x0D: decrement_R_8(emulator, REGISTER_C); break;
        case 0x0E: LOAD_8B_R(emulator, REGISTER_C); break;
        case 0x0F: rotateRightR8(emulator, REGISTER_A, false); break;
        
        case 0x10: break; // STOP (stops cpu)
        case 0x11: LOAD_16B_RR(emulator, REGISTER_D, REGISTER_E);break;
        case 0x12: LOAD_16RB_R(emulator, REGISTER_D, REGISTER_E, REGISTER_A); break;
        case 0x13: INC_R1_R2(emulator, REGISTER_D, REGISTER_E); cyclesSync(emulator); break;
        case 0x14: increment_R_8(emulator, REGISTER_D); break;
        case 0x15: decrement_R_8(emulator, REGISTER_D); break;
        case 0x16: LOAD_8B_R(emulator, REGISTER_D); break;
        case 0x17: RotateLeftCarryR8(emulator, REGISTER_A, false); break;
        case 0x18: emulator->rPC += (uint8_t)readByte4C(emulator); cyclesSync(emulator); break; // JR i8
        case 0x19: addRR16(emulator, REGISTER_H, REGISTER_L, REGISTER_D, REGISTER_E); break;
        case 0x1A: LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_D, REGISTER_E); break;
        case 0x1B: DEC_R1_R2(emulator, REGISTER_D, REGISTER_E); break;
        case 0x1C: increment_R_8(emulator, REGISTER_E); break;
        case 0x1D: decrement_R_8(emulator, REGISTER_E); break;
        case 0x1E: LOAD_8B_R(emulator, REGISTER_E); break;
        case 0x1F: RotateRightCarryR8(emulator, REGISTER_A, false); break;
        
        case 0x20: JumpConditionRelative(emulator, NZ(emulator)); break; // JR NZ i8
        case 0x21: LOAD_16B_RR(emulator, REGISTER_H, REGISTER_L); break;
        case 0x22: LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_A); INC_R1_R2(emulator, REGISTER_H, REGISTER_L); break;
        case 0x23: INC_R1_R2(emulator, REGISTER_H, REGISTER_L); cyclesSync(emulator); break;
        case 0x24: increment_R_8(emulator, REGISTER_H); break;
        case 0x25: decrement_R_8(emulator, REGISTER_H); break;
        case 0x26: LOAD_8B_R(emulator, REGISTER_H); break;
        case 0x27: decimal_adjust(emulator); break;
        case 0x28: JumpConditionRelative(emulator, Z(emulator)); break; // JR Z i8
        case 0x29: addRR16(emulator, REGISTER_H, REGISTER_L, REGISTER_H, REGISTER_L); break;
        case 0x2A: LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); INC_R1_R2(emulator, REGISTER_H, REGISTER_L); break;
        case 0x2B: DEC_R1_R2(emulator, REGISTER_H, REGISTER_L); break;
        case 0x2C: increment_R_8(emulator, REGISTER_L); break;
        case 0x2D: decrement_R_8(emulator, REGISTER_L); break;
        case 0x2E: LOAD_8B_R(emulator,  REGISTER_L); break;
        case 0x2F: cpl(emulator); break; // CPL
        
        case 0x30: JumpConditionRelative(emulator, NC(emulator)); break; // JR NC i8
        case 0x31: emulator->rSP = read2Bytes(emulator); cyclesSync(emulator); cyclesSync(emulator); break;
        case 0x32: LOAD_16RB_R(emulator, REGISTER_H, REGISTER_L, REGISTER_A); DEC_R1_R2(emulator, REGISTER_H, REGISTER_L); break;
        case 0x33: emulator->rSP++; break;
        case 0x34: {
            // increment address in HL
            uint16_t address = get_reg16(emulator, REGISTER_H, REGISTER_L);
            uint8_t old = read_address_4C(emulator, address);
            uint8_t new = old + 1;

            SET_FLAG_Z(emulator, new);
            SET_FLAG_H_ADD16(emulator, old, 1);
            set_flag(emulator, FLAG_N, 0);
            write_address_4C(emulator, address, new);

            break;
        }
        case 0x35: {
            // opposite of 0x34 (decrement)
            uint16_t address = get_reg16(emulator, REGISTER_H, REGISTER_L);
            uint8_t old = read_address_4C(emulator, address);
            uint8_t new = old - 1;

            SET_FLAG_Z(emulator, new);
            SET_FLAG_H_ADD16(emulator, old, 1);
            set_flag(emulator, FLAG_N, 0);
            write_address_4C(emulator, address, new);

            break;
        }
        case 0x36: LOAD_u8_addr_u16(emulator, REGISTER_H, REGISTER_L); break;
        case 0x37: {
            set_flag(emulator, FLAG_C, 1);
            set_flag(emulator, FLAG_N, 0);
            set_flag(emulator, FLAG_H, 0);
            break;
        }
        case 0x38: JumpConditionRelative(emulator, C(emulator)); break; // JR C i8
        case 0x39: {
            // ADD HL SP
            uint16_t hl_byte = get_reg16(emulator, REGISTER_H, REGISTER_L);
            uint16_t sp_byte = emulator->rSP;
            uint16_t final = hl_byte + sp_byte;

            set_reg16(emulator, final, REGISTER_H, REGISTER_L);

            set_flag(emulator, FLAG_N, 0);
            SET_FLAG_H_ADD16(emulator, hl_byte, sp_byte);
            SET_FLAG_C_ADD16(emulator, hl_byte, sp_byte);

            cyclesSync(emulator);

            break;
        }
        case 0x3A: LOAD_8R_16BRR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); DEC_R1_R2(emulator, REGISTER_H, REGISTER_L); break;
        case 0x3B: emulator->rSP--; cyclesSync(emulator); break;
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
        case 0xBE: compareR_RR(emulator, REGISTER_A, REGISTER_H, REGISTER_L); break;
        case 0xBF: compare_R(emulator, REGISTER_A, REGISTER_A); break;

        case 0xC0: return_condition(emulator, NZ(emulator)); break;
        case 0xC1: POP_RR(emulator, REGISTER_B, REGISTER_C); break;
        case 0xC2: jumpCondition(emulator, NZ(emulator)); break;
        case 0xC3: JUMP_u16(emulator, read2Bytes_8C(emulator)); break;
        case 0xC4: callCondition(emulator, read2Bytes_8C(emulator), NZ(emulator)); break;
        case 0xC5: PUSH_RR(emulator, REGISTER_B, REGISTER_C); break;
        case 0xC6: add_u8(emulator, REGISTER_A); break;
        case 0xC7: RST(emulator, 0x00); break;
        case 0xC8: return_condition(emulator, Z(emulator)); break;
        case 0xC9: ret(emulator); break;
        case 0xCA: jumpCondition(emulator, Z(emulator)); break;
        case 0xCB: prefixCB(emulator); break;
        case 0xCC: callCondition(emulator, read2Bytes_8C(emulator), Z(emulator)); break;
        case 0xCD: call(emulator, read2Bytes_8C(emulator)); break;
        case 0xCE: adc_u8(emulator, REGISTER_A); break;
        case 0xCF: RST(emulator, 0x08); break;
        
        case 0xD0: return_condition(emulator, NC(emulator)); break;
        case 0xD1: POP_RR(emulator, REGISTER_D, REGISTER_E); break;
        case 0xD2: jumpCondition(emulator, NC(emulator)); break;
        case 0xD4: callCondition(emulator, read2Bytes_8C(emulator), NC(emulator)); break;
        case 0xD5: PUSH_RR(emulator, REGISTER_D, REGISTER_E); break;
        case 0xD6: sub_u8(emulator, REGISTER_A); break;
        case 0xD7: RST(emulator, 0x10); break;
        case 0xD8: return_condition(emulator, C(emulator)); break;
        case 0xD9: ret(emulator); break;
        case 0xDA: jumpCondition(emulator, C(emulator)); break;
        case 0xDC: callCondition(emulator, read2Bytes_8C(emulator), C(emulator)); break;
        case 0xDE: sbc_u8(emulator, REGISTER_A); break;
        case 0xDF: RST(emulator, 0x18); break;
        
        case 0xE0: write_address_4C(emulator, 0xFF00 + readByte4C(emulator), get_reg_byte(emulator, REGISTER_A)); break;
        case 0xE1: POP_RR(emulator, REGISTER_H, REGISTER_L); break;
        case 0xE2: write_address_4C(emulator, 0xFF00 + get_reg_byte(emulator, REGISTER_C), get_reg_byte(emulator, REGISTER_A)); break;
        case 0xE5: PUSH_RR(emulator, REGISTER_H, REGISTER_L); break;
        case 0xE6: and_u8(emulator, REGISTER_A); break;
        case 0xE7: RST(emulator, 0x20); break;
        case 0xE8: {
            uint16_t old = emulator->rSP;
            int8_t byte = (int8_t)readByte4C(emulator);
            uint16_t result = old + byte;

            emulator->rSP = result;

            cyclesSync(emulator);
            cyclesSync(emulator);

            set_flag(emulator, FLAG_Z, 0);
            set_flag(emulator, FLAG_N, 0);

            old &= 0xFF;

            SET_FLAG_H_ADD(emulator, old, (uint8_t)byte);
            SET_FLAG_C_ADD(emulator, old, (uint8_t)byte);

            break;
        }
        case 0xE9: JUMP_RR(emulator, REGISTER_H, REGISTER_L); break;
        case 0xEA: write_address_4C(emulator, read2Bytes(emulator), get_reg_byte(emulator, REGISTER_A)); cyclesSync(emulator); cyclesSync(emulator); break;
        case 0xEE: xor_u8(emulator, REGISTER_A); break;
        case 0xEF: RST(emulator, 0x28); break;
        
        case 0xF0: {
            // vm->GPR[R] = readAddr_4C(vm, PORT_ADDR + readByte_4C(vm))
            write_to_reg(emulator, REGISTER_A, read_address_4C(emulator, 0xFF00 + readByte4C(emulator)));
            break;
        }
        case 0xF1: {
            POP_RR(emulator, REGISTER_A, REGISTER_F);
            
            write_to_reg(emulator, REGISTER_F, get_reg_byte(emulator, REGISTER_F) & 0xF0);
            break;
        }
        case 0xF2: write_to_reg(emulator, REGISTER_A, read_address_4C(emulator, 0xFF00 + get_reg_byte(emulator, REGISTER_C))); break;
        case 0xF3: IMD(emulator); break;
        case 0xF5: PUSH_RR(emulator, REGISTER_A, REGISTER_F); break;
        case 0xF6: or_u8(emulator, REGISTER_A); break;
        case 0xF7: RST(emulator, 0x30); break;
        case 0xF8: {
            /* This uses 8 cycles, readbyte4C + cyclesSync */
            uint16_t old = emulator->rSP;
            int8_t byte = (int8_t) readByte4C(emulator);
            uint16_t final = set_reg16(emulator, old + byte, REGISTER_H, REGISTER_L);

            set_flag(emulator, FLAG_Z, 0);
            set_flag(emulator, FLAG_N, 0);

            old &= 0xFF;

            SET_FLAG_H_ADD(emulator, old, (uint8_t)byte);
            SET_FLAG_C_ADD(emulator, old, (uint8_t)byte);

            cyclesSync(emulator);

            break;
        }
        case 0xF9: emulator->rSP = get_reg16(emulator, REGISTER_H, REGISTER_L); break;
        case 0xFA: write_to_reg(emulator, REGISTER_A, read_address_4C(emulator, read2Bytes_8C(emulator))); break;
        case 0xFB: IME(emulator); break;
        case 0xFE: compare_u8(emulator, REGISTER_A); break;
        case 0xFF: RST(emulator, 0x38); break;

        default:
            //printf("\tNot implemented this opcode !!!\n\n");
            // wrong = true;
            return;
    }
    //printf("\t");
    

    //printf("\t");
    //printflags(emulator);
    
skip:
    handleInterrupts(emulator);
}
