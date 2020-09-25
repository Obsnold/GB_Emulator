#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_cpu.h"

#define OP_X_MASK 0xC0
#define OP_Y_MASK 0x38
#define OP_Z_MASK 0x07
#define OP_P_MASK 0x30
#define OP_Q_MASK 0x08

#define OP_X_0  0x00
#define OP_X_1  0x40
#define OP_X_2  0x80
#define OP_X_3  0xC0

#define OP_Y_0  0x00
#define OP_Y_1  0x08
#define OP_Y_2  0x10
#define OP_Y_3  0x18
#define OP_Y_4  0x20
#define OP_Y_5  0x28
#define OP_Y_6  0x30
#define OP_Y_7  0x38

#define OP_Z_0  0x00
#define OP_Z_1  0x01
#define OP_Z_2  0x02
#define OP_Z_3  0x03
#define OP_Z_4  0x04
#define OP_Z_5  0x05
#define OP_Z_6  0x06
#define OP_Z_7  0x07

#define OP_P_0  0x00
#define OP_P_1  0x10
#define OP_P_2  0x20
#define OP_P_3  0x30

#define OP_Q_0  0x00
#define OP_Q_1  0x08


#define GET_OPCODE_X(x) ((x && OP_X_MASK) >> 6)
#define GET_OPCODE_Y(x) ((x && OP_Y_MASK) >> 3)
#define GET_OPCODE_Z(x) (x && OP_Z_MASK)
#define GET_OPCODE_P(x) ((x && OP_P_MASK) >> 4)
#define GET_OPCODE_Q(x) ((x && OP_Q_MASK) >> 3)


//unique opcodes
#define OPCODE_CCF      0x3F
#define OPCODE_SCF      0x37
#define OPCODE_NOP      0x00
#define OPCODE_HALT     0x76
#define OPCODE_STOP     0x10
#define OPCODE_DI       0xF3
#define OPCODE_EI       0xFB


// https://izik1.github.io/gbops/

void process_opcode(){
    switch(OPCODE_CCF){
        case 0x00: //HALT
                //byte 1
                //tick 4
                //flags 
                break;
        case 0x01: //LD BC, u16
                //byte 3
                //tick 12
                //flags 
                break;
        case 0x02: //LD (BC), A
                //byte 1
                //tick 8
                //flags 
                break;
        case 0x03: //INC BC
                //byte 1
                //tick 8
                //flags 
                break;
        case 0x04: //INC B
                //byte 1
                //tick 4
                //flags 
                break;
        case 0x05: //DEC B
                //byte 1
                //tick 4
                //flags 
                break;
        case 0x06: //LD B, u8
                //byte 2
                //tick 8
                //flags 
                break;
        case 0x07: //RLCA
                //byte 1
                //tick 4
                //flags 
                break;
        case 0x08: //LD (u16),SP
                //byte 3
                //tick 20
                //flags 
                break;
        case 0x09: //ADD HL,BC
                //byte 1
                //tick 8
                //flags 
                break;
        case 0x0A: //LC A, (BC)
                //byte 1
                //tick 8
                //flags 
                break;
        case 0x0B: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x0C: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x0D: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x0E: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x0F: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x10: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x11: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x12: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x13: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x14: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x15: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x16: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x17: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x18: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x19: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x1A: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x1B: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x1C: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x1D: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x1E: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x1F: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x20: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x21: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x22: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x23: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x24: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x25: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x26: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x27: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x28: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x29: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x2A: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x2B: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x2C: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x2D: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x2E: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x2F: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x30: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x31: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x32: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x33: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x34: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x35: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x36: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x37: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x38: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x39: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x3A: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x3B: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x3C: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x3D: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x3E: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x3F: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x40: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x41: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x42: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x43: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x44: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x45: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x46: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x47: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x48: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x49: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x4A: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x4B: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x4C: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x4D: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x4E: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x4F: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x50: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x51: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x52: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x53: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x54: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x55: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x56: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x57: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x58: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x59: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x5A: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x5B: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x5C: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x5D: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x5E: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x5F: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x60: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x61: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x62: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x63: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x64: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x65: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x66: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x67: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x68: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x69: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x6A: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x6B: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x6C: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x6D: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x6E: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x6F: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x70: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x71: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x72: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x73: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x74: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x75: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x76: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x77: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x78: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x79: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x7A: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x7B: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x7C: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x7D: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x7E: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x7F: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x80: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x81: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x82: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x83: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x84: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x85: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x86: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x87: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x88: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x89: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x8A: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x8B: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x8C: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x8D: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x8E: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x8F: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x90: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x91: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x92: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x93: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x94: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x95: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x96: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x97: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x98: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x99: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x9A: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x9B: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x9C: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x9D: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x9E: //
                //byte 
                //tick 
                //flags 
                break;
        case 0x9F: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xA0: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xA1: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xA2: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xA3: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xA4: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xA5: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xA6: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xA7: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xA8: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xA9: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xAA: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xAB: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xAC: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xAD: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xAE: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xAF: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xB0: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xB1: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xB2: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xB3: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xB4: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xB5: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xB6: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xB7: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xB8: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xB9: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xBA: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xBB: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xBC: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xBD: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xBE: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xBF: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xC0: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xC1: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xC2: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xC3: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xC4: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xC5: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xC6: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xC7: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xC8: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xC9: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xCA: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xCB: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xCC: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xCD: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xCE: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xCF: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xD0: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xD1: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xD2: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xD3: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xD4: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xD5: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xD6: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xD7: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xD8: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xD9: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xDA: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xDB: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xDC: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xDD: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xDE: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xDF: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xE0: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xE1: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xE2: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xE3: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xE4: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xE5: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xE6: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xE7: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xE8: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xE9: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xEA: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xEB: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xEC: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xED: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xEE: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xEF: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xF0: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xF1: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xF2: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xF3: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xF4: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xF5: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xF6: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xF7: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xF8: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xF9: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xFA: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xFB: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xFC: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xFD: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xFE: //
                //byte 
                //tick 
                //flags 
                break;
        case 0xFF: //
                //byte 
                //tick 
                //flags 
                break;

    }
}