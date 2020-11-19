#include "gb_opcodes.h"
#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_cpu.h"


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
    uint8_t opcode = gb_mem_map[opcode_address]];
    
    //check unique opcodes
    switch(opcode){
        case 0x36: // ld (HL),n
            gb_mem_map[get_16_from_8(GB_REG_HL)] = gb_mem_map[opcode_address+1];
            break;
        case 0x0A: // ld A,(BC)
            gb_cpu_reg[GB_REG_A] = gb_mem_map[get_16_from_8(GB_REG_BC)];
            break;
        case 0x1A: // ld A,(DE)
            gb_cpu_reg[GB_REG_A] = gb_mem_map[get_16_from_8(GB_REG_DE)];
            break;
        case 0xFA: // ld A,(nn)
            gb_cpu_reg[GB_REG_A] = gb_mem_map[get_16_from_8(opcode_address + 1)];
            break;
        case 0x02: // ld (BC),A
            gb_mem_map[get_16_from_8(GB_REG_BC)] = gb_cpu_reg[GB_REG_A];
            break;
        case 0x12: // ld (DE),A
            gb_mem_map[get_16_from_8(GB_REG_DE)] = gb_cpu_reg[GB_REG_A];
            break;
        case 0xEA: // ld (nn),A
            gb_mem_map[get_16_from_8(opcode_address + 1)] = gb_cpu_reg[GB_REG_A];
            break;
        case 0xF0: // ld A,(FF00+n)
            gb_cpu_reg[GB_REG_A] = gb_mem_map[IO_PORTS + gb_mem_map[opcode_address + 1]];
            break;
        case 0xE0: // ld (FF00+n),A
            gb_mem_map[IO_PORTS + gb_mem_map[opcode_address + 1]] = gb_cpu_reg[GB_REG_A];
            break;
        case 0xF2: // ld A,(FF00+C)
            gb_cpu_reg[GB_REG_A] = gb_mem_map[IO_PORTS + gb_cpu_reg[GB_REG_C]];
            break;
        case 0xE2: // ld (FF00+C),A
            gb_mem_map[IO_PORTS + gb_cpu_reg[GB_REG_C]] = gb_cpu_reg[GB_REG_A];
            break;
        default:
            uint8_t opcode_x = GET_OPCODE_Z(opcode);
            uint8_t opcode_y = GET_OPCODE_Y(opcode);
            uint8_t opcode_z = GET_OPCODE_Z(opcode);

            if(opcode_z == 0x06 && opcode_x == 0x00){ // check immediate loads
                if(opcode_y == 0x06){
                    gb_mem_map[get_reg_16_value(GB_REG_HL)] = gb_mem_map[opcode_address + 1]];
                } else {
                    gb_cpu_reg[opcode_y] = gb_mem_map[opcode_address + 1]];
                }
            } else if(opcode_x == 0x01){ // check register loads
                gb_cpu_reg[opcode_y] = gb_cpu_reg[opcode_z];
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
    uint8_t opcode = gb_mem_map[opcode_address]];
    uint16_t hl_value = get_16_from_8(&gb_cpu_reg[GB_REG_HL]);

    if(opcode == 0x22){
        gb_mem_map[hl_value] = gb_cpu_reg[GB_REG_A];
    } else {
        gb_cpu_reg[GB_REG_A] = gb_mem_map[hl_value];
    }
    
    hl_value++;

    set_16_from_8(&gb_cpu_reg[GB_REG_HL], hl_value);
    return opcode_table[opcode].cycles;
}


/*
ldd  (HL),A      32         8 ---- (HL)=A, HL=HL-1
ldd  A,(HL)      3A         8 ---- A=(HL), HL=HL-1
*/
uint8_t opcode_8_ldd(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address]];
    uint16_t hl_value = get_16_from_8(&gb_cpu_reg[GB_REG_HL]);

    if(gb_mem_map[opcode_address] == 0x32){
        gb_mem_map[hl_value] = gb_cpu_reg[GB_REG_A];
    } else {
        gb_cpu_reg[GB_REG_A] = gb_mem_map[hl_value];
    }
    
    hl_value--;

    set_16_from_8(&gb_cpu_reg[GB_REG_HL], hl_value);

    return opcode_table[opcode].cycles;
}
