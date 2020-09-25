#include "gb_opcodes.h"
#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_cpu.h"

int check_16_overflow(uint16_t* a, uint16_t* b){
    return (*a > 0xFFFF - *b);
}

int check_16_underflow(uint16_t* a, uint16_t* b){
    return (*a < *b);
}

int check_8_overflow(uint8_t* b){
    return (gb_reg_map[GB_REG_A] > 0xFF - *b);
}

int check_8_underflow(uint8_t* b){
    return (gb_reg_map[GB_REG_A] < *b);
}

int check_4_overflow(uint8_t* b){
    return (((gb_reg_map[GB_REG_A] & 0x0F) + (*b & 0x0F)) & 0x10) == 0x10;
}

int check_4_underflow(uint8_t* b){
    return ((gb_reg_map[GB_REG_A] & 0xF0) < (*b & 0xF0));
}

void check_zero_flag(){
    if (gb_reg_map[GB_REG_A] == 0x00){
        SET_ZERO_FLAG;
    } else {
        CLR_ZERO_FLAG;
    }
}



//******************************************************
//8-bit commands
//******************************************************
/*
ld   r,r         xx         4 ---- r=r
ld   r,n         xx nn      8 ---- r=n
ld   r,(HL)      xx         8 ---- r=(HL)
ld   (HL),r      7x         8 ---- (HL)=r
ld   (HL),n      36 nn     12 ----
ld   A,(BC)      0A         8 ----
ld   A,(DE)      1A         8 ----
ld   A,(nn)      FA        16 ----
ld   (BC),A      02         8 ----
ld   (DE),A      12         8 ----
ld   (nn),A      EA        16 ----
ld   A,(FF00+n)  F0 nn     12 ---- read from io-port n (memory FF00+n)
ld   (FF00+n),A  E0 nn     12 ---- write to io-port n (memory FF00+n)
ld   A,(FF00+C)  F2         8 ---- read from io-port C (memory FF00+C)
ld   (FF00+C),A  E2         8 ---- write to io-port C (memory FF00+C)
*/
void opcode_8_ld(uint8_t* dest, uint8_t* source){
    *dest = *source;
}


/*
ldi  (HL),A      22         8 ---- (HL)=A, HL=HL+1
ldi  A,(HL)      2A         8 ---- A=(HL), HL=HL+1
*/
void opcode_8_ldi(uint8_t* dest, uint8_t* source){
    *dest = *source;
    *dest = *dest + 1;
}

/*
ldd  (HL),A      32         8 ---- (HL)=A, HL=HL-1
ldd  A,(HL)      3A         8 ---- A=(HL), HL=HL-1
*/
void opcode_8_ldd(uint8_t* dest, uint8_t* source){
    *dest = *source;
    *dest = *dest - 1;
}

