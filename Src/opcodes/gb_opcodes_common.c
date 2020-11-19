
int check_16_overflow(uint16_t* a, uint16_t* b){
    return (*a > 0xFFFF - *b);
}

int check_16_underflow(uint16_t* a, uint16_t* b){
    return (*a < *b);
}

int check_8_overflow(uint8_t* b){
    return (gb_cpu_reg[GB_REG_A] > 0xFF - *b);
}

int check_8_underflow(uint8_t* b){
    return (gb_cpu_reg[GB_REG_A] < *b);
}

int check_4_overflow(uint8_t* b){
    return (((gb_cpu_reg[GB_REG_A] & 0x0F) + (*b & 0x0F)) & 0x10) == 0x10;
}

int check_4_underflow(uint8_t* b){
    return ((gb_cpu_reg[GB_REG_A] & 0xF0) < (*b & 0xF0));
}

void check_zero_flag(){
    if (gb_cpu_reg[GB_REG_A] == 0x00){
        SET_ZERO_FLAG;
    } else {
        CLR_ZERO_FLAG;
    }
}


// gb_cpu_reg functions
uint8_t* get_reg_8(uint8_t reg){
    uint8_t* ret = NULL
    if(reg < GB_REG_SIZE){
        if(reg == 0x06){
            ret = &gb_mem_map[get_reg_16_value(GB_REG_HL)];
        } else {
            ret = &gb_cpu_reg[reg];
        }
    }
    return ret;
}

uint8_t* get_reg_16_sp(uint8_t reg){
    uint8_t* ret = 0;
    switch(reg){
        case 0:
            ret = &gb_cpu_reg[GB_REG_BC];
            break;
        case 1:
            ret = &gb_cpu_reg[GB_REG_DE];
            break;
        case 2:
            ret = &gb_cpu_reg[GB_REG_HL];
            break;
        case 3:
            ret = &gb_cpu_reg[GB_REG_SP];
            break;
    }
    return ret;
}

uint8_t* get_reg_16_af(uint8_t reg){
    uint8_t* ret = 0;
    switch(reg){
        case 0:
            ret = &gb_cpu_reg[GB_REG_BC];
            break;
        case 1:
            ret = &gb_cpu_reg[GB_REG_DE];
            break;
        case 2:
            ret = &gb_cpu_reg[GB_REG_HL];
            break;
        case 3:
            ret = &gb_cpu_reg[GB_REG_AF];
            break;
    }
    return ret;
}

bool get_flag_condition(uint8_t flag){
    bool ret = false;
    switch(reg){
        case 0: // NZ
            if(GET_ZERO_FLAG == 0){
                ret = true;
            }
            break;
        case 1: // Z
            if(GET_ZERO_FLAG == 1){
                ret = true;
            }
            break;
        case 2: // NC
            if(GET_CARRY_FLAG == 0){
                ret = true;
            }
            break;
        case 3: // C
            if(GET_CARRY_FLAG == 1){
                ret = true;
            }
            break;
    }
    return ret;
}

uint16_t get_reg_16_value(uint8_t reg){
    if(reg+1 < GB_REG_SIZE){
        return get_16_from_8(&gb_cpu_reg[reg]);
    }
    return 0;
}