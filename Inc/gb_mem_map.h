#ifndef __GB_MEM_MAP_H
#define __GB_MEM_MAP_H
#include <stdint.h>
#include <stdio.h> 

/*

 	$FFFF 	Interrupt Enable Flag
	$FF80-$FFFE 	Zero Page - 127 bytes
	$FF00-$FF7F 	Hardware I/O Registers
	$FEA0-$FEFF 	Unusable Memory
	$FE00-$FE9F 	OAM - Object Attribute Memory
	$E000-$FDFF 	Echo RAM - Reserved, Do Not Use
	$D000-$DFFF 	Internal RAM - Bank 1-7 (switchable - CGB only)
	$C000-$CFFF 	Internal RAM - Bank 0 (fixed)
	$A000-$BFFF 	Cartridge RAM (If Available)
	$9C00-$9FFF 	BG Map Data 2
	$9800-$9BFF 	BG Map Data 1
	$8000-$97FF 	Character RAM
	$4000-$7FFF 	Cartridge ROM - Switchable Banks 1-xx
	$0150-$3FFF 	Cartridge ROM - Bank 0 (fixed)
	$0100-$014F 	Cartridge Header Area
	$0000-$00FF 	Restart and Interrupt Vectors
*/

#define GB_MEM_SIZE         0xFFFF

#define INTERUPT_EN_FLAG    0xFFFF
#define ZERO_PAGE           0xFF80
#define IO_PORTS            0xFF00
#define NA_MEM              0xFEA0
#define OAM                 0xFE00
#define ECHO_RAM            0xE000
#define GB_RAM_2            0xD000
#define GB_RAM_1            0xC000
#define CART_RAM            0xA000
#define BG_MAP_2            0x9C00
#define BG_MAP_1            0x9800
#define CHAR_RAM            0x8000
#define CART_ROM_1          0x4000
#define CART_ROM_0          0x0000

#define CART_MB_SIZE		0x4000

//interupt and start addr locations
#define RESTART_ADDR_00		0x0000
#define RESTART_ADDR_08		0x0008
#define RESTART_ADDR_10		0x0010
#define RESTART_ADDR_18		0x0018
#define RESTART_ADDR_20		0x0020
#define RESTART_ADDR_28		0x0028
#define RESTART_ADDR_30		0x0030
#define RESTART_ADDR_38		0x0038

#define VBLANK_INTR					0x0040
#define LCDC_STAT_INTR_ADDR			0x0048
#define TIMER_OVERFLOW_INTR_ADDR	0x0050
#define SERIAL_COMPLETE_INTR_ADDR	0x0058
#define HTL_P10_P13_INTR_ADDR 		0x0060

//cart info locations
#define CART_START 			0x0100
#define CART_LOGO			0x0104
#define CART_TITLE			0x0134
#define CART_GB_TYPE		0x0143
#define CART_LIC_NEW_HIGH	0x0144
#define CART_LIC_NEW_LOW 	0x0145
#define CART_SGB_TYPE 		0x0146
#define CART_TYPE 			0x0147
#define CART_ROM_SIZE 		0x0148
#define CART_RAM_SIZE 		0x0149
#define CART_REGION 		0x014A
#define CART_LIC_OLD 		0x014B
#define CART_ROM_VER		0x014C
#define CART_COMP_CHECK 	0x014D
#define CART_CHECK_SUM_1 	0x014E
#define CART_CHECK_SUM_2 	0x014F

//CART_GB_TYPE
#define GB_TYPE_CGB 0x80

//CART_SGB_TYPE
#define SGB_TYPE_NA 0x00
#define SGB_TYPE_SGB 0x03

//CART_TYPE
#define TYPE_ROM						0x02
#define TYPE_ROM_MBC1					0x01
#define TYPE_ROM_MBC1_RAM				0x02
#define TYPE_ROM_MBC1_RAM_BAT			0x03
#define TYPE_ROM_MBC2					0x05
#define TYPE_ROM_MBC2_BAT				0x06
#define TYPE_ROM_RAM					0x08
#define TYPE_ROM_RAM_BAT				0x09
#define TYPE_ROM_MMM01					0x0B
#define TYPE_ROM_MMM01_SRAM				0x0C
#define TYPE_ROM_MMM01_SRAM_BAT			0x0D
#define TYPE_ROM_MBC3_TIMER_BAT			0x0F
#define TYPE_ROM_MBC3_TIMER_RAM_BAT		0x10
#define TYPE_ROM_MBC3					0x11
#define TYPE_ROM_MBC3_RAM				0x12
#define TYPE_ROM_MBC3_RAM_BAT			0x13
#define TYPE_ROM_MBC5					0x19
#define TYPE_ROM_MBC5_RAM				0x1A
#define TYPE_ROM_MBC5_RAM_BAT			0x1B
#define TYPE_ROM_MBC5_RUMBL				0x1C
#define TYPE_ROM_MBC5_RUMBL_SRAM		0x1D
#define TYPE_ROM_MBC5_RUMBL_SRAM_BAT	0x1E
#define TYPE_POCKET_CAM					0x1F
#define TYPE_BANDAI_TAMA5				0xFD
#define TYPE_HUDSON_HUC_3				0xFE
#define TYPE_HUDSON_HUC_1				0xFF


//CART_ROM_SIZE
#define ROM_SIZE_32KB	0x00
#define ROM_SIZE_64KB	0x01
#define ROM_SIZE_128KB	0x02
#define ROM_SIZE_256KB	0x03
#define ROM_SIZE_512KB	0x04
#define ROM_SIZE_1MB	0x05
#define ROM_SIZE_2MB	0x06
#define ROM_SIZE_4MB	0x07
#define ROM_SIZE_8MB	0x08
#define ROM_SIZE_1_1MB	0x52
#define ROM_SIZE_1_2MB	0x53
#define ROM_SIZE_1_5MB	0x54