/*
add  A,r         8x         4 z0hc A=A+r
add  A,n         C6 nn      8 z0hc A=A+n
add  A,(HL)      86         8 z0hc A=A+(HL)
*/
void opcode_8_add(uint8_t* source){
    //check carry flags first
    if (check_8_overflow(source)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_overflow(source)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    gb_reg_map[GB_REG_A] += *source;

    //check other flags
    CLR_ADD_SUB_FLAG;
    
    check_zero_flag();
}

/*
adc  A,r         8x         4 z0hc A=A+r+cy
adc  A,n         CE nn      8 z0hc A=A+n+cy
adc  A,(HL)      8E         8 z0hc A=A+(HL)+cy
*/
void opcode_8_adc(uint8_t* source){

    //check carry flags first
    if (check_8_overflow(source)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_overflow(source)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    gb_reg_map[GB_REG_A] += *source;
    gb_reg_map[GB_REG_A] += GET_CARRY_FLAG;

    //check other flags
    CLR_ADD_SUB_FLAG;
    
    check_zero_flag();

}

/*
sub  r           9x         4 z1hc A=A-r
sub  n           D6 nn      8 z1hc A=A-n
sub  (HL)        96         8 z1hc A=A-(HL)
*/
void opcode_8_sub(uint8_t* source){
    //check carry flags first
    if (check_8_underflow(source)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_underflow(source)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    gb_reg_map[GB_REG_A] -= *source;

    //check other flags
    SET_ADD_SUB_FLAG;
    
    check_zero_flag();
}

/*
sbc  A,r         9x         4 z1hc A=A-r-cy
sbc  A,n         DE nn      8 z1hc A=A-n-cy
sbc  A,(HL)      9E         8 z1hc A=A-(HL)-cy
*/
void opcode_8_sbc(uint8_t* source){
    //check carry flags first
    if (check_8_underflow(source)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_underflow(source)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    gb_reg_map[GB_REG_A] -= *source;
    gb_reg_map[GB_REG_A] -= GET_CARRY_FLAG;

    //check other flags
    SET_ADD_SUB_FLAG;
    
    check_zero_flag();
}

/*
and  r           Ax         4 z010 A=A & r
and  n           E6 nn      8 z010 A=A & n
and  (HL)        A6         8 z010 A=A & (HL)
*/
void opcode_8_and(uint8_t* source){
    gb_reg_map[GB_REG_A] &= *source;

    //check other flags
    CLR_CARRY_FLAG;
    SET_HALF_CARRY_FLAG;
    CLR_ADD_SUB_FLAG;

    check_zero_flag();
}

/*
xor  r           Ax         4 z000
xor  n           EE nn      8 z000
xor  (HL)        AE         8 z000
*/ 
void opcode_8_xor(uint8_t* source){

    gb_reg_map[GB_REG_A] ^= *source;

    //check other flags
    CLR_CARRY_FLAG;
    CLR_HALF_CARRY_FLAG;
    CLR_ADD_SUB_FLAG;
    
    check_zero_flag();
}

/*
or   r           Bx         4 z000 A=A | r
or   n           F6 nn      8 z000 A=A | n
or   (HL)        B6         8 z000 A=A | (HL)
*/
void opcode_8_or(uint8_t* source){

    gb_reg_map[GB_REG_A] |= *source;

    //check other flags
    CLR_CARRY_FLAG;
    CLR_HALF_CARRY_FLAG;
    CLR_ADD_SUB_FLAG;
    
    check_zero_flag();
}

/*
cp   r           Bx         4 z1hc compare A-r
cp   n           FE nn      8 z1hc compare A-n
cp   (HL)        BE         8 z1hc compare A-(HL)
*/
void opcode_8_cp(uint8_t* source){

    uint8_t temp = gb_reg_map[GB_REG_A];

    if (check_8_underflow(source)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_underflow(source)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    gb_reg_map[GB_REG_A] -= *source;

    SET_ADD_SUB_FLAG;
    check_zero_flag();

    gb_reg_map[GB_REG_A] = temp;
}

/*
inc  r           xx         4 z0h- r=r+1
inc  (HL)        34        12 z0h- (HL)=(HL)+1
*/
void opcode_8_inc(uint8_t* dest){
    //check carry flags first
    if(check_4_overflow(dest)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    gb_reg_map[GB_REG_A] = *dest++;

    CLR_ADD_SUB_FLAG;
    
    check_zero_flag();
}

/*
dec  r           xx         4 z1h- r=r-1
dec  (HL)        35        12 z1h- (HL)=(HL)-1
*/
void opcode_8_dec(uint8_t* dest){
    //check carry flags first
    if(check_4_underflow(dest)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    //do opperartion
    gb_reg_map[GB_REG_A] = *dest--;

    SET_ADD_SUB_FLAG;
    
    check_zero_flag();
}

/*
daa              27         4 z-0x decimal adjust akku
*/
void opcode_8_daa(){

    CLR_HALF_CARRY_FLAG;
    check_zero_flag();
}

/*
cpl              2F         4 -11- A = A xor FF
*/
void opcode_8_cpl(){
    gb_reg_map[GB_REG_A] ^= 0xFF;
    SET_ADD_SUB_FLAG;
    SET_HALF_CARRY_FLAG;
}



//******************************************************
//16-bit commands
//******************************************************
/*
ld   rr,nn       x1 nn nn  12 ---- rr=nn (rr may be BC,DE,HL or SP)
ld   SP,HL       F9         8 ---- SP=HL
*/
void opcode_16_ld(uint8_t* dest, uint8_t* source){
    *dest = *source;
    *(dest+1) = *(source+1);
}

/*
ld   HL,SP+dd  F8          12 00hc HL = SP +/- dd ;dd is 8bit signed number
*/
void opcode_16_ld_offset(uint8_t* dest, uint8_t* source, int8_t offset){
    *dest = *(source + offset);
}

/*
push rr          x5        16 ---- SP=SP-2  (SP)=rr   (rr may be BC,DE,HL,AF)
*/
void opcode_16_push(uint8_t* source){
    uint16_t temp = get_16_from_8(&gb_reg_map[GB_REG_SP_1]);
    temp -=2;
    gb_reg_map[GB_REG_SP_1] = get_16_high(&temp);
    gb_reg_map[GB_REG_SP_2] = get_16_low(&temp);
    gb_reg_map[temp] = *source;
}
/*
pop  rr          x1        12 (AF) rr=(SP)  SP=SP+2   (rr may be BC,DE,HL,AF)
*/
void opcode_16_pop(uint8_t* dest){
    uint16_t temp = get_16_from_8(&gb_reg_map[GB_REG_SP_1]);
    *dest = gb_reg_map[temp];
    temp +=2;
    gb_reg_map[GB_REG_SP_1] = get_16_high(&temp);
    gb_reg_map[GB_REG_SP_2] = get_16_low(&temp);
}
/*
add  HL,rr     x9           8 -0hc HL = HL+rr     ;rr may be BC,DE,HL,SP
*/
void opcode_16_add_hl(uint8_t* source){
    //first part
    gb_reg_map[GB_REG_A] = gb_reg_map[GB_REG_L];
    if (check_8_overflow(source+1)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_overflow(source+1)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    gb_reg_map[GB_REG_A] += *(source+1);
    gb_reg_map[GB_REG_L] = gb_reg_map[GB_REG_A];

    //second part
    gb_reg_map[GB_REG_A] = gb_reg_map[GB_REG_H];
    if (check_8_overflow(source)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_overflow(source)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }
    gb_reg_map[GB_REG_A] += *source;
    gb_reg_map[GB_REG_A] += GET_CARRY_FLAG;
    gb_reg_map[GB_REG_H] = gb_reg_map[GB_REG_A];

    CLR_ADD_SUB_FLAG;

}

/*
add  SP,dd     E8          16 00hc SP = SP +/- dd ;dd is 8bit signed number
*/
void opcode_16_add_sp(int8_t offset){
    uint16_t temp = get_16_from_8(&gb_reg_map[GB_REG_SP_1]);
    temp += offset;
    gb_reg_map[GB_REG_SP_1] = get_16_high(&temp);
    gb_reg_map[GB_REG_SP_2] = get_16_low(&temp);
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
}

/*
inc  rr        x3           8 ---- rr = rr+1      ;rr may be BC,DE,HL,SP
*/
void opcode_16_inc(uint8_t* dest){
    uint16_t temp = get_16_from_8(dest);
    temp++;
    *dest = get_16_high(&temp);
    *(dest+1) = get_16_low(&temp);
}

/*
dec  rr        xB           8 ---- rr = rr-1      ;rr may be BC,DE,HL,SP
*/
void opcode_16_dec(uint8_t* dest){
    uint16_t temp = get_16_from_8(dest);
    temp--;
    *dest = get_16_high(&temp);
    *(dest+1) = get_16_low(&temp);
}




//******************************************************
// rotate commands
//******************************************************
/*
rlca           07           4 000c rotate akku left
*/
void opcode_rlca(){
    gb_reg_map[GB_REG_A] = (gb_reg_map[GB_REG_A] << 1) | (gb_reg_map[GB_REG_A] >> 7);
    if (gb_reg_map[GB_REG_A] & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
rla            17           4 000c rotate akku left through carry
*/
void opcode_rla(){
    uint8_t temp = GET_CARRY_FLAG;
    if(gb_reg_map[GB_REG_A] & 0x80){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    gb_reg_map[GB_REG_A] = (gb_reg_map[GB_REG_A] << 1) | temp;
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
rrca           0F           4 000c rotate akku right
*/
void opcode_rrca(){
    gb_reg_map[GB_REG_A] = (gb_reg_map[GB_REG_A] >> 0x01) | (gb_reg_map[GB_REG_A] << 0x07);
    if (gb_reg_map[GB_REG_A] & 0x80){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
rra            1F           4 000c rotate akku right through carry
*/
void opcode_rra(){
    uint8_t temp = GET_CARRY_FLAG;
    if(gb_reg_map[GB_REG_A] & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    gb_reg_map[GB_REG_A] = (gb_reg_map[GB_REG_A] >> 0x01) | (temp << 0x07);
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
rlc  r         CB 0x        8 z00c rotate left
rlc  (HL)      CB 06       16 z00c rotate left
*/
void opcode_rlc(uint8_t* dest){
    *dest = (*dest << 1) | (*dest >> 7);
    if (*dest & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    check_zero_flag();
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
rl   r         CB 1x        8 z00c rotate left through carry
rl   (HL)      CB 16       16 z00c rotate left through carry
*/
void opcode_rl(uint8_t* dest){
    uint8_t temp = GET_CARRY_FLAG;
    if(*dest & 0x80){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    *dest = (*dest << 1) | temp;
    check_zero_flag();
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
rrc  r         CB 0x        8 z00c rotate right
rrc  (HL)      CB 0E       16 z00c rotate right
*/
void opcode_rrc(uint8_t* dest){
    *dest = (*dest >> 0x01) | (*dest << 0x07);
    if (*dest & 0x80){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    check_zero_flag();
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
rr   r         CB 1x        8 z00c rotate right through carry
rr   (HL)      CB 1E       16 z00c rotate right through carry
*/
void opcode_rr(uint8_t* dest){
    uint8_t temp = GET_CARRY_FLAG;
    if(*dest & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    *dest = (*dest >> 0x01) | (temp << 0x07);
    check_zero_flag();
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
sla  r         CB 2x        8 z00c shift left arithmetic (b0=0)
sla  (HL)      CB 26       16 z00c shift left arithmetic (b0=0)
*/
void opcode_sla(uint8_t* dest){
    if(*dest & 0x80){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    *dest = (*dest << 1);
    check_zero_flag();
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;

}

/*
swap r         CB 3x        8 z000 exchange low/hi-nibble
swap (HL)      CB 36       16 z000 exchange low/hi-nibble
*/
void opcode_swap(uint8_t* dest){
    *dest = (*dest << 0x04) & (*dest >> 0x04);
    check_zero_flag();
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    CLR_CARRY_FLAG;
}

/*
sra  r         CB 2x        8 z00c shift right arithmetic (b7=b7)
sra  (HL)      CB 2E       16 z00c shift right arithmetic (b7=b7)
*/
void opcode_sra(uint8_t* dest){
    uint8_t temp = *dest & 0x08;
    if(*dest & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    *dest = (*dest >> 1) & temp;
    check_zero_flag();
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
srl  r         CB 3x        8 z00c shift right logical (b7=0)
srl  (HL)      CB 3E       16 z00c shift right logical (b7=0)
*/
void opcode_srl(uint8_t* dest){
    if(*dest & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    *dest = (*dest >> 1);
    check_zero_flag();
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}



//******************************************************
// Single bit opperations
//******************************************************
/*
bit  n,r       CB xx       8 z01- test bit n
bit  n,(HL)    CB xx       12 z01- test bit n
*/
void opcode_bit(uint8_t* dest, uint8_t bit){
    CLR_ADD_SUB_FLAG;
    SET_HALF_CARRY_FLAG;
    if(*dest & (0x01 << bit)){
        SET_ZERO_FLAG;
    } else {
        CLR_ZERO_FLAG;
    }
}

/*
set  n,r       CB xx        8 ---- set bit n
set  n,(HL)    CB xx       16 ---- set bit n
*/
void opcode_set(uint8_t* dest, uint8_t bit){
    *dest = *dest | (0x01 << bit);
}

/*
res  n,r       CB xx        8 ---- reset bit n
res  n,(HL)    CB xx       16 ---- reset bit n
*/
void opcode_res(uint8_t* dest, uint8_t bit){
    *dest = *dest & ~(0x01 << bit);
}



//******************************************************
//cpu commands
//******************************************************
/*
ccf            3F           4 -00c cy=cy xor 1
*/
void opcode_ccf(){
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    if(GET_CARRY_FLAG){
        CLR_CARRY_FLAG;
    } else {
        SET_CARRY_FLAG;
    }
}

/*
scf            37           4 -001 cy=1
*/
void opcode_scf(){
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    SET_CARRY_FLAG;
}

/*
nop            00           4 ---- no operation
*/
void opcode_nop(){
    //do nothing.....
}

/*
halt           76         N*4 ---- halt until interrupt occurs (low power)
*/
void opcode_halt(){
    power_mode = PWR_HALT;
}

/*
stop           10 00        ? ---- low power standby mode (VERY low power)
*/
void opcode_stop(){
    power_mode = PWR_STOP;
}

/*
di             F3           4 ---- disable interrupts, IME=0
*/
void opcode_di(){
    gb_mem_map[INTERUPT_EN_FLAG] = 0x00;
}

/*
ei             FB           4 ---- enable interrupts, IME=1
*/
void opcode_ei(){
    gb_mem_map[INTERUPT_EN_FLAG] = 0x01;
}



//******************************************************
//jump commands
//******************************************************
/*
jp   nn        C3 nn nn    16 ---- jump to nn, PC=nn
*/
void opcode_jp(uint16_t address){
    gb_reg_map[GB_REG_PC_1] = get_16_high(&address);
    gb_reg_map[GB_REG_PC_2] = get_16_low(&address);
}

/*
jp   HL        E9           4 ---- jump to HL, PC=HL
*/
void opcode_jp_hl(){
    gb_reg_map[GB_REG_PC_1] = gb_reg_map[GB_REG_H];
    gb_reg_map[GB_REG_PC_2] = gb_reg_map[GB_REG_L];
}

/*
jp   f,nn      xx nn nn 16;12 ---- conditional jump if nz,z,nc,c
*/
void opcode_jp_cnd(uint16_t address, uint8_t condition){
    if((condition == 0 && GET_ZERO_FLAG == 0) || 
        (condition == 1 && GET_ZERO_FLAG == 1) ||
        (condition == 2 && GET_CARRY_FLAG == 0) ||
        (condition == 3 && GET_CARRY_FLAG == 1)){
        opcode_jp(address);
    }
}

/*
jr   PC+dd     18 dd       12 ---- relative jump to nn (PC=PC+/-7bit)
*/
void opcode_jr(int8_t offset){
    uint16_t temp = get_16_from_8(&gb_reg_map[GB_REG_PC_1]);
    temp += offset;
    gb_reg_map[GB_REG_PC_1] = get_16_high(&temp);
    gb_reg_map[GB_REG_PC_2] = get_16_low(&temp);
}

/*
jr   f,PC+dd   xx dd     12;8 ---- conditional relative jump if nz,z,nc,c
*/
void opcode_jr_cnd(int8_t offset,uint8_t condition){
    if((condition == 0 && GET_ZERO_FLAG == 0) || 
        (condition == 1 && GET_ZERO_FLAG == 1) ||
        (condition == 2 && GET_CARRY_FLAG == 0) ||
        (condition == 3 && GET_CARRY_FLAG == 1)){
        opcode_jr(offset);
    }
}

/*
call nn        CD nn nn    24 ---- call to nn, SP=SP-2, (SP)=PC, PC=nn
*/
void opcode_call(uint16_t address){
    uint16_t temp = get_16_from_8(&gb_reg_map[GB_REG_SP_1]);
    temp -= 2;
    gb_reg_map[GB_REG_SP_1] = get_16_high(&temp);
    gb_reg_map[GB_REG_SP_2] = get_16_low(&temp);
    gb_mem_map[temp] = gb_reg_map[GB_REG_PC_1];
    gb_mem_map[temp+1] = gb_reg_map[GB_REG_PC_2];
    gb_reg_map[GB_REG_PC_1] = get_16_high(&address);
    gb_reg_map[GB_REG_PC_2] = get_16_low(&address);
}

/*
call f,nn      xx nn nn 24;12 ---- conditional call if nz,z,nc,c
*/
void opcode_call_cnd(uint16_t address, uint8_t condition){
    if((condition == 0 && GET_ZERO_FLAG == 0) || 
        (condition == 1 && GET_ZERO_FLAG == 1) ||
        (condition == 2 && GET_CARRY_FLAG == 0) ||
        (condition == 3 && GET_CARRY_FLAG == 1)){
        opcode_call(address);
    }
}

/*
ret            C9          16 ---- return, PC=(SP), SP=SP+2
*/
void opcode_ret(){
    uint16_t temp = get_16_from_8(&gb_reg_map[GB_REG_SP_1]);
    gb_reg_map[GB_REG_PC_1] = gb_reg_map[temp];
    gb_reg_map[GB_REG_PC_2] = gb_reg_map[temp+1];
    temp += 2;
    gb_reg_map[GB_REG_SP_1] = get_16_high(&temp);
    gb_reg_map[GB_REG_SP_2] = get_16_low(&temp);
}

/*
ret  f         xx        20;8 ---- conditional return if nz,z,nc,c
*/
void opcode_ret_cnd(uint8_t condition){
    if((condition == 0 && GET_ZERO_FLAG == 0) || 
        (condition == 1 && GET_ZERO_FLAG == 1) ||
        (condition == 2 && GET_CARRY_FLAG == 0) ||
        (condition == 3 && GET_CARRY_FLAG == 1)){
        opcode_ret();
    }
}

/*
reti           D9          16 ---- return and enable interrupts (IME=1)
*/
void opcode_reti(){
    opcode_ret();
    opcode_ei();
}

/*
rst  n         xx          16 ---- call to 00,08,10,18,20,28,30,38
*/
void opcode_rst(uint16_t address){
    opcode_call(address);
}

//this should never be called
void opcode_CB_prefix(){

}

//this should never be called
void opcode_NA(){
    //do nothing
}