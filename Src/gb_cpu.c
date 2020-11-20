#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_opcodes.h"
#include "gb_cpu.h"

uint8_t gb_cpu_reg[GB_REG_SIZE];

// cpu modes and flags
enum cpu_power power_mode;
//bool interrupts_enabled = false;
uint8_t current_opcode_time = 0;

void cpu_set_power_mode(enum cpu_power mode ){
    power_mode = mode;
}

void gb_cpu(){
    uint16_t program_counter = get_reg_16_value(GB_REG_PC);
    uint8_t opcode = gb_mem_map[program_counter];




    //check interrupts
    /*if(interrupts_enabled){
        if(){

        } else if(){

        } else if(){
            
        } else if(){
            
        }

    }*/

    //if no interrupts then process normally
    //process_opcode(opcode);
}


// https://izik1.github.io/gbops/
/*
void process_opcode(){
    //current_opcode_time =

    uint8_t opcode = gb_mem_map[gb_cpu_reg_pc];
    uint8_t operands[2]
    operands[0] = gb_mem_map[gb_cpu_reg_pc + 1];
    operands[1] = gb_mem_map[gb_cpu_reg_pc + 2];

    //check x
    switch(GET_OPCODE_X(opcode)){
        switch 0x00:
            switch(GET_OPCODE_Z(opcode)){
                case 0x00: // relative jumps and others
                    break;
                case 0x01: // 16 bit load immediate and adds
                    if(GET_OPCODE_Q(opcode) == 0x01){
                        //16 bit load
                        opcode_16_ld(get_reg_16_sp(GET_OPCODE_P(opcode)),
                                        &operands[0]);
                    } else {
                        //16 bit add hl
                        opcode_16_add_hl(get_reg_16_sp(GET_OPCODE_P(opcode)));
                    }                    
                    break;
                case 0x02: // 8 bit indirect loading
                    if(GET_OPCODE_Q(opcode) == 0x01){
                        switch(GET_OPCODE_P(opcode))){
                            case 0x00:
                                opcode_8_ld(gb_mem_map[get_reg_16_value(GB_REG_BC)],
                                            &gb_cpu_reg[GB_REG_A]);
                                break;
                            case 0x01:
                                opcode_8_ld(gb_mem_map[get_reg_16_value(GB_REG_DE)],
                                            &gb_cpu_reg[GB_REG_A]);
                                break;
                            case 0x02:
                                opcode_8_ldi(gb_mem_map[get_reg_16_value(GB_REG_HL)],
                                            &gb_cpu_reg[GB_REG_A]);
                                break;
                            case 0x03:
                                opcode_8_ldd(gb_mem_map[get_reg_16_value(GB_REG_HL)],
                                            &gb_cpu_reg[GB_REG_A]);
                                break;
                        }
                    } else {
                        //16 bit add hl
                        opcode_16_add_hl(get_reg_16_sp(GET_OPCODE_P(opcode)));
                    }       
                    break;
                case 0x03: // 16 bit inc/dec
                    if(GET_OPCODE_Q(opcode) == 0x01){
                        //16 bit inc
                        opcode_16_inc(get_reg_16_sp(GET_OPCODE_P(opcode)));
                    } else {
                        //16 bit dec
                        opcode_16_dec(get_reg_16_sp(GET_OPCODE_P(opcode)));
                    }       
                    break;
                case 0x04: // 8 bit inc
                    opcode_8_inc(get_reg_8(GET_OPCODE_Y(opcode)));
                    break;
                case 0x05: // 8 bit dec
                    opcode_8_dec(get_reg_8(GET_OPCODE_Y(opcode)));
                    break;
                case 0x06: // 8bit load immediate
                    opcode_8_ld(get_reg_8(GET_OPCODE_Y(opcode)),
                                &operands[0]);
                    break;
                case 0x07: //assoted ops on accumulater flags
                    break;
            }
            break;
            break;
        switch 0x01: //LD and HALT------------------------------
            if(opcode == OPCODE_HALT){ 
                // just regular old halt
                opcode_halt();
            } else {
                opcode_8_ld(get_reg_8(GET_OPCODE_Y(opcode)),
                            get_reg_8(GET_OPCODE_Z(opcode)));
            }
            break;
        switch 0x02: //ALU--------------------------------------
            uint8_t* reg = get_reg_8(GET_OPCODE_Z(opcode));

            switch(GET_OPCODE_Y(opcode)){
                case 0x00: // ADD A
                    opcode_8_add(reg);
                    break;
                case 0x01: // ADC A
                    opcode_8_adc(reg);
                    break;
                case 0x02: // SUB
                    opcode_8_sub(reg);
                    break;
                case 0x03: // SUB A
                    opcode_8_sbc(reg);
                    break;
                case 0x04: // AND
                    opcode_8_and(reg);
                    break;
                case 0x05: // XOR
                    opcode_8_xor(reg);
                    break;
                case 0x06: // OR
                    opcode_8_or(reg);
                    break;
                case 0x07: // CP
                    opcode_8_cp(reg);
                    break;
            }
            break;
        switch 0x03:
            break;
        default:

    }
}*/