//CART_RAM_SIZE
#define RAM_SIZE_0		0x00
#define RAM_SIZE_2KB 	0x01
#define RAM_SIZE_8KB	0x02
#define RAM_SIZE_32KB	0x03
#define RAM_SIZE_128KB	0x04

//CART_REGION
#define REGION_JAP		0x00
#define REGION_OTHER	0x01

//CART_LIC_OLD
#define LIC_OLD_CHECK_NEW 	0x33
#define LIC_OLD_ACCOLADE	0x79
#define LIC_OLD_KONAMI		0xA4

/*
Nintendo logo:
CE ED 66 66 CC 0D 00 0B 03 73 00 83 00 0C 00 0D
00 08 11 1F 88 89 00 0E DC CC 6E E6 DD DD D9 99
BB BB 67 63 6E 0E EC CC DD DC 99 9F BB B9 33 3E
*/


//IO-PORTS
//LCD
#define IO_LCD_CONTROL	0xFF40
#define IO_LCD_STATUS	0xFF41
#define IO_LCD_SCY		0xFF42
#define IO_LCD_SCX		0xFF43
#define IO_LCD_LY		0xFF44
#define IO_LCD_LYC		0xFF45
#define IO_LCD_DMA		0xFF46
#define IO_LCD_BGP		0xFF47
#define IO_LCD_OBP0		0xFF48
#define IO_LCD_OBP1		0xFF49
#define IO_LCD_WY		0xFF4A

//cgb only
#define IO_LCD_VBK		0xFF4F
#define IO_LCD_HDMA1	0xFF51
#define IO_LCD_HDMA2	0xFF52
#define IO_LCD_HDMA3	0xFF53
#define IO_LCD_HDMA4	0xFF54
#define IO_LCD_HDMA5	0xFF55
#define IO_LCD_BCPS		0xFF68
#define IO_LCD_BCPD		0xFF69
#define IO_LCD_OCPS		0xFF6A
//---

//VRAM tile data
#define VRAM_BLOCK_0	0x8000
#define VRAM_BLOCK_1	0x8800
#define VRAM_BLOCK_2	0x9000


//SOUND
#define	IO


//CPU Registers
#define GB_REG_SIZE 0x0C

#define GB_REG_B	0x00
#define GB_REG_C	0x01
#define GB_REG_BC	0x00
#define GB_REG_D	0x02
#define GB_REG_E	0x03
#define GB_REG_DE	0x02
#define GB_REG_H	0x04
#define GB_REG_L	0x05
#define GB_REG_HL	0x04
#define GB_REG_A	0x06
#define GB_REG_F	0x07
#define GB_REG_AF	0x06

#define GB_REG_SP_1	0x08
#define GB_REG_SP_2	0x09
#define GB_REG_PC_1	0x0A
#define GB_REG_PC_2	0x0B

//flags
#define ZERO_FLAG       BIT_7
#define ADD_SUB_FLAG    BIT_6
#define HALF_CARRY_FLAG BIT_5
#define CARRY_FLAG      BIT_4

#define GET_ZERO_FLAG		((gb_reg_map[GB_REG_F] & ZERO_FLAG) >> 0x07)
#define GET_ADD_SUB_FLAG	((gb_reg_map[GB_REG_F] & ZERO_FLAG) >> 0x06)
#define GET_HALF_CARRY_FLAG	((gb_reg_map[GB_REG_F] & ZERO_FLAG) >> 0x05)
#define GET_CARRY_FLAG		((gb_reg_map[GB_REG_F] & ZERO_FLAG) >> 0x04)

#define GET_HIGH_NIBBLE x ((x & 0xF0) >> 0x04)
#define GET_LOW_NIBBLE x (x & 0x0F)

extern uint8_t gb_mem_map[GB_MEM_SIZE];
extern uint8_t gb_reg_map[GB_REG_SIZE];
extern uint8_t bc_pointer;
extern uint8_t de_pointer;
extern uint8_t hl_pointer;

extern uint8_t u8_pointer;
extern uint8_t u16_pointer_1;
extern uint8_t u16_pointer_2;
extern uint8_t s8_pointer;

extern uint8_t addr_38H;
extern uint8_t addr_30H;
extern uint8_t addr_28H;
extern uint8_t addr_20H;
extern uint8_t addr_18H;
extern uint8_t addr_10H;
extern uint8_t addr_08H;
extern uint8_t addr_00H;

#define SET_ZERO_FLAG		gb_reg_map[GB_REG_F] |= ZERO_FLAG
#define CLR_ZERO_FLAG		gb_reg_map[GB_REG_F] &= ~ZERO_FLAG
#define SET_ADD_SUB_FLAG	gb_reg_map[GB_REG_F] |= ADD_SUB_FLAG
#define CLR_ADD_SUB_FLAG	gb_reg_map[GB_REG_F] &= ~ADD_SUB_FLAG
#define SET_HALF_CARRY_FLAG	gb_reg_map[GB_REG_F] |= HALF_CARRY_FLAG
#define CLR_HALF_CARRY_FLAG	gb_reg_map[GB_REG_F] &= ~HALF_CARRY_FLAG
#define SET_CARRY_FLAG		gb_reg_map[GB_REG_F] |= CARRY_FLAG
#define CLR_CARRY_FLAG		gb_reg_map[GB_REG_F] &= ~CARRY_FLAG

void parse_header();
uint16_t get_16_from_8(uint8_t* source);
uint8_t get_16_low(uint16_t* source);
uint8_t get_16_high(uint16_t* source);

#endif