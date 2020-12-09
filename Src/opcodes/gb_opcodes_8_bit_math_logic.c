#include "gb_opcodes.h"
#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_cpu.h"

//******************************************************
//8-bit math/logic commands
//******************************************************

/*
add  A,r         8x         4 z0hc A=A+r
add  A,n         C6 nn      8 z0hc A=A+n
add  A,(HL)      86         8 z0hc A=A+(HL)
*/
uint8_t opcode_8_add(uint16_t opcode_address){
    uint8_t value = 0;
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_z = GET_OPCODE_Z(opcode);

    // get value depending on opcode
    switch(opcode){
        case 0xC6:
            value = gb_mem_map[opcode_address + 1];
            break;
        default:
            value = *get_reg_8(opcode_z);
            break;
    }
        if(CPU_REG.PC > 0xF3){          
        printf("value %02x\n",value);
    }   
    //check carry flags first
    if (check_8_overflow(&value)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_overflow(&value)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }
    if(CPU_REG.PC > 0xF3){          
        printf("value %02x\n",value);
    }    
    //do opperartion
    CPU_REG.A += value;

    //check other flags
    CLR_ADD_SUB_FLAG;
    
    check_zero_flag(CPU_REG.A);
    return opcode_table[opcode].cycles;
}

/*
adc  A,r         8x         4 z0hc A=A+r+cy
adc  A,n         CE nn      8 z0hc A=A+n+cy
adc  A,(HL)      8E         8 z0hc A=A+(HL)+cy
*/
uint8_t opcode_8_adc(uint16_t opcode_address){
    uint8_t value = 0;
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_z = GET_OPCODE_Z(opcode);

    // get value depending on opcode
    switch(opcode){
        case 0xCE:
            value = gb_mem_map[opcode_address + 1];
            break;
        default:
            value = *get_reg_8(opcode_z);
            break;
    }

    //check carry flags first
    if (check_8_overflow(&value)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_overflow(&value)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    CPU_REG.A += value;
    CPU_REG.A += GET_CARRY_FLAG;

    //check other flags
    CLR_ADD_SUB_FLAG;
    
    check_zero_flag(CPU_REG.A);
    return opcode_table[opcode].cycles;
}

/*
sub  r           9x         4 z1hc A=A-r
sub  n           D6 nn      8 z1hc A=A-n
sub  (HL)        96         8 z1hc A=A-(HL)
*/
uint8_t opcode_8_sub(uint16_t opcode_address){
    uint8_t value = 0;
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_z = GET_OPCODE_Z(opcode);

    // get value depending on opcode
    switch(opcode){
        case 0xD6:
            value = gb_mem_map[opcode_address + 1];
            break;
        default:
            value = *get_reg_8(opcode_z);
            break;
    }

    //check carry flags first
    if (check_8_underflow(&value)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_underflow(&value)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    CPU_REG.A -= value;

    //check other flags
    SET_ADD_SUB_FLAG;
    
    check_zero_flag(CPU_REG.A);
    return opcode_table[opcode].cycles;
}

/*
sbc  A,r         9x         4 z1hc A=A-r-cy
sbc  A,n         DE nn      8 z1hc A=A-n-cy
sbc  A,(HL)      9E         8 z1hc A=A-(HL)-cy
*/
uint8_t opcode_8_sbc(uint16_t opcode_address){
    uint8_t value = 0;
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_z = GET_OPCODE_Z(opcode);
    
    // get value depending on opcode
    switch(opcode){
        case 0xDE:
            value = gb_mem_map[opcode_address + 1];
            break;
        default:
            value = *get_reg_8(opcode_z);
            break;
    }

    //check carry flags first
    if (check_8_underflow(&value)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_underflow(&value)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    CPU_REG.A -= value;
    CPU_REG.A -= GET_CARRY_FLAG;

    //check other flags
    SET_ADD_SUB_FLAG;
    
    check_zero_flag(CPU_REG.A);
    return opcode_table[opcode].cycles;
}

/*
and  r           Ax         4 z010 A=A & r
and  n           E6 nn      8 z010 A=A & n
and  (HL)        A6         8 z010 A=A & (HL)
*/
uint8_t opcode_8_and(uint16_t opcode_address){
    uint8_t value = 0;
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_z = GET_OPCODE_Z(opcode);
    
    // get value depending on opcode
    switch(opcode){
        case 0xE6:
            value = gb_mem_map[opcode_address + 1];
            break;
        default:
            value = *get_reg_8(opcode_z);
            break;
    }

    CPU_REG.A &= value;

    //check other flags
    CLR_CARRY_FLAG;
    SET_HALF_CARRY_FLAG;
    CLR_ADD_SUB_FLAG;

    check_zero_flag(CPU_REG.A);
    return opcode_table[opcode].cycles;
}

/*
xor  r           Ax         4 z000
xor  n           EE nn      8 z000
xor  (HL)        AE         8 z000
*/ 
uint8_t opcode_8_xor(uint16_t opcode_address){
    uint8_t value = 0;
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_z = GET_OPCODE_Z(opcode);
    
    // get value depending on opcode
    switch(opcode){
        case 0xEE:
            value = gb_mem_map[opcode_address + 1];
            break;
        default:
            value = *get_reg_8(opcode_z);
            break;
    }

    CPU_REG.A ^= value;

    //check other flags
    CLR_CARRY_FLAG;
    CLR_HALF_CARRY_FLAG;
    CLR_ADD_SUB_FLAG;
    
    check_zero_flag(CPU_REG.A);
    return opcode_table[opcode].cycles;
}

/*
or   r           Bx         4 z000 A=A | r
or   n           F6 nn      8 z000 A=A | n
or   (HL)        B6         8 z000 A=A | (HL)
*/
uint8_t opcode_8_or(uint16_t opcode_address){
    uint8_t value = 0;
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_z = GET_OPCODE_Z(opcode);

    // get value depending on opcode
    switch(opcode){
        case 0xF6:
            value = gb_mem_map[opcode_address + 1];
            break;
        default:
            value = *get_reg_8(opcode_z);
            break;
    }

    CPU_REG.A |= value;

    //check other flags
    CLR_CARRY_FLAG;
    CLR_HALF_CARRY_FLAG;
    CLR_ADD_SUB_FLAG;
    
    check_zero_flag(CPU_REG.A);
    return opcode_table[opcode].cycles;
}

/*
cp   r           Bx         4 z1hc compare A-r
cp   n           FE nn      8 z1hc compare A-n
cp   (HL)        BE         8 z1hc compare A-(HL)
*/
uint8_t opcode_8_cp(uint16_t opcode_address){
    uint8_t value = 0;
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_z = GET_OPCODE_Z(opcode);
    
    // get value depending on opcode
    switch(opcode){
        case 0xFE:
            value = gb_mem_map[opcode_address + 1];
            break;
        default:
            value = *get_reg_8(opcode_z);
            break;
    }

    uint8_t temp = CPU_REG.A;

    if (check_8_underflow(&value)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_underflow(&value)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    CPU_REG.A -= value;

    SET_ADD_SUB_FLAG;
    check_zero_flag(CPU_REG.A);

    CPU_REG.A = temp;
    return opcode_table[opcode].cycles;
}

/*
inc  r           xx         4 z0h- r=r+1
inc  (HL)        34        12 z0h- (HL)=(HL)+1
*/
uint8_t opcode_8_inc(uint16_t opcode_address){
    uint8_t* addr = NULL;
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_y = GET_OPCODE_Y(opcode);
    
    // get value depending on opcode
    addr = get_reg_8(opcode_y);

    //check carry flags first
    if(check_4_overflow(addr)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    //CPU_REG.A = 
    ++*addr;

    CLR_ADD_SUB_FLAG;
    
    check_zero_flag(*addr);
    return opcode_table[opcode].cycles;
}

/*
dec  r           xx         4 z1h- r=r-1
dec  (HL)        35        12 z1h- (HL)=(HL)-1
*/
uint8_t opcode_8_dec(uint16_t opcode_address){
    uint8_t* addr = NULL;
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_y = GET_OPCODE_Y(opcode);
    
    // get value depending on opcode
    addr = get_reg_8(opcode_y);

    //check carry flags first
    if(check_4_underflow(addr)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    //CPU_REG.A = 
    --*addr;

    SET_ADD_SUB_FLAG;
    
    check_zero_flag(*addr);
    return opcode_table[opcode].cycles;
}

/*
daa              27         4 z-0x decimal adjust akku
*/
uint8_t opcode_8_daa(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t low = GET_LOW_NIBBLE(CPU_REG.A);
    uint8_t high = GET_HIGH_NIBBLE(CPU_REG.A);
    bool cf = false;
    bool hf = false;
    bool nf = false;

    if(GET_ADD_SUB_FLAG == 1)
        nf = true; 
    
    if(GET_HALF_CARRY_FLAG == 1)
        hf = true;

    if(GET_CARRY_FLAG == 1)
        cf = true;

    if(!cf && !hf && high <= 8 && low >= 10){
        CPU_REG.A += 0x06;
        CLR_CARRY_FLAG;
    } else if(!cf && hf && high <= 9 && low <= 3){
        CPU_REG.A += 0x06;
        CLR_CARRY_FLAG;
    } else if(!nf && !cf && !hf && high >= 10 && low <= 9){
        CPU_REG.A += 0x60;
        SET_CARRY_FLAG;
    } else if(!nf && !cf && !hf && high >= 9 && low >= 10){
        CPU_REG.A += 0x66;
        SET_CARRY_FLAG;
    } else if(!nf && !cf && hf && high >= 10 && low <= 3){
        CPU_REG.A += 0x66;
        SET_CARRY_FLAG;
    } else if(cf && !hf && high <= 2 && low <= 9){
        CPU_REG.A += 0x60;
        SET_CARRY_FLAG;
    } else if(cf && !hf && high <= 2 && low >= 10){
        CPU_REG.A += 0x66;
        SET_CARRY_FLAG;
    } else if(cf && hf && high <= 3 && low <= 3){
        CPU_REG.A += 0x66;
        SET_CARRY_FLAG;
    } else if(nf && !cf && hf && high <= 8 && low >= 6){
        CPU_REG.A += 0xFA;
        CLR_CARRY_FLAG;
    } else if(nf && cf && !hf && high >= 7 && low <= 9){
        CPU_REG.A += 0xA0;
        SET_CARRY_FLAG;
    } else if(nf && cf && hf && (high == 7 || high == 6) && low >= 6){
        CPU_REG.A += 0x9A;
        SET_CARRY_FLAG;
    }

    CLR_HALF_CARRY_FLAG;
    check_zero_flag(CPU_REG.A);
    return opcode_table[opcode].cycles;
}

/*
cpl              2F         4 -11- A = A xor FF
*/
uint8_t opcode_8_cpl(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    
    CPU_REG.A ^= 0xFF;
    SET_ADD_SUB_FLAG;
    SET_HALF_CARRY_FLAG;
    return opcode_table[opcode].cycles;
}

