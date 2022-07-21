#include "gb_opcodes.h"
#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_cpu.h"
#include "debug_print.h"


//******************************************************
//8-bit load/store/move commands
//******************************************************
/*
ld   r,r         xx         4 ---- r=r
ld   r,n         xx nn      8 ---- r=n
ld   r,(HL)      xx         8 ---- r=(HL)
ld   (HL),r      7x         8 ---- (HL)=r
ld   (HL),n      36 nn     12 ----
ld   A,(BC)      0A         8 ----
ld   A,(DE)      1A         8 ----
ld   A,(nn)      FA nn nn  16 ----
ld   (BC),A      02         8 ----
ld   (DE),A      12         8 ----
ld   (nn),A      EA        16 ----
ld   A,(FF00+n)  F0 nn     12 ---- read from io-port n (memory FF00+n)
ld   (FF00+n),A  E0 nn     12 ---- write to io-port n (memory FF00+n)
ld   A,(FF00+C)  F2         8 ---- read from io-port C (memory FF00+C)
ld   (FF00+C),A  E2         8 ---- write to io-port C (memory FF00+C)
*/
uint8_t opcode_8_ld(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    uint8_t opcode_x = GET_OPCODE_X(opcode);
    uint8_t opcode_y = GET_OPCODE_Y(opcode);
    uint8_t opcode_z = GET_OPCODE_Z(opcode);
    
    //check unique opcodes
    switch(opcode){
        case 0x36: // ld (HL),n
            set_mem_map_8(CPU_REG.HL, get_mem_map_8(opcode_address+1));
            break;
        case 0x0A: // ld A,(BC)
            CPU_REG.A = get_mem_map_8(CPU_REG.BC);
            break;
        case 0x1A: // ld A,(DE)
            CPU_REG.A = get_mem_map_8(CPU_REG.DE);
            break;
        case 0xFA: // ld A,(nn)
            CPU_REG.A = get_mem_map_8(get_mem_map_16(opcode_address + 1));
            break;
        case 0x02: // ld (BC),A
            set_mem_map_8(CPU_REG.BC, CPU_REG.A);
            break;
        case 0x12: // ld (DE),A
            set_mem_map_8(CPU_REG.DE, CPU_REG.A);
            break;
        case 0xEA: // ld (nn),A
            set_mem_map_8(get_mem_map_16(opcode_address + 1), CPU_REG.A);
            break;
        case 0xF0: // ld A,(FF00+n)
            CPU_REG.A = get_mem_map_8(IO_PORTS + get_mem_map_8(opcode_address + 1));
            break;
        case 0xE0: // ld (FF00+n),A
            set_mem_map_8(IO_PORTS + get_mem_map_8(opcode_address + 1), CPU_REG.A);
            break;
        case 0xF2: // ld A,(FF00+C)
            CPU_REG.A = get_mem_map_8(IO_PORTS + CPU_REG.C);
            break;
        case 0xE2: // ld (FF00+C),A
            set_mem_map_8(IO_PORTS + CPU_REG.C, CPU_REG.A);
            break;
        default:
            if(opcode_z == 0x06 && opcode_x == 0x00){ // check immediate loads
                if(opcode_y == 0x06){
                    set_mem_map_8(CPU_REG.HL, get_mem_map_8(opcode_address + 1));
                } else {
                    uint8_t* reg = get_reg_8(opcode_y);
                    *reg = get_mem_map_8(opcode_address + 1);
                }
            } else if(opcode_x == 0x01){ // check register loads
                if(opcode_y == 0x06){
                    if(opcode_z == 0x06){
                        //should never get here
                        PRINT("Error should be halt command!!!");
                    } else {
                        set_mem_map_8(CPU_REG.HL,*get_reg_8(opcode_z));
                    }
                } else {
                    if(opcode_z == 0x06){
                        uint8_t* reg = get_reg_8(opcode_y);
                        *reg = get_mem_map_8(CPU_REG.HL);
                    } else {
                        uint8_t* reg = get_reg_8(opcode_y);
                        *reg = *get_reg_8(opcode_z);
                    }
                }
            }

            break;
    }
    return opcode_table[opcode].cycles;
}


/*
ldi  (HL),A      22         8 ---- (HL)=A, HL=HL+1
ldi  A,(HL)      2A         8 ---- A=(HL), HL=HL+1
*/
uint8_t opcode_8_ldi(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);

    if(opcode == 0x22){
        set_mem_map_8(CPU_REG.HL, CPU_REG.A);
    } else {
        CPU_REG.A = get_mem_map_8(CPU_REG.HL);
    }
    
    CPU_REG.HL++;
    return opcode_table[opcode].cycles;
}


/*
ldd  (HL),A      32         8 ---- (HL)=A, HL=HL-1
ldd  A,(HL)      3A         8 ---- A=(HL), HL=HL-1
*/
uint8_t opcode_8_ldd(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);

    if(opcode == 0x32){
        set_mem_map_8(CPU_REG.HL, CPU_REG.A);
    } else {
        CPU_REG.A = get_mem_map_8(CPU_REG.HL);
    }
    
    CPU_REG.HL--;
    return opcode_table[opcode].cycles;
}
