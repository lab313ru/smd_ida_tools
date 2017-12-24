%include "nasmhead.inc"

%define CYCLE_FOR_TAKE_Z80_BUS_GENESIS 16
%define CYCLE_FOR_TAKE_Z80_BUS_SEGACD 32

	extern Write_To_68K_Space
	extern Read_To_68K_Space

section .data align=64

	extern Controller_1_Counter
	extern Controller_1_Delay
	extern Controller_1_State
	extern Controller_1_COM
	extern Controller_2_Counter
	extern Controller_2_Delay
	extern Controller_2_State
	extern Controller_2_COM
	extern Memory_Control_Status
	extern Cell_Conv_Tab
	extern VDP_Current_Line
	extern hook_address
	extern hook_value
	extern hook_pc
	extern hook_write_byte

	DECL Genesis_M68K_Read_Byte_Table
		dd M68K_Read_Byte_Rom0,		; 0x000000 - 0x07FFFF
		dd M68K_Read_Byte_Rom1,		; 0x080000 - 0x0FFFFF
		dd M68K_Read_Byte_Rom2,		; 0x100000 - 0x17FFFF
		dd M68K_Read_Byte_Rom3,		; 0x180000 - 0x1FFFFF
		dd M68K_Read_Byte_Rom4,		; 0x200000 - 0x27FFFF
		dd M68K_Read_Byte_Rom5,		; 0x280000 - 0x2FFFFF
		dd M68K_Read_Byte_Rom6,		; 0x300000 - 0x37FFFF
		dd M68K_Read_Byte_Rom7,		; 0x380000 - 0x3FFFFF
		dd M68K_Read_Byte_Rom8,		; 0x400000 - 0x47FFFF
		dd M68K_Read_Byte_Rom9,		; 0x480000 - 0x4FFFFF
		dd M68K_Read_Byte_RomA,		; 0x500000 - 0x57FFFF
		dd M68K_Read_Byte_RomB,		; 0x580000 - 0x5FFFFF
		dd M68K_Read_Byte_RomC,		; 0x600000 - 0x67FFFF
		dd M68K_Read_Byte_RomD,		; 0x680000 - 0x6FFFFF
		dd M68K_Read_Byte_RomE,		; 0x700000 - 0x77FFFF
		dd M68K_Read_Byte_RomF,		; 0x780000 - 0x7FFFFF
		dd M68K_Read_Byte_Bad,		; 0x800000 - 0x87FFFF
		dd M68K_Read_Byte_Bad,		; 0x880000 - 0x8FFFFF
		dd M68K_Read_Byte_Bad,		; 0x900000 - 0x97FFFF
		dd M68K_Read_Byte_Bad,		; 0x980000 - 0x9FFFFF
		dd M68K_Read_Byte_Misc,		; 0xA00000 - 0xA7FFFF
		dd M68K_Read_Byte_Bad,		; 0xA80000 - 0xAFFFFF
		dd M68K_Read_Byte_Bad,		; 0xB00000 - 0xB7FFFF
		dd M68K_Read_Byte_Bad,		; 0xB80000 - 0xBFFFFF
		dd M68K_Read_Byte_VDP,		; 0xC00000 - 0xC7FFFF
		dd M68K_Read_Byte_Bad,		; 0xC80000 - 0xCFFFFF
		dd M68K_Read_Byte_Bad,		; 0xD00000 - 0xD7FFFF
		dd M68K_Read_Byte_Bad,		; 0xD80000 - 0xDFFFFF
		dd M68K_Read_Byte_Ram,		; 0xE00000 - 0xE7FFFF
		dd M68K_Read_Byte_Ram,		; 0xE80000 - 0xEFFFFF
		dd M68K_Read_Byte_Ram,		; 0xF00000 - 0xF7FFFF
		dd M68K_Read_Byte_Ram,		; 0xF80000 - 0xFFFFFF

	DECL Genesis_M68K_Read_Word_Table
		dd M68K_Read_Word_Rom0,		; 0x000000 - 0x07FFFF
		dd M68K_Read_Word_Rom1,		; 0x080000 - 0x0FFFFF
		dd M68K_Read_Word_Rom2,		; 0x100000 - 0x17FFFF
		dd M68K_Read_Word_Rom3,		; 0x180000 - 0x1FFFFF
		dd M68K_Read_Word_Rom4,		; 0x200000 - 0x27FFFF
		dd M68K_Read_Word_Rom5,		; 0x280000 - 0x2FFFFF
		dd M68K_Read_Word_Rom6,		; 0x300000 - 0x37FFFF
		dd M68K_Read_Word_Rom7,		; 0x380000 - 0x3FFFFF
		dd M68K_Read_Word_Rom8,		; 0x400000 - 0x47FFFF
		dd M68K_Read_Word_Rom9,		; 0x480000 - 0x4FFFFF
		dd M68K_Read_Word_RomA,		; 0x500000 - 0x57FFFF
		dd M68K_Read_Word_RomB,		; 0x580000 - 0x5FFFFF
		dd M68K_Read_Word_RomC,		; 0x600000 - 0x67FFFF
		dd M68K_Read_Word_RomD,		; 0x680000 - 0x6FFFFF
		dd M68K_Read_Word_RomE,		; 0x700000 - 0x77FFFF
		dd M68K_Read_Word_RomF,		; 0x780000 - 0x7FFFFF
		dd M68K_Read_Word_Bad,		; 0x800000 - 0x87FFFF
		dd M68K_Read_Word_Bad,		; 0x880000 - 0x8FFFFF
		dd M68K_Read_Word_Bad,		; 0x900000 - 0x97FFFF
		dd M68K_Read_Word_Bad,		; 0x980000 - 0x9FFFFF
		dd M68K_Read_Word_Misc,		; 0xA00000 - 0xA7FFFF
		dd M68K_Read_Word_Bad,		; 0xA80000 - 0xAFFFFF
		dd M68K_Read_Word_Bad,		; 0xB00000 - 0xB7FFFF
		dd M68K_Read_Word_Bad,		; 0xB80000 - 0xBFFFFF
		dd M68K_Read_Word_VDP,		; 0xC00000 - 0xC7FFFF
		dd M68K_Read_Word_Bad,		; 0xC80000 - 0xCFFFFF
		dd M68K_Read_Word_Bad,		; 0xD00000 - 0xD7FFFF
		dd M68K_Read_Word_Bad,		; 0xD80000 - 0xDFFFFF
		dd M68K_Read_Word_Ram,		; 0xE00000 - 0xE7FFFF
		dd M68K_Read_Word_Ram,		; 0xE80000 - 0xEFFFFF
		dd M68K_Read_Word_Ram,		; 0xF00000 - 0xF7FFFF
		dd M68K_Read_Word_Ram,		; 0xF80000 - 0xFFFFFF

	DECL Genesis_M68K_Write_Byte_Table
		dd M68K_Write_Byte_SRAM,	; 0x000000 - 0x0FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x100000 - 0x1FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x200000 - 0x2FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x300000 - 0x3FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x400000 - 0x4FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x500000 - 0x5FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x600000 - 0x6FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x700000 - 0x7FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x800000 - 0x8FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x900000 - 0x9FFFFF
		dd M68K_Write_Byte_Misc,	; 0xA00000 - 0xAFFFFF
		dd M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd M68K_Write_Byte_VDP,		; 0xC00000 - 0xCFFFFF
		dd M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd M68K_Write_Byte_Ram,		; 0xE00000 - 0xEFFFFF
		dd M68K_Write_Byte_Ram,		; 0xF00000 - 0xFFFFFF

	DECL Genesis_M68K_Write_Byte_Cheat_Table
		dd M68K_Write_Byte_ROM,		; 0x000000 - 0x0FFFFF
		dd M68K_Write_Byte_ROM,		; 0x100000 - 0x1FFFFF
		dd M68K_Write_Byte_ROM2,	; 0x200000 - 0x2FFFFF
		dd M68K_Write_Byte_ROM,		; 0x300000 - 0x3FFFFF
		dd M68K_Write_Byte_ROM,		; 0x400000 - 0x4FFFFF
		dd M68K_Write_Byte_ROM,		; 0x500000 - 0x5FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x600000 - 0x6FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x700000 - 0x7FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x800000 - 0x8FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x900000 - 0x9FFFFF
		dd M68K_Write_Byte_Misc,	; 0xA00000 - 0xAFFFFF
		dd M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd M68K_Write_Byte_VDP,		; 0xC00000 - 0xCFFFFF
		dd M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd M68K_Write_Byte_Ram,		; 0xE00000 - 0xEFFFFF
		dd M68K_Write_Byte_Ram,		; 0xF00000 - 0xFFFFFF


	DECL Genesis_M68K_Write_Word_Table
		dd M68K_Write_Word_SRAM,	; 0x000000 - 0x0FFFFF
		dd M68K_Write_Word_SRAM,	; 0x100000 - 0x1FFFFF
		dd M68K_Write_Word_SRAM,	; 0x200000 - 0x2FFFFF
		dd M68K_Write_Word_SRAM,	; 0x300000 - 0x3FFFFF
		dd M68K_Write_Word_SRAM,	; 0x400000 - 0x4FFFFF
		dd M68K_Write_Word_SRAM,	; 0x500000 - 0x5FFFFF
		dd M68K_Write_Word_SRAM,	; 0x600000 - 0x6FFFFF
		dd M68K_Write_Word_SRAM,	; 0x700000 - 0x7FFFFF
		dd M68K_Write_Word_SRAM,	; 0x800000 - 0x8FFFFF
		dd M68K_Write_Word_SRAM,	; 0x900000 - 0x9FFFFF
		dd M68K_Write_Word_Misc,	; 0xA00000 - 0xAFFFFF
		dd M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd M68K_Write_Word_VDP,		; 0xC00000 - 0xCFFFFF
		dd M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd M68K_Write_Word_Ram,		; 0xE00000 - 0xEFFFFF
		dd M68K_Write_Word_Ram,		; 0xF00000 - 0xFFFFFF

	; Sega CD Default Jump Table

	DECL SegaCD_M68K_Read_Byte_Table
		dd M68K_Read_Byte_Bios_CD,	; 0x000000 - 0x07FFFF
		dd M68K_Read_Byte_Bad,		; 0x080000 - 0x0FFFFF
		dd M68K_Read_Byte_Bad,		; 0x100000 - 0x17FFFF
		dd M68K_Read_Byte_Bad,		; 0x180000 - 0x1FFFFF
		dd M68K_Read_Byte_WRam,		; 0x200000 - 0x27FFFF
		dd M68K_Read_Byte_Bad,		; 0x280000 - 0x2FFFFF
		dd M68K_Read_Byte_Bad,		; 0x300000 - 0x37FFFF
		dd M68K_Read_Byte_Bad,		; 0x380000 - 0x3FFFFF
		dd M68K_Read_Byte_BRAM_L,	; 0x400000 - 0x47FFFF
		dd M68K_Read_Byte_Bad,		; 0x480000 - 0x4FFFFF
		dd M68K_Read_Byte_Bad,		; 0x500000 - 0x57FFFF
		dd M68K_Read_Byte_Bad,		; 0x580000 - 0x5FFFFF
		dd M68K_Read_Byte_BRAM,		; 0x600000 - 0x67FFFF
		dd M68K_Read_Byte_Bad,		; 0x680000 - 0x6FFFFF
		dd M68K_Read_Byte_Bad,		; 0x700000 - 0x77FFFF
		dd M68K_Read_Byte_BRAM_W,	; 0x780000 - 0x7FFFFF
		dd M68K_Read_Byte_Bad,		; 0x800000 - 0x87FFFF
		dd M68K_Read_Byte_Bad,		; 0x880000 - 0x8FFFFF
		dd M68K_Read_Byte_Bad,		; 0x900000 - 0x97FFFF
		dd M68K_Read_Byte_Bad,		; 0x980000 - 0x9FFFFF
		dd M68K_Read_Byte_Misc_CD,	; 0xA00000 - 0xA7FFFF
		dd M68K_Read_Byte_Bad,		; 0xA80000 - 0xAFFFFF
		dd M68K_Read_Byte_Bad,		; 0xB00000 - 0xB7FFFF
		dd M68K_Read_Byte_Bad,		; 0xB80000 - 0xBFFFFF
		dd M68K_Read_Byte_VDP,		; 0xC00000 - 0xC7FFFF
		dd M68K_Read_Byte_Bad,		; 0xC80000 - 0xCFFFFF
		dd M68K_Read_Byte_Bad,		; 0xD00000 - 0xD7FFFF
		dd M68K_Read_Byte_Bad,		; 0xD80000 - 0xDFFFFF
		dd M68K_Read_Byte_Ram,		; 0xE00000 - 0xE7FFFF
		dd M68K_Read_Byte_Ram,		; 0xE80000 - 0xEFFFFF
		dd M68K_Read_Byte_Ram,		; 0xF00000 - 0xF7FFFF
		dd M68K_Read_Byte_Ram,		; 0xF80000 - 0xFFFFFF

	DECL SegaCD_M68K_Read_Word_Table
		dd M68K_Read_Word_Bios_CD,	; 0x000000 - 0x07FFFF
		dd M68K_Read_Word_Bad,		; 0x080000 - 0x0FFFFF
		dd M68K_Read_Word_Bad,		; 0x100000 - 0x17FFFF
		dd M68K_Read_Word_Bad,		; 0x180000 - 0x1FFFFF
		dd M68K_Read_Word_WRam,		; 0x200000 - 0x27FFFF
		dd M68K_Read_Word_Bad,		; 0x280000 - 0x2FFFFF
		dd M68K_Read_Word_Bad,		; 0x300000 - 0x37FFFF
		dd M68K_Read_Word_Bad,		; 0x380000 - 0x3FFFFF
		dd M68K_Read_Word_BRAM_L,	; 0x400000 - 0x47FFFF
		dd M68K_Read_Word_Bad,		; 0x480000 - 0x4FFFFF
		dd M68K_Read_Word_Bad,		; 0x500000 - 0x57FFFF
		dd M68K_Read_Word_Bad,		; 0x580000 - 0x5FFFFF
		dd M68K_Read_Word_BRAM,		; 0x600000 - 0x67FFFF
		dd M68K_Read_Word_Bad,		; 0x680000 - 0x6FFFFF
		dd M68K_Read_Word_Bad,		; 0x700000 - 0x77FFFF
		dd M68K_Read_Word_BRAM_W,	; 0x780000 - 0x7FFFFF
		dd M68K_Read_Word_Bad,		; 0x800000 - 0x87FFFF
		dd M68K_Read_Word_Bad,		; 0x880000 - 0x8FFFFF
		dd M68K_Read_Word_Bad,		; 0x900000 - 0x97FFFF
		dd M68K_Read_Word_Bad,		; 0x980000 - 0x9FFFFF
		dd M68K_Read_Word_Misc_CD,	; 0xA00000 - 0xA7FFFF
		dd M68K_Read_Word_Bad,		; 0xA80000 - 0xAFFFFF
		dd M68K_Read_Word_Bad,		; 0xB00000 - 0xB7FFFF
		dd M68K_Read_Word_Bad,		; 0xB80000 - 0xBFFFFF
		dd M68K_Read_Word_VDP,		; 0xC00000 - 0xC7FFFF
		dd M68K_Read_Word_Bad,		; 0xC80000 - 0xCFFFFF
		dd M68K_Read_Word_Bad,		; 0xD00000 - 0xD7FFFF
		dd M68K_Read_Word_Bad,		; 0xD80000 - 0xDFFFFF
		dd M68K_Read_Word_Ram,		; 0xE00000 - 0xE7FFFF
		dd M68K_Read_Word_Ram,		; 0xE80000 - 0xEFFFFF
		dd M68K_Read_Word_Ram,		; 0xF00000 - 0xF7FFFF
		dd M68K_Read_Word_Ram,		; 0xF80000 - 0xFFFFFF

	DECL SegaCD_M68K_Write_Byte_Table
		dd M68K_Write_Byte_Bios_CD,	; 0x000000 - 0x0FFFFF
		dd M68K_Write_Bad,			; 0x100000 - 0x1FFFFF
		dd M68K_Write_Byte_WRam,	; 0x200000 - 0x2FFFFF
		dd M68K_Write_Bad,			; 0x300000 - 0x3FFFFF
		dd M68K_Write_Bad,			; 0x400000 - 0x4FFFFF
		dd M68K_Write_Bad,			; 0x500000 - 0x5FFFFF
		dd M68K_Write_Byte_BRAM,	; 0x600000 - 0x6FFFFF
		dd M68K_Write_Byte_BRAM_W,	; 0x700000 - 0x7FFFFF
		dd M68K_Write_Bad,			; 0x800000 - 0x8FFFFF
		dd M68K_Write_Bad,			; 0x900000 - 0x9FFFFF
		dd M68K_Write_Byte_Misc_CD,	; 0xA00000 - 0xAFFFFF
		dd M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd M68K_Write_Byte_VDP,		; 0xC00000 - 0xCFFFFF
		dd M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd M68K_Write_Byte_Ram,		; 0xE00000 - 0xEFFFFF
		dd M68K_Write_Byte_Ram,		; 0xF00000 - 0xFFFFFF

	DECL SegaCD_M68K_Write_Word_Table
		dd M68K_Write_Word_Bios_CD,	; 0x000000 - 0x0FFFFF
		dd M68K_Write_Bad,			; 0x100000 - 0x1FFFFF
		dd M68K_Write_Word_WRam,	; 0x200000 - 0x2FFFFF
		dd M68K_Write_Bad,			; 0x300000 - 0x3FFFFF
		dd M68K_Write_Bad,			; 0x400000 - 0x4FFFFF
		dd M68K_Write_Bad,			; 0x500000 - 0x5FFFFF
		dd M68K_Write_Word_BRAM,	; 0x600000 - 0x6FFFFF
		dd M68K_Write_Word_BRAM_W,	; 0x700000 - 0x7FFFFF
		dd M68K_Write_Bad,			; 0x800000 - 0x8FFFFF
		dd M68K_Write_Bad,			; 0x900000 - 0x9FFFFF
		dd M68K_Write_Word_Misc_CD,	; 0xA00000 - 0xAFFFFF
		dd M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd M68K_Write_Word_VDP,		; 0xC00000 - 0xCFFFFF
		dd M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd M68K_Write_Word_Ram,		; 0xE00000 - 0xEFFFFF
		dd M68K_Write_Word_Ram,		; 0xF00000 - 0xFFFFFF

	; 32X Default Jump Table

	DECL _32X_M68K_Read_Byte_Table
		dd M68K_Read_Byte_Rom0,		; 0x000000 - 0x07FFFF
		dd M68K_Read_Byte_Rom1,		; 0x080000 - 0x0FFFFF
		dd M68K_Read_Byte_Rom2,		; 0x100000 - 0x17FFFF
		dd M68K_Read_Byte_Rom3,		; 0x180000 - 0x1FFFFF
		dd M68K_Read_Byte_Rom4,		; 0x200000 - 0x27FFFF
		dd M68K_Read_Byte_Rom5,		; 0x280000 - 0x2FFFFF
		dd M68K_Read_Byte_Rom6,		; 0x300000 - 0x37FFFF
		dd M68K_Read_Byte_Rom7,		; 0x380000 - 0x3FFFFF
		dd M68K_Read_Byte_Bios_32X,	; 0x400000 - 0x47FFFF
		dd M68K_Read_Byte_BiosR_32X,; 0x480000 - 0x4FFFFF
		dd M68K_Read_Byte_Bad,		; 0x500000 - 0x57FFFF
		dd M68K_Read_Byte_Bad,		; 0x580000 - 0x5FFFFF
		dd M68K_Read_Byte_Bad,		; 0x600000 - 0x67FFFF
		dd M68K_Read_Byte_Bad,		; 0x680000 - 0x6FFFFF
		dd M68K_Read_Byte_Bad,		; 0x700000 - 0x77FFFF
		dd M68K_Read_Byte_32X_FB0,	; 0x780000 - 0x7FFFFF
		dd M68K_Read_Byte_32X_FB1,	; 0x800000 - 0x87FFFF
		dd M68K_Read_Byte_Rom0,		; 0x880000 - 0x8FFFFF
		dd M68K_Read_Byte_Rom1,		; 0x900000 - 0x97FFFF
		dd M68K_Read_Byte_Rom2,		; 0x980000 - 0x9FFFFF
		dd M68K_Read_Byte_Misc_32X,	; 0xA00000 - 0xA7FFFF
		dd M68K_Read_Byte_Bad,		; 0xA80000 - 0xAFFFFF
		dd M68K_Read_Byte_Bad,		; 0xB00000 - 0xB7FFFF
		dd M68K_Read_Byte_Bad,		; 0xB80000 - 0xBFFFFF
		dd M68K_Read_Byte_VDP,		; 0xC00000 - 0xC7FFFF
		dd M68K_Read_Byte_Bad,		; 0xC80000 - 0xCFFFFF
		dd M68K_Read_Byte_Bad,		; 0xD00000 - 0xD7FFFF
		dd M68K_Read_Byte_Bad,		; 0xD80000 - 0xDFFFFF
		dd M68K_Read_Byte_Ram,		; 0xE00000 - 0xE7FFFF
		dd M68K_Read_Byte_Ram,		; 0xE80000 - 0xEFFFFF
		dd M68K_Read_Byte_Ram,		; 0xF00000 - 0xF7FFFF
		dd M68K_Read_Byte_Ram,		; 0xF80000 - 0xFFFFFF

	DECL _32X_M68K_Read_Word_Table
		dd M68K_Read_Word_Rom0,		; 0x000000 - 0x07FFFF
		dd M68K_Read_Word_Rom1,		; 0x080000 - 0x0FFFFF
		dd M68K_Read_Word_Rom2,		; 0x100000 - 0x17FFFF
		dd M68K_Read_Word_Rom3,		; 0x180000 - 0x1FFFFF
		dd M68K_Read_Word_Rom4,		; 0x200000 - 0x27FFFF
		dd M68K_Read_Word_Rom5,		; 0x280000 - 0x2FFFFF
		dd M68K_Read_Word_Rom6,		; 0x300000 - 0x37FFFF
		dd M68K_Read_Word_Rom7,		; 0x380000 - 0x3FFFFF
		dd M68K_Read_Word_Bios_32X,	; 0x480000 - 0x4FFFFF
		dd M68K_Read_Word_BiosR_32X,; 0x480000 - 0x4FFFFF
		dd M68K_Read_Word_Bad,		; 0x500000 - 0x57FFFF
		dd M68K_Read_Word_Bad,		; 0x580000 - 0x5FFFFF
		dd M68K_Read_Word_Bad,		; 0x600000 - 0x67FFFF
		dd M68K_Read_Word_Bad,		; 0x680000 - 0x6FFFFF
		dd M68K_Read_Word_Bad,		; 0x700000 - 0x77FFFF
		dd M68K_Read_Word_32X_FB0,	; 0x780000 - 0x7FFFFF
		dd M68K_Read_Word_32X_FB1,	; 0x800000 - 0x87FFFF
		dd M68K_Read_Word_Rom0,		; 0x880000 - 0x8FFFFF
		dd M68K_Read_Word_Rom1,		; 0x900000 - 0x97FFFF
		dd M68K_Read_Word_Rom2,		; 0x980000 - 0x9FFFFF
		dd M68K_Read_Word_Misc_32X,	; 0xA00000 - 0xA7FFFF
		dd M68K_Read_Word_Bad,		; 0xA80000 - 0xAFFFFF
		dd M68K_Read_Word_Bad,		; 0xB00000 - 0xB7FFFF
		dd M68K_Read_Word_Bad,		; 0xB80000 - 0xBFFFFF
		dd M68K_Read_Word_VDP,		; 0xC00000 - 0xC7FFFF
		dd M68K_Read_Word_Bad,		; 0xC80000 - 0xCFFFFF
		dd M68K_Read_Word_Bad,		; 0xD00000 - 0xD7FFFF
		dd M68K_Read_Word_Bad,		; 0xD80000 - 0xDFFFFF
		dd M68K_Read_Word_Ram,		; 0xE00000 - 0xE7FFFF
		dd M68K_Read_Word_Ram,		; 0xE80000 - 0xEFFFFF
		dd M68K_Read_Word_Ram,		; 0xF00000 - 0xF7FFFF
		dd M68K_Read_Word_Ram,		; 0xF80000 - 0xFFFFFF

	DECL _32X_M68K_Write_Byte_Table
		dd M68K_Write_Bad,			; 0x000000 - 0x0FFFFF
		dd M68K_Write_Bad,			; 0x100000 - 0x1FFFFF
		dd M68K_Write_Byte_SRAM,	; 0x200000 - 0x2FFFFF
		dd M68K_Write_Bad,			; 0x300000 - 0x3FFFFF
		dd M68K_Write_Bad,			; 0x400000 - 0x4FFFFF
		dd M68K_Write_Bad,			; 0x500000 - 0x5FFFFF
		dd M68K_Write_Bad,			; 0x600000 - 0x6FFFFF
		dd M68K_Write_Byte_32X_FB0,	; 0x700000 - 0x7FFFFF
		dd M68K_Write_Byte_32X_FB1,	; 0x800000 - 0x8FFFFF
		dd M68K_Write_Bad,			; 0x900000 - 0x9FFFFF
		dd M68K_Write_Byte_Misc_32X,; 0xA00000 - 0xAFFFFF
		dd M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd M68K_Write_Byte_VDP,		; 0xC00000 - 0xCFFFFF
		dd M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd M68K_Write_Byte_Ram,		; 0xE00000 - 0xEFFFFF
		dd M68K_Write_Byte_Ram,		; 0xF00000 - 0xFFFFFF

	DECL _32X_M68K_Write_Byte_Cheat_Table
		dd M68K_Write_Byte_ROM,		; 0x000000 - 0x0FFFFF
		dd M68K_Write_Byte_ROM,		; 0x100000 - 0x1FFFFF
		dd M68K_Write_Byte_ROM2,	; 0x200000 - 0x2FFFFF
		dd M68K_Write_Byte_ROM,		; 0x300000 - 0x3FFFFF
		dd M68K_Write_Bad,			; 0x400000 - 0x4FFFFF
		dd M68K_Write_Bad,			; 0x500000 - 0x5FFFFF
		dd M68K_Write_Bad,			; 0x600000 - 0x6FFFFF
		dd M68K_Write_Byte_32X_FB0,	; 0x700000 - 0x7FFFFF
		dd M68K_Write_Byte_32X_FB1,	; 0x800000 - 0x8FFFFF
		dd M68K_Write_Bad,			; 0x900000 - 0x9FFFFF
		dd M68K_Write_Byte_Misc_32X,; 0xA00000 - 0xAFFFFF
		dd M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd M68K_Write_Byte_VDP,		; 0xC00000 - 0xCFFFFF
		dd M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd M68K_Write_Byte_Ram,		; 0xE00000 - 0xEFFFFF
		dd M68K_Write_Byte_Ram,		; 0xF00000 - 0xFFFFFF


	DECL _32X_M68K_Write_Word_Table
		dd M68K_Write_Bad,			; 0x000000 - 0x0FFFFF
		dd M68K_Write_Bad,			; 0x100000 - 0x1FFFFF
		dd M68K_Write_Word_SRAM,	; 0x200000 - 0x2FFFFF
		dd M68K_Write_Bad,			; 0x300000 - 0x3FFFFF
		dd M68K_Write_Bad,			; 0x400000 - 0x4FFFFF
		dd M68K_Write_Bad,			; 0x500000 - 0x5FFFFF
		dd M68K_Write_Bad,			; 0x600000 - 0x6FFFFF
		dd M68K_Write_Word_32X_FB0,	; 0x700000 - 0x7FFFFF
		dd M68K_Write_Word_32X_FB1,	; 0x800000 - 0x8FFFFF
		dd M68K_Write_Bad,			; 0x900000 - 0x9FFFFF
		dd M68K_Write_Word_Misc_32X,; 0xA00000 - 0xAFFFFF
		dd M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd M68K_Write_Word_VDP,		; 0xC00000 - 0xCFFFFF
		dd M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd M68K_Write_Word_Ram,		; 0xE00000 - 0xEFFFFF
		dd M68K_Write_Word_Ram,		; 0xF00000 - 0xFFFFFF


	; Current Main 68000 Jump Table

	DECL M68K_Read_Byte_Table
		times 32	dd M68K_Read_Byte_Bad

	DECL M68K_Read_Word_Table
		times 32	dd M68K_Read_Byte_Bad

	DECL M68K_Write_Byte_Cheat_Table
		times 16	dd M68K_Write_Bad

	DECL M68K_Write_Byte_Table
		times 16	dd M68K_Write_Bad

	DECL M68K_Write_Word_Table
		times 16	dd M68K_Write_Bad

section .bss align=64

	extern Ram_Z80
	extern Ram_Prg
	extern Ram_Word_2M
	extern Ram_Word_1M
	extern Ram_Word_State

	extern S68K_Mem_WP
	extern Int_Mask_S68K
	extern Bank_Z80

	extern M_SH2
	extern S_SH2
	extern M_Z80

	extern _32X_Comm
	extern _32X_ADEN
	extern _32X_RES
	extern _32X_FM
	extern _32X_RV
	extern _32X_DREQ_ST
	extern _32X_DREQ_SRC
	extern _32X_DREQ_DST
	extern _32X_DREQ_LEN
	extern _32X_FIFO_A
	extern _32X_FIFO_B
	extern _32X_FIFO_Block
	extern _32X_FIFO_Write
	extern _32X_FIFO_Read
	extern _32X_MINT
	extern _32X_SINT
	extern _32X_Palette_16B
	extern _32X_Palette_32B
	extern _32X_VDP_Ram
	extern _32X_VDP_CRam
	extern _32X_VDP_CRam_Ajusted
	extern _32X_VDP_CRam_Ajusted32
	extern _32X_VDP

	extern PWM_FIFO_R
	extern PWM_FIFO_L
	extern PWM_FULL_TAB
	extern PWM_RP_R
	extern PWM_WP_R
	extern PWM_RP_L
	extern PWM_WP_L
	extern PWM_Mode

	extern PWM_Cycle_Tmp
	extern PWM_Int_Tmp
	extern PWM_FIFO_L_Tmp
	extern PWM_FIFO_R_Tmp

	extern COMM.Flag
	extern COMM.Command
	extern COMM.Status

	extern CDC.RS0
	extern CDC.RS1
	extern CDC.Host_Data
	extern CDC.DMA_Adr
	extern CDC.Stop_Watch

	struc vx
		.Mode		resd 1
		.State		resd 1
		.AF_Data	resd 1
		.AF_St		resd 1
		.AF_Len		resd 1
		.AF_Line	resd 1
	endstruc

	DECL Ram_68k
	resb 64 * 1024

	DECL Rom_Data
	resb 8 * 1024 * 1024

	DECL SRAM
	resb 64 * 1024

	DECL Ram_Backup_Ex
	resb 64 * 1024

	DECL Genesis_Rom
	resb 2 * 1024

	DECL _32X_Genesis_Rom
	resb 256

	DECL Rom_Size
	resd 1

	DECL SRAM_Start
	resd 1
	DECL SRAM_End
	resd 1
	DECL SRAM_ON
	resd 1
	DECL SRAM_Write
	resd 1
	DECL SRAM_Custom
	resd 1
	DECL BRAM_Ex_State
	resd 1
	DECL BRAM_Ex_Size
	resd 1

	ALIGNB64

	DECL Z80_M68K_Cycle_Tab
	resd 512

	DECL S68K_State
	resd 1
	DECL Z80_State
	resd 1
	DECL Last_BUS_REQ_Cnt
	resd 1
	DECL Last_BUS_REQ_St
	resd 1
	DECL Bank_M68K
	resd 1
	DECL Bank_SH2
	resd 1
	DECL Fake_Fetch
	resd 1

	DECL Game_Mode
	resd 1
	DECL CPU_Mode
	resd 1
	DECL Gen_Mode
	resd 1
	DECL Gen_Version
	resd 1

	DECL CPL_M68K
	resd 1
	DECL CPL_S68K
	resd 1
	DECL CPL_Z80
	resd 1
	DECL Cycles_S68K
	resd 1
	DECL Cycles_M68K
	resd 1
	DECL Cycles_Z80
	resd 1


section .text align=64

	extern Z80_ReadB_Table
	extern Z80_ReadW_Table
	extern Z80_WriteB_Table
	extern Z80_WriteW_Table

	extern Read_VDP_Data
	extern Read_VDP_Status
	extern Read_VDP_V_Counter
	extern Read_VDP_H_Counter
	extern Write_Byte_VDP_Data
	extern Write_Word_VDP_Data
	extern Write_VDP_Ctrl
	extern RD_Controller_1
	extern RD_Controller_2
	extern WR_Controller_1
	extern WR_Controller_2
	extern SH2_Reset
	extern SH2_Interrupt
	extern SH2_DMA0_Request
	extern main68k_readOdometer
	extern sub68k_reset
	extern sub68k_interrupt
	extern z80_Reset
	extern z80_Exec
	extern z80_Set_Odo
	extern M68K_Set_Prg_Ram
	extern MS68K_Set_Word_Ram
	extern M68K_Set_32X_Rom_Bank
	extern YM2612_Write
	extern YM2612_Read
	extern YM2612_Reset
	extern PSG_Write
	extern Read_CDC_Host_MAIN
	extern M68K_32X_Mode
	extern M68K_Set_32X_Rom_Bank
	extern _32X_Set_FB
	extern PWM_Set_Cycle
	extern PWM_Set_Int
	extern Fix_Codes


	;void Init_Memory_M68K(int System_ID)
	DECL Init_Memory_M68K

		push rax
		push rbx
		mov ebx, 15
		cmp byte [esp + 12], 1
		ja near .SegaCD
		je near ._32X
		jmp short .Genesis

	ALIGN4

	.Genesis
		mov eax, [Genesis_M68K_Read_Byte_Table + ebx * 8]
		mov [M68K_Read_Byte_Table + ebx * 8], eax
		mov eax, [Genesis_M68K_Read_Byte_Table + ebx * 8 + 4]
		mov [M68K_Read_Byte_Table + ebx * 8 + 4], eax
		mov eax, [Genesis_M68K_Read_Word_Table + ebx * 8]
		mov [M68K_Read_Word_Table + ebx * 8], eax
		mov eax, [Genesis_M68K_Read_Word_Table + ebx * 8 + 4]
		mov [M68K_Read_Word_Table + ebx * 8 + 4], eax

		mov eax, [Genesis_M68K_Write_Byte_Table + ebx * 4]
		mov [M68K_Write_Byte_Table + ebx * 4], eax
		mov eax, [Genesis_M68K_Write_Byte_Cheat_Table + ebx * 4]
		mov [M68K_Write_Byte_Cheat_Table + ebx * 4], eax
		mov eax, [Genesis_M68K_Write_Word_Table + ebx * 4]
		mov [M68K_Write_Word_Table + ebx * 4], eax

		dec ebx
		jns short .Genesis

		pop rbx
		pop rax
		ret

	ALIGN32

	._32X
		mov eax, [_32X_M68K_Read_Byte_Table + ebx * 8]
		mov [M68K_Read_Byte_Table + ebx * 8], eax
		mov eax, [_32X_M68K_Read_Byte_Table + ebx * 8 + 4]
		mov [M68K_Read_Byte_Table + ebx * 8 + 4], eax
		mov eax, [_32X_M68K_Read_Word_Table + ebx * 8]
		mov [M68K_Read_Word_Table + ebx * 8], eax
		mov eax, [_32X_M68K_Read_Word_Table + ebx * 8 + 4]
		mov [M68K_Read_Word_Table + ebx * 8 + 4], eax

		mov eax, [_32X_M68K_Write_Byte_Table + ebx * 4]
		mov [M68K_Write_Byte_Table + ebx * 4], eax
;		mov eax, [_32X_M68K_Write_Byte_Cheat_Table + ebx * 4]
		mov [M68K_Write_Byte_Cheat_Table + ebx * 4], eax
			mov eax, [_32X_M68K_Write_Word_Table + ebx * 4]
		mov [M68K_Write_Word_Table + ebx * 4], eax

		dec ebx
		jns short ._32X

		mov eax, [_32X_M68K_Read_Byte_Table + 6 * 8]
		mov [M68K_Read_Byte_Table + 8 * 8 - 4], eax
		mov eax, [_32X_M68K_Read_Word_Table + 6 * 8]
		mov [M68K_Read_Word_Table + 8 * 8 - 4], eax

		mov eax, [_32X_M68K_Write_Byte_Table + 6 * 4]
		mov [M68K_Write_Byte_Table + 8 * 4 + 4], eax
		mov eax, [_32X_M68K_Write_Word_Table + 6 * 4]
		mov [M68K_Write_Word_Table + 8 * 4 + 4], eax

		pop rbx
		pop rax
		ret

	ALIGN32

	.SegaCD
		mov eax, [SegaCD_M68K_Read_Byte_Table + ebx * 8]
		mov [M68K_Read_Byte_Table + ebx * 8], eax
		mov eax, [SegaCD_M68K_Read_Byte_Table + ebx * 8 + 4]
		mov [M68K_Read_Byte_Table + ebx * 8 + 4], eax
		mov eax, [SegaCD_M68K_Read_Word_Table + ebx * 8]
		mov [M68K_Read_Word_Table + ebx * 8], eax
		mov eax, [SegaCD_M68K_Read_Word_Table + ebx * 8 + 4]
		mov [M68K_Read_Word_Table + ebx * 8 + 4], eax

		mov eax, [SegaCD_M68K_Write_Byte_Table + ebx * 4]
		mov [M68K_Write_Byte_Table + ebx * 4], eax
;		mov eax, [SegaCD_M68K_Write_Byte_Cheat_Table + ebx * 4]
		mov [M68K_Write_Byte_Cheat_Table + ebx * 4], eax
		mov eax, [SegaCD_M68K_Write_Word_Table + ebx * 4]
		mov [M68K_Write_Word_Table + ebx * 4], eax

		dec ebx
		jns short .SegaCD

		pop rbx
		pop rax
		ret

	ALIGN64

	;unsigned char M68K_RB(unsigned int Adr)
	DECL M68K_RB

		mov eax, [esp + 4]
		push rbx

		mov ebx, eax
		and eax, 0xF80000
		shr eax, 17
		and ebx, 0xFFFFFF
		jmp [M68K_Read_Byte_Table + eax]

	ALIGN64

	;unsigned short M68K_RW(unsigned int Adr)
	DECL M68K_RW

		mov eax, [esp + 4]
		push rbx

		mov ebx, eax
		and eax, 0xF80000
		shr eax, 17
		and ebx, 0xFFFFFF
		jmp [M68K_Read_Word_Table + eax]

	ALIGN64

	;void M68K_WBC(unsigned int Adr, unsigned char Data)
	DECL M68K_WBC

		push rbx
		push rcx

		mov ecx, [esp + 12]
		mov eax, [esp + 16]
		mov [hook_pc], dword 0
		mov [hook_address],ecx
		mov [hook_value],eax
		call hook_write_byte
		mov ecx, [esp + 12]
		mov eax, [esp + 16]
		mov ebx, ecx
		and ecx, 0xF00000
		and eax, 0xFF
		shr ecx, 18
		and ebx, 0xFFFFFF
		call [M68K_Write_Byte_Cheat_Table + ecx]

		pop rcx
		pop rbx
		ret

	ALIGN64

	;void M68K_WB(unsigned int Adr, unsigned char Data)
	DECL M68K_WB

		push rbx
		push rcx

		mov ecx, [esp + 12]
		mov eax, [esp + 16]
		mov ebx, ecx
		and ecx, 0xF00000
		and eax, 0xFF
		shr ecx, 18
		and ebx, 0xFFFFFF
		call [M68K_Write_Byte_Table + ecx]
		mov ecx, [esp + 12]
		push dword 1
		and ecx, 0xFFFFFF
		push rcx
		call Fix_Codes
		add esp, byte 8

		pop rcx
		pop rbx
		ret

	ALIGN64

	;void M68K_WW(unsigned int Adr, unsigned short Data)
	DECL M68K_WW

		push rbx
		push rcx

		mov ecx, [esp + 12]
		mov eax, [esp + 16]
		mov ebx, ecx
		and ecx, 0xF00000
		and eax, 0xFFFF
		shr ecx, 18
		and ebx, 0xFFFFFF
		call [M68K_Write_Word_Table + ecx]

		mov ecx, [esp + 12]
		push dword 2
		and ecx, 0xFFFFFF
		push rcx
		call Fix_Codes
		add esp, byte 8

		pop rcx
		pop rbx
		ret


	;******** Read Byte Proc

	ALIGN32

	global M68K_Read_Byte_Bad

	M68K_Read_Byte_Bad:
		xor al, al
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Rom0:
		and ebx, 0x7FFFF
		xor ebx, 1
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Rom1:
		and ebx, 0x7FFFF
		xor ebx, 0x080001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Rom2:
		and ebx, 0x7FFFF
		xor ebx, 0x100001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Rom3:
		and ebx, 0x7FFFF
		xor ebx, 0x180001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Rom4:
		test dword [SRAM_ON], 1
		jz short .Rom
		cmp ebx, [SRAM_Start]
		jb short .Rom
		cmp ebx, [SRAM_End]
		ja short .Rom

		test byte [SRAM_Custom], 1
		jnz short .Custom_SRAM

		sub ebx, [SRAM_Start]
		mov al, [SRAM + ebx]			; no byte swapped
		pop rbx
		ret

	ALIGN4

	.Rom
		and ebx, 0x7FFFF
		xor ebx, 0x200001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN4

	.Custom_SRAM
		mov al, 0
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Rom5:
		and ebx, 0x7FFFF
		xor ebx, 0x280001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Rom6:
		and ebx, 0x7FFFF
		xor ebx, 0x300001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Rom7:
		and ebx, 0x7FFFF
		xor ebx, 0x380001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Rom8:
		and ebx, 0x7FFFF
		xor ebx, 0x400001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Rom9:
		and ebx, 0x7FFFF
		xor ebx, 0x480001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_RomA:
		and ebx, 0x7FFFF
		xor ebx, 0x500001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_RomB:
		and ebx, 0x7FFFF
		xor ebx, 0x580001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_RomC:
		and ebx, 0x7FFFF
		xor ebx, 0x600001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_RomD:
		and ebx, 0x7FFFF
		xor ebx, 0x680001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_RomE:
		and ebx, 0x7FFFF
		xor ebx, 0x700001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_RomF:
		and ebx, 0x7FFFF
		xor ebx, 0x780001
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Ram:
		and ebx, 0xFFFF
		xor ebx, 1
		mov al, [Ram_68k + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_Misc:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_mem

		test byte [Z80_State], 6
		jnz short .bad

		push rcx
		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		call [Z80_ReadB_Table + ebx]
		pop rdx
		pop rcx
		pop rbx
		ret

	ALIGN4

	.bad
		xor al, al
		pop rbx
		ret

	ALIGN4

	.no_Z80_mem
		cmp ebx, 0xA11100
		jne short .no_busreq

		test byte [Z80_State], 2
		jnz short .z80_on

	.z80_off
		call main68k_readOdometer
		sub eax, [Last_BUS_REQ_Cnt]
		cmp eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja short .bus_taken

		mov al, [Last_BUS_REQ_St]
		pop rbx
		or al, 0x80
		ret

	ALIGN4

	.bus_taken
		mov al, 0x80
		pop rbx
		ret

	ALIGN4

	.z80_on
		mov al, 0x81
		pop rbx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA1000D
		ja short .bad

		and ebx, 0x00000E
		jmp [.Table_IO_RB + ebx * 2]

	ALIGN4

	.Table_IO_RB
		dd .MD_Spec, .Pad_1, .Pad_2, .Ser
		dd .CT_Pad_1, .CT_Pad_2, .CT_Ser, .bad

	ALIGN4

	.MD_Spec
		mov al, [Game_Mode]
		add al, al
		or al, [CPU_Mode]
		shl al, 6
		pop rbx
		or al, [Gen_Version]
		ret

	ALIGN4

	.Pad_1
		call RD_Controller_1
		pop rbx
		ret

	ALIGN4

	.Pad_2
		call RD_Controller_2
		pop rbx
		ret

	ALIGN4

	.Ser
		mov al, 0xFF
		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov al, [Controller_1_COM]
		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov al, [Controller_2_COM]
		pop rbx
		ret

	ALIGN4

	.CT_Ser
		xor al, al
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_VDP:
		cmp ebx, 0xC00004
		jb short .bad
		cmp ebx, 0xC00008
		jb short .vdp_status
		cmp ebx, 0xC00009
		ja short .bad

	.vdp_counter
		test ebx, 1
		jnz short .vdp_h_counter

	.vdp_v_counter
		call Read_VDP_V_Counter
		pop rbx
		ret

	ALIGN4

	.vdp_h_counter
		call Read_VDP_H_Counter
		pop rbx
		ret

	ALIGN4

	.bad
		xor al, al
		pop rbx
		ret

	ALIGN4

	.vdp_status
		call Read_VDP_Status
		test ebx, 1
		jnz .no_swap_status
		mov al, ah					; on lit que le poids fort

	.no_swap_status
		pop rbx
		ret


	;******** Read Word Proc

	ALIGN32

	global M68K_Read_Word_Bad

	M68K_Read_Word_Bad:
		xor ax, ax
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Rom0:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Rom1:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x080000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Rom2:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x100000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Rom3:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x180000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Rom4:
		test dword [SRAM_ON], 1
		jz short .Rom
		cmp ebx, [SRAM_Start]
		jb short .Rom
		cmp ebx, [SRAM_End]
		ja short .Rom

		test byte [SRAM_Custom], 1
		jnz short .Custom_SRAM

		sub ebx, [SRAM_Start]
		mov ax, [SRAM + ebx]		; no byte swapped
		rol ax, 8
		pop rbx
		ret

	ALIGN4

	.Rom
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x200000]
		pop rbx
		ret

	ALIGN4

	.Custom_SRAM
		mov ax, 0
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Rom5:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x280000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Rom6:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x300000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Rom7:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x380000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Rom8:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x400000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Rom9:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x480000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_RomA:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x500000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_RomB:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x580000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_RomC:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x600000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_RomD:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x680000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_RomE:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x700000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_RomF:
		and ebx, 0x7FFFF
		mov ax, [Rom_Data + ebx + 0x780000]
		pop rbx
		ret

		;xor ax, ax
		;pop rbx
		;ret

	ALIGN32

	M68K_Read_Word_Ram:
		and ebx, 0xFFFF
		mov ax, [Ram_68k + ebx]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_Misc:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_ram

		test byte [Z80_State], 6
		jnz near .bad

		push rcx
		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		call [Z80_ReadB_Table + ebx]
		pop rdx
		pop rcx
		pop rbx
		ret

	ALIGN4

	.no_Z80_ram
		cmp ebx, 0xA11100
		jne short .no_busreq

		test byte [Z80_State], 2
		jnz short .z80_on

	.z80_off
		call main68k_readOdometer
		sub eax, [Last_BUS_REQ_Cnt]
		cmp eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja short .bus_taken

		mov al, [Fake_Fetch]
		mov ah, [Last_BUS_REQ_St]
		xor al, 0xFF
		add ah, 0x80
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		pop rbx
		ret

	ALIGN4

	.bus_taken
		mov al, [Fake_Fetch]
		mov ah, 0x80
		xor al, 0xFF
		pop rbx
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		ret

	ALIGN4

	.z80_on
		mov al, [Fake_Fetch]
		mov ah, 0x81
		xor al, 0xFF
		pop rbx
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA1000D
		ja short .bad

		and ebx, 0x00000E
		jmp [.Table_IO_RW + ebx * 2]

	ALIGN4

	.Table_IO_RW
		dd .MD_Spec, .Pad_1, .Pad_2, .Ser
		dd .CT_Pad_1, .CT_Pad_2, .CT_Ser, .bad

	ALIGN4

	.MD_Spec
		mov eax, [Game_Mode]
		add eax, eax
		or eax, [CPU_Mode]
		shl eax, 6
		pop rbx
		or eax, [Gen_Version]									; on recupere les infos hardware de la machine
		ret

	ALIGN4

	.Pad_1
		call RD_Controller_1
		pop rbx
		ret

	ALIGN4

	.Pad_2
		call RD_Controller_2
		pop rbx
		ret

	ALIGN4

	.Ser
		mov ax, 0xFF00
		pop rbx
		ret

	ALIGN4

	.bad
		xor eax, eax
		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov eax, [Controller_1_COM]
		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov eax, [Controller_2_COM]
		pop rbx
		ret

	ALIGN4

	.CT_Ser
		xor eax, eax
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_VDP:
		cmp ebx, 0xC00003
		ja short .no_vdp_data

		call Read_VDP_Data
		pop rbx
		ret

	ALIGN4

	.no_vdp_data
		cmp ebx, 0xC00007
		ja .no_vdp_status

		call Read_VDP_Status
		pop rbx
		ret

	ALIGN4

	.no_vdp_status
		cmp ebx, 0xC00009
		ja short .bad
		call Read_VDP_V_Counter
		mov bl, al
		call Read_VDP_H_Counter
		mov ah, bl
		pop rbx
		ret

	ALIGN4

	.bad
		xor eax, eax
		pop rbx
		ret


	;******** Write Byte Proc

	ALIGN32

	global M68K_Write_Bad

	M68K_Write_Bad:
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	M68K_Write_Byte_ROM:
		shl ecx, 18
		add ecx, 1
		and ebx, 0xFFFFF
		xor ebx, ecx
		mov [Rom_Data + ebx], al
		ret

	M68K_Write_Byte_ROM2:
		cmp ebx, 0x280000
		jae short M68K_Write_Byte_ROM
		test dword [SRAM_ON], 1
		jz short M68K_Write_Byte_ROM
		cmp ebx, [SRAM_Start]
		jb short M68K_Write_Byte_ROM
		cmp ebx, [SRAM_End]
		ja short M68K_Write_Byte_ROM
	M68K_Write_Byte_SRAM:
		test dword [SRAM_ON], 1
		jz short M68K_Write_Bad
		test dword [SRAM_Write], 1
		jz short M68K_Write_Bad
		cmp ebx, [SRAM_Start]
		jb short  M68K_Write_Bad
		cmp ebx, [SRAM_End]
		ja near M68K_Write_Bad

		sub ebx, [SRAM_Start]
		mov [SRAM + ebx], al
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	M68K_Write_Byte_Ram:
		and ebx, 0xFFFF
		xor ebx, 1
		mov [Ram_68k + ebx], al
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	M68K_Write_Byte_Misc:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_mem

		test byte [Z80_State], 6
		jnz short .bad

		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		mov edx, eax
		call [Z80_WriteB_Table + ebx]
		pop rdx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_Z80_mem
		cmp ebx, 0xA11100
		jne near .no_busreq

		xor ecx, ecx
		mov ah, [Z80_State]
		mov dword [Controller_1_Counter], ecx
		test al, 1
		mov dword [Controller_1_Delay], ecx
		mov dword [Controller_2_Counter], ecx
		mov dword [Controller_2_Delay], ecx
		jnz short .deactivated

		test ah, 2
		jnz short .already_actived

		or ah, 2
		push rdx
		mov [Z80_State], ah
		mov ebx, [Cycles_M68K]
		call main68k_readOdometer
		sub ebx, eax
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Set_Odo
		pop rdx

	.already_actived
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.deactivated
		call main68k_readOdometer
		mov cl, [Z80_State]
		mov [Last_BUS_REQ_Cnt], eax
		test cl, 2
		setnz [Last_BUS_REQ_St]
		jz short .already_deactivated

		push rdx
		mov ebx, [Cycles_M68K]
		and cl, ~2
		sub ebx, eax
		mov [Z80_State], cl
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Exec
		pop rdx

	.already_deactivated
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA11200
		jne short .no_reset_z80

		test al, 1
		jnz short .no_reset

		push rdx
		mov ecx, M_Z80
		call z80_Reset
		or byte [Z80_State], 4
		call YM2612_Reset
		pop rdx
;		pop rcx
;		pop rbx
		ret

	.no_reset
		and byte [Z80_State], ~4
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_reset_z80
		cmp ebx, 0xA1000D
		ja short .no_ctrl_io

		and ebx, 0x00000E
		jmp [.Table_IO_WB + ebx * 2]

	ALIGN4

	.Table_IO_WB
		dd .bad, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.Pad_1
		push rax
		call WR_Controller_1
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Pad_2
		push rax
		call WR_Controller_2
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov [Controller_1_COM], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov [Controller_2_COM], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_ctrl_io
		cmp ebx, 0xA130F1
		jb near .bad
		jne short .no_sram_ctrl

		test al, 1
		setnz [SRAM_ON]
		test al, 2
		setz [SRAM_Write]
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_sram_ctrl
		cmp ebx, 0xA130FF
		ja near .bad

		and ebx, 0xF
		and eax, 0x1F
		shr ebx, 1
		mov ecx, [Genesis_M68K_Read_Byte_Table + eax * 4]
		mov [M68K_Read_Byte_Table + ebx * 4], ecx
		mov ecx, [Genesis_M68K_Read_Word_Table + eax * 4]
		mov [M68K_Read_Word_Table + ebx * 4], ecx

;		pop rcx
;		pop rbx
		ret

	ALIGN32

	M68K_Write_Byte_VDP:
		cmp ebx, 0xC00003
		ja short .no_data_port

		push rax
		call Write_Byte_VDP_Data
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_data_port
		cmp ebx, 0xC00011
		jne .bad

		push rax
		call PSG_Write
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
;		pop rcx
;		pop rbx
		ret


	;******** Write Word Proc

	ALIGN32

	M68K_Write_Word_SRAM:
		test dword [SRAM_ON], 1
		jz short .bad
		test dword [SRAM_Write], 1
		jz short .bad
		cmp ebx, [SRAM_Start]
		jb short .bad
		cmp ebx, [SRAM_End]
		ja short .bad

		rol ax, 8
		sub ebx, [SRAM_Start]
		mov [SRAM + ebx], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	M68K_Write_Word_Ram:
		and ebx, 0xFFFF
		mov [Ram_68k + ebx], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	M68K_Write_Word_Misc:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_ram

		test byte [Z80_State], 6
		jnz near .bad

		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		mov dh, al
		shr ebx, 10
		mov dl, al
		call [Z80_WriteB_Table + ebx]
		pop rdx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_Z80_ram
		cmp ebx, 0xA11100
		jne near .no_busreq

		xor ecx, ecx
		mov al, [Z80_State]
		mov dword [Controller_1_Counter], ecx
		test ah, 1
		mov dword [Controller_1_Delay], ecx
		mov dword [Controller_2_Counter], ecx
		mov dword [Controller_2_Delay], ecx
		jnz short .deactivated

		test al, 2
		jnz short .already_actived

		or al, 2
		push rdx
		mov [Z80_State], al
		mov ebx, [Cycles_M68K]
		call main68k_readOdometer
		sub ebx, eax
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Set_Odo
		pop rdx

	.already_actived
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.deactivated
		call main68k_readOdometer
		mov cl, [Z80_State]
		mov [Last_BUS_REQ_Cnt], eax
		test cl, 2
		setnz [Last_BUS_REQ_St]
		jz short .already_deactivated

		push rdx
		mov ebx, [Cycles_M68K]
		and cl, ~2
		sub ebx, eax
		mov [Z80_State], cl
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Exec
		pop rdx

	.already_deactivated
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA11200
		jne short .no_reset_z80

		test ah, 1
		jnz short .no_reset

		push rdx
		mov ecx, M_Z80
		call z80_Reset
		or byte [Z80_State], 4
		call YM2612_Reset
		pop rdx
;		pop rcx
;		pop rbx
		ret

	.no_reset
		and byte [Z80_State], ~4
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_reset_z80
		cmp ebx, 0xA1000D
		ja short .no_ctrl_io

		and ebx, 0x00000E
		jmp [.Table_IO_WW + ebx * 2]

	ALIGN4

	.Table_IO_WW
		dd .bad, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.Pad_1
		push rax
		call WR_Controller_1
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Pad_2
		push rax
		call WR_Controller_2
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov [Controller_1_COM], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov [Controller_2_COM], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_ctrl_io
		cmp ebx, 0xA130F0
		jb short .bad
		jne short .no_sram_ctrl

		test ax, 0x1
		setnz [SRAM_ON]
		test ax, 0x2
		setz [SRAM_Write]
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_sram_ctrl
		cmp ebx, 0xA130FF
		ja short .bad

		mov al, ah
		and ebx, 0xF
		and eax, 0x1F
		shr ebx, 1
		mov ecx, [Genesis_M68K_Read_Byte_Table + eax * 4]
		mov [M68K_Read_Byte_Table + ebx * 4], ecx
		mov ecx, [Genesis_M68K_Read_Word_Table + eax * 4]
		mov [M68K_Read_Word_Table + ebx * 4], ecx

;		pop rcx
;		pop rbx
		ret

	ALIGN32

	M68K_Write_Word_VDP:
		cmp ebx, 0xC00003
		ja short .no_data_port

		push rax
		call Write_Word_VDP_Data
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_data_port
		cmp ebx, 0xC00007
		ja short .bad

		push rax
		call Write_VDP_Ctrl
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
;		pop rcx
;		pop rbx
		ret




	; SegaCD extended Read Byte
	; *******************************************

	ALIGN32

	M68K_Read_Byte_Bios_CD:
		cmp ebx, 0x1FFFF
		ja short .Bank_RAM

		xor ebx, 1
		mov al, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN4

	.Bank_RAM
		cmp ebx, 0x3FFFF
		ja near M68K_Read_Byte_Bad

		add ebx, [Bank_M68K]
		cmp byte [S68K_State], 1			; BUS available ?
		je near M68K_Read_Byte_Bad

		xor ebx, 1
		mov al, [Ram_Prg + ebx - 0x20000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_WRam:
		cmp ebx, 0x23FFFF
		mov eax, [Ram_Word_State]
		ja short .bad
		and eax, 0x3
		jmp [.Table_Word_Ram + eax * 4]

	ALIGN4

	.Table_Word_Ram
;		dd .Word_Ram_2M, .bad
		dd .Word_Ram_2M, .Word_Ram_2M
		dd .Word_Ram_1M_0, .Word_Ram_1M_1

	ALIGN4

	.Word_Ram_2M
		xor ebx, 1
		mov al, [Ram_Word_2M + ebx - 0x200000]
		pop rbx
		ret

	ALIGN4

	.Word_Ram_1M_0
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_0

		xor ebx, 1
		mov al, [Ram_Word_1M + ebx - 0x200000]
		pop rbx
		ret

	ALIGN4

	.Word_Ram_1M_1
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_1

		xor ebx, 1
		mov al, [Ram_Word_1M + ebx - 0x200000 + 0x20000]
		pop rbx
		ret

	ALIGN4

	.bad
		mov al, 0
		pop rbx
		ret

	ALIGN4

	.Cell_Arranged_0
		shr ebx, 1
		mov eax, 0
		mov bx, [Cell_Conv_Tab + ebx * 2 - 0x220000]
		adc eax, 0
		and ebx, 0xFFFF
		mov al, [Ram_Word_1M + ebx * 2 + eax]
		pop rbx
		ret

	ALIGN4

	.Cell_Arranged_1
		shr ebx, 1
		mov eax, 0
		mov bx, [Cell_Conv_Tab + ebx * 2 - 0x220000]
		adc eax, 0
		and ebx, 0xFFFF
		mov al, [Ram_Word_1M + ebx * 2 + eax + 0x20000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Byte_BRAM_L:
		cmp ebx, 0x400001
		mov al, 0
		jne short .bad

		mov al, [BRAM_Ex_Size]

	.bad
		pop rbx
		ret


	ALIGN32

	M68K_Read_Byte_BRAM:
		cmp ebx, 0x61FFFF
		mov al, 0
		ja short .bad

		test word [BRAM_Ex_State], 0x100
		jz short .bad

		and ebx, 0x1FFFF
		shr ebx, 1
		mov al, [Ram_Backup_Ex + ebx]

	.bad
		pop rbx
		ret


	ALIGN32

	M68K_Read_Byte_BRAM_W:
		cmp ebx, 0x7FFFFF
		mov al, 0
		jne short .bad

		mov al, [BRAM_Ex_State]

	.bad
		pop rbx
		ret


	ALIGN32

	M68K_Read_Byte_Misc_CD:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_mem

		test byte [Z80_State], 6
		jnz near .bad

		push rcx
		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		call [Z80_ReadB_Table + ebx]
		pop rdx
		pop rcx
		pop rbx
		ret

	ALIGN4

	.no_Z80_mem
		cmp ebx, 0xA11100
		jne short .no_busreq

		test byte [Z80_State], 2
		jnz short .z80_on

	.z80_off
		call main68k_readOdometer
		sub eax, [Last_BUS_REQ_Cnt]
		cmp eax, CYCLE_FOR_TAKE_Z80_BUS_SEGACD
		ja short .bus_taken

		mov al, [Last_BUS_REQ_St]
		pop rbx
		or al, 0x80
		ret

	ALIGN4

	.bus_taken
		mov al, 0x80
		pop rbx
		ret

	ALIGN4

	.z80_on
		mov al, 0x81
		pop rbx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA1000D
		ja short .CD_Reg

		and ebx, 0x00000E
		jmp [.Table_IO_RB + ebx * 2]

	ALIGN4

	.Table_IO_RB
		dd .MD_Spec, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.bad
		mov al, 0
		pop rbx
		ret

	ALIGN4

	.MD_Spec
		mov al, [Game_Mode]
		add al, al
		or al, [CPU_Mode]
		shl al, 6
		pop rbx
		or al, [Gen_Version]
		ret

	ALIGN4

	.Pad_1
		call RD_Controller_1
		pop rbx
		ret

	ALIGN4

	.Pad_2
		call RD_Controller_2
		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov al, [Controller_1_COM]
		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov al, [Controller_2_COM]
		pop rbx
		ret

	ALIGN4

	.CD_Reg
		cmp ebx, 0xA12000
		jb short .bad
		cmp ebx, 0xA1202F
		ja short .bad

		and ebx, 0x3F
		jmp [.Table_Extended_IO + ebx * 4]

	ALIGN4

	.Table_Extended_IO
		dd .S68K_Ctrl_H, .S68K_Ctrl_L, .Memory_Ctrl_H, .Memory_Ctrl_L
		dd .CDC_Mode_H, .CDC_Mode_L, .HINT_Vector_H, .HINT_Vector_L
		dd .CDC_Host_Data_H, .CDC_Host_Data_L, .Unknow, .Unknow
		dd .Stop_Watch_H, .Stop_Watch_L, .Com_Flag_H, .Com_Flag_L
		dd .Com_D0_H, .Com_D0_L, .Com_D1_H, .Com_D1_L
		dd .Com_D2_H, .Com_D2_L, .Com_D3_H, .Com_D3_L
		dd .Com_D4_H, .Com_D4_L, .Com_D5_H, .Com_D5_L
		dd .Com_D6_H, .Com_D6_L, .Com_D7_H, .Com_D7_L
		dd .Com_S0_H, .Com_S0_L, .Com_S1_H, .Com_S1_L
		dd .Com_S2_H, .Com_S2_L, .Com_S3_H, .Com_S3_L
		dd .Com_S4_H, .Com_S4_L, .Com_S5_H, .Com_S5_L
		dd .Com_S6_H, .Com_S6_L, .Com_S7_H, .Com_S7_L

	ALIGN4

	.S68K_Ctrl_L
		mov al, [S68K_State]
		pop rbx
		ret

	ALIGN4

	.S68K_Ctrl_H
		mov al, [Int_Mask_S68K]
		and al, 4
		shl al, 5
		pop rbx
		ret

	ALIGN4

	.Memory_Ctrl_L
		mov eax, [Bank_M68K]
		mov ebx, [Ram_Word_State]
		shr eax, 11
		and ebx, 3
		or al, [Memory_Control_Status + ebx]
		pop rbx
		ret

	ALIGN4

	.Memory_Ctrl_H
		mov al, [S68K_Mem_WP]
		pop rbx
		ret

	ALIGN4

	.CDC_Mode_H
		mov al, [CDC.RS0 + 1]
		pop rbx
		ret

	ALIGN4

	.CDC_Mode_L
		mov al, 0
		pop rbx
		ret

	ALIGN4

	.HINT_Vector_H
		mov al, [Rom_Data + 0x73]
		pop rbx
		ret

	ALIGN4

	.HINT_Vector_L
		mov al, [Rom_Data + 0x72]
		pop rbx
		ret

	ALIGN4

	.CDC_Host_Data_H
		xor al, al
		pop rbx
		ret

	ALIGN4

	.CDC_Host_Data_L
		xor al, al
		pop rbx
		ret

	ALIGN4

	.Unknow
		mov al, 0
		pop rbx
		ret

	ALIGN4

	.Stop_Watch_H
		mov al, [CDC.Stop_Watch + 3]
		pop rbx
		ret

	ALIGN4

	.Stop_Watch_L
		mov al, [CDC.Stop_Watch + 2]
		pop rbx
		ret

	ALIGN4

	.Com_Flag_H
		mov al, [COMM.Flag + 1]
		pop rbx
		ret

	ALIGN4

	.Com_Flag_L
		mov al, [COMM.Flag]
		pop rbx
		ret

	ALIGN4

	.Com_D0_H
	.Com_D0_L
	.Com_D1_H
	.Com_D1_L
	.Com_D2_H
	.Com_D2_L
	.Com_D3_H
	.Com_D3_L
	.Com_D4_H
	.Com_D4_L
	.Com_D5_H
	.Com_D5_L
	.Com_D6_H
	.Com_D6_L
	.Com_D7_H
	.Com_D7_L
		xor ebx, 1
		mov al, [COMM.Command + ebx - 0x10]
		pop rbx
		ret

	ALIGN4

	.Com_S0_H
	.Com_S0_L
	.Com_S1_H
	.Com_S1_L
	.Com_S2_H
	.Com_S2_L
	.Com_S3_H
	.Com_S3_L
	.Com_S4_H
	.Com_S4_L
	.Com_S5_H
	.Com_S5_L
	.Com_S6_H
	.Com_S6_L
	.Com_S7_H
	.Com_S7_L
		xor ebx, 1
		mov al, [COMM.Status + ebx - 0x20]
		pop rbx
		ret


	; SegaCD extended Read Word
	; *******************************************

	ALIGN32

	M68K_Read_Word_Bios_CD:
		cmp ebx, 0x1FFFF
		ja short .Bank_RAM

		mov ax, [Rom_Data + ebx]
		pop rbx
		ret

	ALIGN4

	.Bank_RAM
		cmp ebx, 0x3FFFF
		ja near M68K_Read_Word_Bad

		add ebx, [Bank_M68K]
		cmp byte [S68K_State], 1			; BUS available ?
		je near M68K_Read_Byte_Bad

		mov ax, [Ram_Prg + ebx - 0x20000]
		pop rbx
		ret

	ALIGN32

	M68K_Read_Word_WRam:
		cmp ebx, 0x23FFFF
		mov eax, [Ram_Word_State]
		ja short .bad
		and eax, 0x3
		jmp [.Table_Word_Ram + eax * 4]

	ALIGN4

	.Table_Word_Ram
;		dd .Word_Ram_2M, .bad
		dd .Word_Ram_2M, .Word_Ram_2M
		dd .Word_Ram_1M_0, .Word_Ram_1M_1

	ALIGN4

	.Word_Ram_2M
		mov ax, [Ram_Word_2M + ebx - 0x200000]
		pop rbx
		ret

	ALIGN4

	.Word_Ram_1M_0
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_0

		mov ax, [Ram_Word_1M + ebx - 0x200000]
		pop rbx
		ret

	ALIGN4

	.Word_Ram_1M_1
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_1

		mov ax, [Ram_Word_1M + ebx - 0x200000 + 0x20000]
		pop rbx
		ret

	ALIGN4

	.bad
		mov ax, 0
		pop rbx
		ret

	ALIGN4

	.Cell_Arranged_0
		xor eax, eax
		mov ax, [Cell_Conv_Tab + ebx - 0x220000]
		mov ax, [Ram_Word_1M + eax * 2]
		pop rbx
		ret

	ALIGN4

	.Cell_Arranged_1
		xor eax, eax
		mov ax, [Cell_Conv_Tab + ebx - 0x220000]
		mov ax, [Ram_Word_1M + eax * 2 + 0x20000]
		pop rbx
		ret


	ALIGN32

	M68K_Read_Word_BRAM_L:
		cmp ebx, 0x400000
		mov ax, 0
		jne short .bad

		mov ax, [BRAM_Ex_Size]

	.bad
		pop rbx
		ret


	ALIGN32

	M68K_Read_Word_BRAM:
		cmp ebx, 0x61FFFF
		mov ax, 0
		ja short .bad

		test word [BRAM_Ex_State], 0x100
		jz short .bad

		and ebx, 0x1FFFF
		shr ebx, 1
		mov ax, [Ram_Backup_Ex + ebx]

	.bad
		pop rbx
		ret


	ALIGN32

	M68K_Read_Word_BRAM_W:
		cmp ebx, 0x7FFFFE
		mov ax, 0
		jne short .bad

		xor ah, ah
		mov al, [BRAM_Ex_State]

	.bad
		pop rbx
		ret


	ALIGN32

	M68K_Read_Word_Misc_CD:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_ram

		test byte [Z80_State], 6
		jnz near .bad

		push rcx
		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		call [Z80_ReadB_Table + ebx]
		pop rdx
		pop rcx
		pop rbx
		ret

	ALIGN4

	.no_Z80_ram
		cmp ebx, 0xA11100
		jne short .no_busreq

		test byte [Z80_State], 2
		jnz short .z80_on

	.z80_off
		call main68k_readOdometer
		sub eax, [Last_BUS_REQ_Cnt]
		cmp eax, CYCLE_FOR_TAKE_Z80_BUS_SEGACD
		ja short .bus_taken

		mov al, [Fake_Fetch]
		mov ah, [Last_BUS_REQ_St]
		xor al, 0xFF
		or ah, 0x80
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		pop rbx
		ret

	ALIGN4

	.bus_taken
		mov al, [Fake_Fetch]
		mov ah, 0x80
		xor al, 0xFF
		pop rbx
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		ret

	ALIGN4

	.z80_on
		mov al, [Fake_Fetch]
		mov ah, 0x81
		xor al, 0xFF
		pop rbx
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA1000D
		ja .CD_Reg

		and ebx, 0x00000E
		jmp [.Table_IO_RW + ebx * 2]

	ALIGN4

	.Table_IO_RW
		dd .MD_Spec, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.MD_Spec
		mov ax, [Game_Mode]
		add ax, ax
		or ax, [CPU_Mode]
		shl ax, 6
		pop rbx
		or ax, [Gen_Version]									; on recupere les infos hardware de la machine
		ret

	ALIGN4

	.Pad_1
		call RD_Controller_1
		pop rbx
		ret

	ALIGN4

	.Pad_2
		call RD_Controller_2
		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov ax, [Controller_1_COM]
		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov ax, [Controller_2_COM]
		pop rbx
		ret

	ALIGN4

	.bad
		xor ax, ax
		pop rbx
		ret

	ALIGN4

	.CD_Reg
		cmp ebx, 0xA12000
		jb short .bad
		cmp ebx, 0xA1202F
		ja short .bad

		and ebx, 0x3E
		jmp [.Table_Extended_IO + ebx * 2]

	ALIGN4

	.Table_Extended_IO
		dd .S68K_Ctrl, .Memory_Ctrl, .CDC_Mode, .HINT_Vector
		dd .CDC_Host_Data, .Unknow, .Stop_Watch, .Com_Flag
		dd .Com_D0, .Com_D1, .Com_D2, .Com_D3
		dd .Com_D4, .Com_D5, .Com_D6, .Com_D7
		dd .Com_S0, .Com_S1, .Com_S2, .Com_S3
		dd .Com_S4, .Com_S5, .Com_S6, .Com_S7

	ALIGN4

	.S68K_Ctrl
		mov ah, [Int_Mask_S68K]
		mov al, [S68K_State]
		and ah, 4
		shl ah, 5
		pop rbx
		ret

	ALIGN4

	.Memory_Ctrl
		mov eax, [Bank_M68K]
		mov ebx, [Ram_Word_State]
		shr eax, 11
		and ebx, 3
		mov ah, [S68K_Mem_WP]
		or al, [Memory_Control_Status + ebx]
		pop rbx
		ret

	ALIGN4

	.CDC_Mode
		mov ah, [CDC.RS0 + 1]
		mov al, 0
		pop rbx
		ret

	ALIGN4

	.HINT_Vector
		mov ax, [Rom_Data + 0x72]
		pop rbx
		ret

	ALIGN4

	.CDC_Host_Data
		call Read_CDC_Host_MAIN
		pop rbx
		ret

	ALIGN4

	.Unknow
		mov ax, 0
		pop rbx
		ret

	ALIGN4

	.Stop_Watch
		mov ax, [CDC.Stop_Watch + 2]
		pop rbx
		ret

	ALIGN4

	.Com_Flag
		mov ax, [COMM.Flag]
		pop rbx
		ret

	ALIGN4

	.Com_D0
	.Com_D1
	.Com_D2
	.Com_D3
	.Com_D4
	.Com_D5
	.Com_D6
	.Com_D7
		mov ax, [COMM.Command + ebx - 0x10]
		pop rbx
		ret

	ALIGN4

	.Com_S0
	.Com_S1
	.Com_S2
	.Com_S3
	.Com_S4
	.Com_S5
	.Com_S6
	.Com_S7
		mov ax, [COMM.Status + ebx - 0x20]
		pop rbx
		ret


	; SegaCD extended Write Byte
	; *******************************************

	ALIGN32

	M68K_Write_Byte_Bios_CD:
		cmp ebx, 0x20000
		jb short .bad
		cmp ebx, 0x3FFFF
		ja short .bad

		add ebx, [Bank_M68K]
		cmp byte [S68K_State], 1			; BUS available ?
		je short .bad

		xor ebx, 1
		mov [Ram_Prg + ebx - 0x20000], al

	.bad
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	M68K_Write_Byte_WRam:
		cmp ebx, 0x23FFFF
		mov ecx, [Ram_Word_State]
		ja short .bad
		and ecx, 0x3
		jmp [.Table_Word_Ram + ecx * 4]

	ALIGN4

	.Table_Word_Ram
;		dd .Word_Ram_2M, .bad
		dd .Word_Ram_2M, .Word_Ram_2M
		dd .Word_Ram_1M_0, .Word_Ram_1M_1

	ALIGN4

	.Word_Ram_2M
		xor ebx, 1
		mov [Ram_Word_2M + ebx - 0x200000], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Word_Ram_1M_0
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_0
		xor ebx, 1
		mov [Ram_Word_1M + ebx - 0x200000], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Word_Ram_1M_1
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_1

		xor ebx, 1
		mov [Ram_Word_1M + ebx - 0x200000 + 0x20000], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
		mov ax, 0
;		pop rbx
		ret

	ALIGN4

	.Cell_Arranged_0
		shr ebx, 1
		mov ecx, 0
		mov bx, [Cell_Conv_Tab + ebx * 2 - 0x220000]
		adc ecx, 0
		and ebx, 0xFFFF
		mov [Ram_Word_1M + ebx * 2 + ecx], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Cell_Arranged_1
		shr ebx, 1
		mov ecx, 0
		mov bx, [Cell_Conv_Tab + ebx * 2 - 0x220000]
		adc ecx, 0
		and ebx, 0xFFFF
		mov [Ram_Word_1M + ebx * 2 + ecx + 0x20000], al
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	M68K_Write_Byte_BRAM:
		cmp ebx, 0x61FFFF
		ja short .bad

		cmp word [BRAM_Ex_State], 0x101
		jne short .bad

		and ebx, 0x1FFFF
		shr ebx, 1
		mov [Ram_Backup_Ex + ebx], al

	.bad
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	M68K_Write_Byte_BRAM_W:
		cmp ebx, 0x7FFFFF
		jne short .bad

		mov [BRAM_Ex_State], al

	.bad
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	M68K_Write_Byte_Misc_CD:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_mem

		test byte [Z80_State], 6
		jnz short .bad

		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		mov edx, eax
		call [Z80_WriteB_Table + ebx]
		pop rdx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_Z80_mem
		cmp ebx, 0xA11100
		jne near .no_busreq

		xor ecx, ecx
		mov ah, [Z80_State]
		mov dword [Controller_1_Counter], ecx
		test al, 1
		mov dword [Controller_1_Delay], ecx
		mov dword [Controller_2_Counter], ecx
		mov dword [Controller_2_Delay], ecx
		jnz short .deactivated

		test ah, 2
		jnz short .already_actived

		or ah, 2
		push rdx
		mov [Z80_State], ah
		mov ebx, [Cycles_M68K]
		call main68k_readOdometer
		sub ebx, eax
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Set_Odo
		pop rdx

	.already_actived
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.deactivated
		call main68k_readOdometer
		mov cl, [Z80_State]
		mov [Last_BUS_REQ_Cnt], eax
		test cl, 2
		setnz [Last_BUS_REQ_St]
		jz short .already_deactivated

		push rdx
		mov ebx, [Cycles_M68K]
		and cl, ~2
		sub ebx, eax
		mov [Z80_State], cl
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Exec
		pop rdx

	.already_deactivated
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA11200
		jne short .no_reset_z80

		test al, 1
		jnz short .no_reset

		push rdx
		mov ecx, M_Z80
		call z80_Reset
		or byte [Z80_State], 4
		call YM2612_Reset
		pop rdx
;		pop rcx
;		pop rbx
		ret

	.no_reset
		and byte [Z80_State], ~4
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_reset_z80
		cmp ebx, 0xA1000D
		ja short .no_ctrl_io

		and ebx, 0x00000E
		jmp [.Table_IO_WB + ebx * 2]

	ALIGN4

	.Table_IO_WB
		dd .bad, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.Pad_1
		push rax
		call WR_Controller_1
		add esp, 4
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Pad_2
		push rax
		call WR_Controller_2
		add esp, 4
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov [Controller_1_COM], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov [Controller_2_COM], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_ctrl_io
		cmp ebx, 0xA12000
		jb near M68K_Write_Bad
		cmp ebx, 0xA1202F
		ja near M68K_Write_Bad

		and ebx, 0x3F
		jmp [.Table_Extended_IO + ebx * 4]

	ALIGN4

	.Table_Extended_IO
		dd .S68K_Ctrl_H, .S68K_Ctrl_L, .Memory_Ctrl_H, .Memory_Ctrl_L
		dd .CDC_Mode_H, .CDC_Mode_L, .HINT_Vector_H, .HINT_Vector_L
		dd .CDC_Host_Data_H, .CDC_Host_Data_L, .Unknow, .Unknow
		dd .Stop_Watch_H, .Stop_Watch_L, .Com_Flag_H, .Com_Flag_L
		dd .Com_D0_H, .Com_D0_L, .Com_D1_H, .Com_D1_L
		dd .Com_D2_H, .Com_D2_L, .Com_D3_H, .Com_D3_L
		dd .Com_D4_H, .Com_D4_L, .Com_D5_H, .Com_D5_L
		dd .Com_D6_H, .Com_D6_L, .Com_D7_H, .Com_D7_L
		dd .Com_S0_H, .Com_S0_L, .Com_S1_H, .Com_S1_L
		dd .Com_S2_H, .Com_S2_L, .Com_S3_H, .Com_S3_L
		dd .Com_S4_H, .Com_S4_L, .Com_S5_H, .Com_S5_L
		dd .Com_S6_H, .Com_S6_L, .Com_S7_H, .Com_S7_L

	ALIGN4

	.S68K_Ctrl_L
		test al, 1
		jz short .S68K_Reseting
		test byte [S68K_State], 1
		jnz short .S68K_Already_Running

	.S68K_Restart

		push rax
		call sub68k_reset
		pop rax

	.S68K_Reseting
	.S68K_Already_Running
		and al, 3
		mov [S68K_State], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.S68K_Ctrl_H
		test al, 0x1
		jz .No_Process_INT2
		test byte [Int_Mask_S68K], 0x4
		jz .No_Process_INT2

		push dword -1
		push dword 2
		call sub68k_interrupt
		add esp, 8

	.No_Process_INT2
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Memory_Ctrl_L

		mov ebx, eax
		shr eax, 1
		and ebx, 0xC0
		test byte [Ram_Word_State], 0x2
		jnz short .Mode_1M

	.Mode_2M
		shl ebx, 11
		test al, 1
		mov [Bank_M68K], ebx
		jz short .No_DMNA

		mov byte [Ram_Word_State], 1
		call MS68K_Set_Word_Ram

	.No_DMNA
		call M68K_Set_Prg_Ram
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Mode_1M
		shl ebx, 11
		test al, 1
		jnz short .no_swap

		or word [Memory_Control_Status + 2], 0x0202		; DMNA bit = 1

	.no_swap
		mov [Bank_M68K], ebx
		call M68K_Set_Prg_Ram
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Memory_Ctrl_H
		mov [S68K_Mem_WP], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CDC_Mode_H
	.CDC_Mode_L
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.HINT_Vector_H
		mov [Rom_Data + 0x73], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.HINT_Vector_L
		mov [Rom_Data + 0x72], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CDC_Host_Data_H
	.CDC_Host_Data_L
	.Unknow
	.Stop_Watch_H
	.Stop_Watch_L
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Com_Flag_H

		mov [COMM.Flag + 1], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Com_Flag_L

		rol al, 1
		mov byte [COMM.Flag + 1], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Com_D0_H
	.Com_D0_L
	.Com_D1_H
	.Com_D1_L
	.Com_D2_H
	.Com_D2_L
	.Com_D3_H
	.Com_D3_L
	.Com_D4_H
	.Com_D4_L
	.Com_D5_H
	.Com_D5_L
	.Com_D6_H
	.Com_D6_L
	.Com_D7_H
	.Com_D7_L

		xor ebx, 1
		mov [COMM.Command + ebx - 0x10], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Com_S0_H
	.Com_S0_L
	.Com_S1_H
	.Com_S1_L
	.Com_S2_H
	.Com_S2_L
	.Com_S3_H
	.Com_S3_L
	.Com_S4_H
	.Com_S4_L
	.Com_S5_H
	.Com_S5_L
	.Com_S6_H
	.Com_S6_L
	.Com_S7_H
	.Com_S7_L
;		pop rcx
;		pop rbx
		ret


	; SegaCD extended Write Word
	; *******************************************

	ALIGN32

	M68K_Write_Word_Bios_CD:
		cmp ebx, 0x20000
		jb short .bad
		cmp ebx, 0x3FFFF
		ja short .bad

		add ebx, [Bank_M68K]
		cmp byte [S68K_State], 1			; BUS available ?
		je short .bad

		mov [Ram_Prg + ebx - 0x20000], ax

	.bad
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	M68K_Write_Word_WRam:
		cmp ebx, 0x23FFFF
		mov ecx, [Ram_Word_State]
		ja short .bad
		and ecx, 0x3
		jmp [.Table_Word_Ram + ecx * 4]

	ALIGN4

	.Table_Word_Ram
		dd .Word_Ram_2M, .Word_Ram_2M
		dd .Word_Ram_1M_0, .Word_Ram_1M_1

	ALIGN4

	.Word_Ram_2M
		mov [Ram_Word_2M + ebx - 0x200000], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Word_Ram_1M_0
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_0

		mov [Ram_Word_1M + ebx - 0x200000], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Word_Ram_1M_1
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_1

		mov [Ram_Word_1M + ebx  - 0x200000 + 0x20000], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Cell_Arranged_0
		mov bx, [Cell_Conv_Tab + ebx - 0x220000]
		and ebx, 0xFFFF
;		pop rcx
		mov [Ram_Word_1M + ebx * 2], ax
;		pop rbx
		ret

	ALIGN4

	.Cell_Arranged_1
		mov bx, [Cell_Conv_Tab + ebx - 0x220000]
		and ebx, 0xFFFF
;		pop rcx
		mov [Ram_Word_1M + ebx * 2 + 0x20000], ax
;		pop rbx
		ret


	ALIGN32

	M68K_Write_Word_BRAM:
		cmp ebx, 0x61FFFF
		ja short .bad

		cmp word [BRAM_Ex_State], 0x101
		jne short .bad

		and ebx, 0x1FFFE
		shr ebx, 1
		mov [Ram_Backup_Ex + ebx], ax

	.bad
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	M68K_Write_Word_BRAM_W:
		cmp ebx, 0x7FFFFE
		jne short .bad

		mov [BRAM_Ex_State], al

	.bad
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	M68K_Write_Word_Misc_CD:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_ram

		test byte [Z80_State], 6
		jnz near .bad

		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		mov dh, al
		shr ebx, 10
		mov dl, al
		call [Z80_WriteB_Table + ebx]
		pop rdx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_Z80_ram
		cmp ebx, 0xA11100
		jne near .no_busreq

		xor ecx, ecx
		mov al, [Z80_State]
		mov dword [Controller_1_Counter], ecx
		test ah, 1
		mov dword [Controller_1_Delay], ecx
		mov dword [Controller_2_Counter], ecx
		mov dword [Controller_2_Delay], ecx
		jnz short .deactivated

		test al, 2
		jnz short .already_actived

		or al, 2
		push rdx
		mov [Z80_State], al
		mov ebx, [Cycles_M68K]
		call main68k_readOdometer
		sub ebx, eax
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Set_Odo
		pop rdx

	.already_actived
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.deactivated
		call main68k_readOdometer
		mov cl, [Z80_State]
		mov [Last_BUS_REQ_Cnt], eax
		test cl, 2
		setnz [Last_BUS_REQ_St]
		jz short .already_deactivated

		push rdx
		mov ebx, [Cycles_M68K]
		and cl, ~2
		sub ebx, eax
		mov [Z80_State], cl
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Exec
		pop rdx

	.already_deactivated
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA11200
		jne short .no_reset_z80

		test ah, 1
		jnz short .no_reset

		push rdx
		mov ecx, M_Z80
		call z80_Reset
		or byte [Z80_State], 4
		call YM2612_Reset
		pop rdx
;		pop rcx
;		pop rbx
		ret

	.no_reset
		and byte [Z80_State], ~4
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_reset_z80
		cmp ebx, 0xA1000D
		ja short .no_ctrl_io

		and ebx, 0x00000E
		jmp [.Table_IO_WW + ebx * 2]

	ALIGN4

	.Table_IO_WW
		dd .bad, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.Pad_1
		push rax
		call WR_Controller_1
		add esp, 4
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Pad_2
		push rax
		call WR_Controller_2
		add esp, 4
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov [Controller_1_COM], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov [Controller_2_COM], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_ctrl_io
		cmp ebx, 0xA12000
		jb short .bad
		cmp ebx, 0xA1202F
		ja short .bad

		and ebx, 0x3E
		jmp [.Table_Extended_IO + ebx * 2]

	ALIGN4

	.Table_Extended_IO
		dd .S68K_Ctrl, .Memory_Ctrl, .CDC_Mode, .HINT_Vector
		dd .CDC_Host_Data, .Unknow, .Stop_Watch, .Com_Flag
		dd .Com_D0, .Com_D1, .Com_D2, .Com_D3
		dd .Com_D4, .Com_D5, .Com_D6, .Com_D7
		dd .Com_S0, .Com_S1, .Com_S2, .Com_S3
		dd .Com_S4, .Com_S5, .Com_S6, .Com_S7

	ALIGN4

	.S68K_Ctrl
		test al, 1
		jz short .S68K_Reseting
		test byte [S68K_State], 1
		jnz short .S68K_Already_Running

	.S68K_Restart

		push rax
		call sub68k_reset
		pop rax

	.S68K_Reseting
	.S68K_Already_Running
		and al, 3
		test ah, 1
		mov [S68K_State], al
		jz short .No_Process_INT2
		test byte [Int_Mask_S68K], 0x4
		jz short .No_Process_INT2

		push dword -1
		push dword 2
		call sub68k_interrupt
		add esp, 8

	.No_Process_INT2
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Memory_Ctrl

		mov [S68K_Mem_WP], ah
		mov ebx, eax
		shr eax, 1
		and ebx, 0xC0
		test byte [Ram_Word_State], 0x2
		jnz short .Mode_1M

	.Mode_2M
		shl ebx, 11
		test al, 1
		mov [Bank_M68K], ebx
		jz short .No_DMNA

		mov byte [Ram_Word_State], 1
		call MS68K_Set_Word_Ram

	.No_DMNA
		call M68K_Set_Prg_Ram
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Mode_1M
		shl ebx, 11
		test al, 1
		jnz short .no_swap

		or word [Memory_Control_Status + 2], 0x0202		; DMNA bit = 1

	.no_swap
		mov [Bank_M68K], ebx
		call M68K_Set_Prg_Ram
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CDC_Mode
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.HINT_Vector
		mov [Rom_Data + 0x72], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CDC_Host_Data
	.Unknow
	.Stop_Watch
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Com_Flag

		mov [COMM.Flag + 1], ah
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Com_D0
	.Com_D1
	.Com_D2
	.Com_D3
	.Com_D4
	.Com_D5
	.Com_D6
	.Com_D7

		mov [COMM.Command + ebx - 0x10], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Com_S0
	.Com_S1
	.Com_S2
	.Com_S3
	.Com_S4
	.Com_S5
	.Com_S6
	.Com_S7
;		pop rcx
;		pop rbx
		ret


%define _PWM_BUF_SIZE 4

	; 32X extended Read Byte
	; *******************************************

	ALIGN32

	M68K_Read_Byte_Bios_32X:
		and ebx, 0xFF
		xor ebx, byte 1
		mov al, [_32X_Genesis_Rom + ebx]
		pop rbx
		ret


	ALIGN32

	M68K_Read_Byte_BiosR_32X:
		cmp ebx, 0x100
		jae short .Rom

		xor ebx, byte 1
		mov al, [_32X_Genesis_Rom + ebx]
		pop rbx
		ret

	ALIGN4

	.Rom
		xor ebx, byte 1
		mov al, [Rom_Data + ebx]
		pop rbx
		ret


	ALIGN32

	M68K_Read_Byte_Misc_32X:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_mem

		test byte [Z80_State], 6
		jnz short .bad

		push rcx
		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		call [Z80_ReadB_Table + ebx]
		pop rdx
		pop rcx
		pop rbx
		ret

	ALIGN4

	.bad
		xor al, al
		pop rbx
		ret

	ALIGN4

	.no_Z80_mem
		cmp ebx, 0xA11100
		jne short .no_busreq

		test byte [Z80_State], 2
		jnz short .z80_on

	.z80_off
		call main68k_readOdometer
		sub eax, [Last_BUS_REQ_Cnt]
		cmp eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja short .bus_taken

		mov al, [Last_BUS_REQ_St]
		pop rbx
		or al, 0x80
		ret

	ALIGN4

	.bus_taken
		mov al, 0x80
		pop rbx
		ret

	ALIGN4

	.z80_on
		mov al, 0x81
		pop rbx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA15100
		jae near .32X_Reg

		cmp ebx, 0xA130EC
		jae .32X_ID

		cmp ebx, 0xA1000F
		ja short .bad

		and ebx, 0x00000E
		jmp [.Table_IO_RB + ebx * 2]

	ALIGN4

	.Table_IO_RB
		dd .MD_Spec, .Pad_1, .Pad_2, .Ser
		dd .CT_Pad_1, .CT_Pad_2, .CT_Ser, .bad

	ALIGN4

	.MD_Spec
		mov al, [Game_Mode]
		add al, al
		or al, [CPU_Mode]
		shl al, 6
		pop rbx
		or al, [Gen_Version]
		ret

	ALIGN4

	.Pad_1
		call RD_Controller_1
		pop rbx
		ret

	ALIGN4

	.Pad_2
		call RD_Controller_2
		pop rbx
		ret

	ALIGN4

	.Ser
		mov al, 0xFF
		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov al, [Controller_1_COM]
		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov al, [Controller_2_COM]
		pop rbx
		ret

	ALIGN4

	.CT_Ser
		xor al, al
		pop rbx
		ret

	ALIGN4

	.32X_ID
		and ebx, 3
		mov al, [.32X_ID_Tab + ebx]
		pop rbx
		ret

	ALIGN4

	.32X_ID_Tab
		db 'A', 'M', 'S', 'R'

	ALIGN32

	.32X_Reg
		cmp ebx, 0xA15180
		jae near .32X_VDP_Reg

		and ebx, 0x3F
		jmp [.Table_32X_Reg + ebx * 4]

	ALIGN4

	.Table_32X_Reg
		dd .32X_ACR_H, .32X_ACR_L, .32X_bad, .32X_Int		; 00-03
		dd .32X_bad, .32X_Bank, .32X_bad, .32X_DREQ_C		; 04-07
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 08-0B
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 0C-0F

		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 10-13
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 14-17
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 18-1B
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 1C-1F

		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm		; 20
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm

		dd .32X__PWM_Cont_H, .32X__PWM_Cont_L
		dd .32X__PWM_Cycle_H, .32X__PWM_Cycle_L
		dd .32X__PWM_Pulse_L, .32X_bad
		dd .32X__PWM_Pulse_R, .32X_bad
		dd .32X__PWM_Pulse_L, .32X_bad
		dd .32X_bad, .32X_bad
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad

	ALIGN32

	.32X_ACR_H
		mov al, [_32X_FM]
		pop rbx
		ret

	ALIGN4

	.32X_ACR_L
		mov al, [_32X_ADEN]
		mov ah, [_32X_RES]
		or al, ah
		pop rbx
		or al, 0x80
		ret

	ALIGN4

	.32X_Int
		xor al, al
		pop rbx
		ret

	ALIGN4

	.32X_Bank
		mov al, [Bank_SH2]
		pop rbx
		ret

	ALIGN32

	.32X_DREQ_C
		mov al, [_32X_RV]
		mov bl, [_32X_DREQ_ST + 0]
		mov ah, [_32X_DREQ_ST + 1]
		or al, bl
		and ah, 0x80
		pop rbx
		or al, ah
		ret


	ALIGN32

	.32X_Comm
		mov al, [_32X_Comm + ebx - 0x20]
		pop rbx
		ret


	ALIGN32

	.32X__PWM_Cont_H
		mov al, [PWM_Mode + 1]
		pop rbx
		ret

	ALIGN4

	.32X__PWM_Cont_L
		mov al, [PWM_Mode + 0]
		pop rbx
		ret

	ALIGN4

	.32X__PWM_Cycle_H
		mov al, [PWM_Cycle_Tmp + 1]
		pop rbx
		ret

	ALIGN4

	.32X__PWM_Cycle_L
		mov al, [PWM_Cycle_Tmp + 0]
		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_L
		mov ebx, [PWM_RP_L]
		mov eax, [PWM_WP_L]
		mov al, [PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop rbx
		ret

	ALIGN4

	.32X__PWM_Pulse_R
		mov ebx, [PWM_RP_R]
		mov eax, [PWM_WP_R]
		mov al, [PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop rbx
		ret

	ALIGN4

	.32X__PWM_Pulse_C
		mov ebx, [PWM_RP_L]
		mov eax, [PWM_WP_L]
		mov al, [PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop rbx
		ret


	ALIGN32

	.32X_VDP_Reg
		test byte [_32X_FM], 0xFF
		jnz near .32X_bad
		cmp ebx, 0xA15200
		jae near .32X_bad

		and ebx, 0xF
		jmp [.Table_32X_VDP_Reg + ebx * 4]

	ALIGN4

	.Table_32X_VDP_Reg
		dd .32X_VDP_Mode_H, .32X_VDP_Mode_L, .32X_bad, .32X_VDP_Shift
		dd .32X_bad, .32X_VDP_AF_Len_L, .32X_VDP_AF_St_H, .32X_VDP_AF_St_L
		dd .32X_VDP_AF_Data_H, .32X_VDP_AF_Data_L, .32X_VDP_State_H, .32X_VDP_State_L
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad

	ALIGN32

	.32X_VDP_Mode_H
		mov al, [_32X_VDP + vx.Mode + 1]
		pop rbx
		ret

	ALIGN4

	.32X_VDP_Mode_L
		mov al, [_32X_VDP + vx.Mode + 0]
		pop rbx
		ret

	ALIGN4

	.32X_VDP_Shift
		mov al, [_32X_VDP + vx.Mode + 2]
		pop rbx
		ret

	ALIGN4

	.32X_VDP_AF_Len_L
		mov al, [_32X_VDP + vx.AF_Len + 0]
		pop rbx
		ret

	ALIGN32

	.32X_VDP_AF_St_H
		mov al, [_32X_VDP + vx.AF_St + 1]
		pop rbx
		ret

	ALIGN4

	.32X_VDP_AF_St_L
		mov al, [_32X_VDP + vx.AF_St + 0]
		pop rbx
		ret

	ALIGN4

	.32X_VDP_AF_Data_H
		mov al, [_32X_VDP + vx.AF_Data + 1]
		pop rbx
		ret

	ALIGN4

	.32X_VDP_AF_Data_L
		mov al, [_32X_VDP + vx.AF_Data + 0]
		pop rbx
		ret

	ALIGN32

	.32X_VDP_State_H
		mov al, [_32X_VDP + vx.State + 1]
		pop rbx
		ret

	ALIGN4

	.32X_VDP_State_L
		mov al, [_32X_VDP + vx.State]
		xor al, 2
		mov [_32X_VDP + vx.State], al
		pop rbx
		ret

	ALIGN4

	.32X_bad
		xor al, al
		pop rbx
		ret


	ALIGN32

	DECL M68K_Read_Byte_32X_FB0
		and ebx, 0x1FFFF
		xor ebx, byte 1
		mov al, [_32X_VDP_Ram + ebx]
		pop rbx
		ret


	ALIGN32

	DECL M68K_Read_Byte_32X_FB1
		and ebx, 0x1FFFF
		xor ebx, byte 1
		mov al, [_32X_VDP_Ram + ebx + 0x20000]
		pop rbx
		ret




	; 32X extended Read Word
	; *******************************************

	ALIGN32

	M68K_Read_Word_Bios_32X:
		and ebx, 0xFE
		mov ax, [_32X_Genesis_Rom + ebx]
		pop rbx
		ret


	ALIGN32

	M68K_Read_Word_BiosR_32X:
		cmp ebx, 0x100
		jae short .Rom

		mov ax, [_32X_Genesis_Rom + ebx]
		pop rbx
		ret

	ALIGN4

	.Rom
		mov ax, [Rom_Data + ebx]
		pop rbx
		ret


	ALIGN32

	M68K_Read_Word_Misc_32X:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_ram

		test byte [Z80_State], 6
		jnz near .bad

		push rcx
		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		call [Z80_ReadB_Table + ebx]
		pop rdx
		pop rcx
		pop rbx
		ret

	ALIGN4

	.no_Z80_ram
		cmp ebx, 0xA11100
		jne short .no_busreq

		test byte [Z80_State], 2
		jnz short .z80_on

	.z80_off
		call main68k_readOdometer
		sub eax, [Last_BUS_REQ_Cnt]
		cmp eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja short .bus_taken

		mov al, [Fake_Fetch]
		mov ah, [Last_BUS_REQ_St]
		xor al, 0xFF
		add ah, 0x80
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		pop rbx
		ret

	ALIGN4

	.bus_taken
		mov al, [Fake_Fetch]
		mov ah, 0x80
		xor al, 0xFF
		pop rbx
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		ret

	ALIGN4

	.z80_on
		mov al, [Fake_Fetch]
		mov ah, 0x81
		xor al, 0xFF
		pop rbx
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA15100
		jae near .32X_Reg

		cmp ebx, 0xA130EC
		jae .32X_ID

		cmp ebx, 0xA1000F
		ja short .bad

		and ebx, 0x00000E
		jmp [.Table_IO_RW + ebx * 2]

	ALIGN4

	.Table_IO_RW
		dd .MD_Spec, .Pad_1, .Pad_2, .Ser
		dd .CT_Pad_1, .CT_Pad_2, .CT_Ser, .bad

	ALIGN4

	.MD_Spec
		mov eax, [Game_Mode]
		add eax, eax
		or eax, [CPU_Mode]
		shl eax, 6
		pop rbx
		or eax, [Gen_Version]									; on recupere les infos hardware de la machine
		ret

	ALIGN4

	.Pad_1
		call RD_Controller_1
		pop rbx
		ret

	ALIGN4

	.Pad_2
		call RD_Controller_2
		pop rbx
		ret

	ALIGN4

	.Ser
		mov ax, 0xFF00
		pop rbx
		ret

	ALIGN4

	.bad
		xor eax, eax
		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov eax, [Controller_1_COM]
		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov eax, [Controller_2_COM]
		pop rbx
		ret

	ALIGN4

	.CT_Ser
		xor eax, eax
		pop rbx
		ret

	ALIGN4

	.32X_ID
		and ebx, 3
		mov ax, [.32X_ID_Tab + ebx]
		pop rbx
		ret

	ALIGN4

	.32X_ID_Tab
		db 'A', 'M', 'S', 'R'

	ALIGN4

	.32X_Reg
		cmp ebx, 0xA15180
		jae near .32X_VDP_Reg

		and ebx, 0x3E
		jmp [.Table_32X_Reg + ebx * 2]

	ALIGN4

	.Table_32X_Reg
		dd .32X_ACR, .32X_INT, .32X_Bank, .32X_DREQ_C		; 00-07
		dd .32X_DREQ_Src_H, .32X_DREQ_Src_L,				; 08-0B
		dd .32X_DREQ_Dest_H, .32X_DREQ_Dest_L,				; 0C-0F

		dd .32X_DREQ_Len, .32X_FIFO, .32X_bad, .32X_bad		; 10-17
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 18-1F

		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm		; 20
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm

		dd .32X__PWM_Cont, .32X__PWM_Cycle
		dd .32X__PWM_Pulse_L, .32X__PWM_Pulse_R
		dd .32X__PWM_Pulse_C, .32X_bad
		dd .32X_bad, .32X_bad

	ALIGN32

	.32X_ACR
		mov al, [_32X_ADEN]
		mov ah, [_32X_RES]
		or al, ah
		mov ah, [_32X_FM]
		or al, 0x80
		pop rbx
		ret

	ALIGN4

	.32X_INT
		xor ax, ax
		pop rbx
		ret

	ALIGN4

	.32X_Bank
		mov al, [Bank_SH2]
		xor ah, ah
		pop rbx
		ret

	ALIGN4

	.32X_DREQ_C
		mov bl, [_32X_DREQ_ST + 0]
		mov ah, [_32X_DREQ_ST + 1]
		mov al, [_32X_RV]
		and ah, 0x80
		or al, bl
		or al, ah
		pop rbx
		xor ah, ah
		ret

	ALIGN4

	.32X_DREQ_Src_H
		mov ax, [_32X_DREQ_SRC + 2]
		pop rbx
		ret

	ALIGN4

	.32X_DREQ_Src_L
		mov ax, [_32X_DREQ_SRC]
		pop rbx
		ret

	ALIGN4

	.32X_DREQ_Dest_H
		mov ax, [_32X_DREQ_DST + 2]
		pop rbx
		ret

	ALIGN4

	.32X_DREQ_Dest_L
		mov ax, [_32X_DREQ_DST]
		pop rbx
		ret

	ALIGN4

	.32X_DREQ_Len
		mov ax, [_32X_DREQ_LEN]
		pop rbx
		ret

	ALIGN4

	.32X_FIFO
		pop rbx
		ret


	ALIGN32

	.32X_Comm
		mov ah, [_32X_Comm + ebx - 0x20 + 0]
		mov al, [_32X_Comm + ebx - 0x20 + 1]
		pop rbx
		ret


	ALIGN32

	.32X__PWM_Cont
		mov ax, [PWM_Mode]
		pop rbx
		ret

	ALIGN4

	.32X__PWM_Cycle
		mov ax, [PWM_Cycle_Tmp]
		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_L
		mov ebx, [PWM_RP_L]
		mov eax, [PWM_WP_L]
		mov ah, [PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop rbx
		ret

	ALIGN4

	.32X__PWM_Pulse_R
		mov ebx, [PWM_RP_R]
		mov eax, [PWM_WP_R]
		mov ah, [PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop rbx
		ret

	ALIGN4

	.32X__PWM_Pulse_C
		mov ebx, [PWM_RP_L]
		mov eax, [PWM_WP_L]
		mov ah, [PWM_FULL_TAB + ebx * _PWM_BUF_SIZE + eax]
		pop rbx
		ret


	ALIGN32

	.32X_VDP_Reg
		test byte [_32X_FM], 0xFF
		jnz near .32X_bad
		cmp ebx, 0xA15200
		jae near .32X_CRAM

		and ebx, 0xE
		jmp [.Table_32X_VDP_Reg + ebx * 2]

	ALIGN4

	.Table_32X_VDP_Reg
		; VDP REG

		dd .32X_VDP_Mode, .32X_VDP_Shift, .32X_VDP_AF_Len, .32X_VDP_AF_St
		dd .32X_VDP_AF_Data, .32X_VDP_State, .32X_bad, .32X_bad

	ALIGN32

	.32X_VDP_Mode
		mov ax, [_32X_VDP + vx.Mode]
		pop rbx
		ret

	ALIGN4

	.32X_VDP_Shift
		mov al, [_32X_VDP + vx.Mode + 2]
		xor ah, ah
		pop rbx
		ret

	ALIGN4

	.32X_VDP_AF_Len
		mov al, [_32X_VDP + vx.AF_Len]
		xor ah, ah
		pop rbx
		ret

	ALIGN4

	.32X_VDP_AF_St
		mov ax, [_32X_VDP + vx.AF_St]
		pop rbx
		ret

	ALIGN4

	.32X_VDP_AF_Data
		mov ax, [_32X_VDP + vx.AF_Data]
		pop rbx
		ret

	ALIGN32

	.32X_VDP_State
		mov ax, [_32X_VDP + vx.State]
		xor ax, byte 2
		mov [_32X_VDP + vx.State], ax
		pop rbx
		ret

	ALIGN4

	.32X_bad
		xor ax, ax
		pop rbx
		ret

	ALIGN32

	.32X_CRAM
		cmp ebx, 0xA15400
		jae short .32X_bad

		mov ax, [_32X_VDP_CRam + ebx - 0xA15200]
		pop rbx
		ret


	ALIGN32

	DECL M68K_Read_Word_32X_FB0
		and ebx, 0x1FFFE
		mov ax, [_32X_VDP_Ram + ebx]
		pop rbx
		ret


	ALIGN32

	DECL M68K_Read_Word_32X_FB1
		and ebx, 0x1FFFE
		mov ax, [_32X_VDP_Ram + ebx + 0x20000]
		pop rbx
		ret



	; 32X extended Write Byte
	; *******************************************

	ALIGN32

	M68K_Write_Byte_Misc_32X:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_mem

		test byte [Z80_State], 6
		jnz short .bad

		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		mov edx, eax
		call [Z80_WriteB_Table + ebx]
		pop rdx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_Z80_mem
		cmp ebx, 0xA11100
		jne near .no_busreq

		xor ecx, ecx
		mov ah, [Z80_State]
		mov dword [Controller_1_Counter], ecx
		test al, 1
		mov dword [Controller_1_Delay], ecx
		mov dword [Controller_2_Counter], ecx
		mov dword [Controller_2_Delay], ecx
		jnz short .deactivated

		test ah, 2
		jnz short .already_actived

		or ah, 2
		push rdx
		mov [Z80_State], ah
		mov ebx, [Cycles_M68K]
		call main68k_readOdometer
		sub ebx, eax
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Set_Odo
		pop rdx

	.already_actived
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.deactivated
		call main68k_readOdometer
		mov cl, [Z80_State]
		mov [Last_BUS_REQ_Cnt], eax
		test cl, 2
		setnz [Last_BUS_REQ_St]
		jz short .already_deactivated

		push rdx
		mov ebx, [Cycles_M68K]
		and cl, ~2
		sub ebx, eax
		mov [Z80_State], cl
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Exec
		pop rdx

	.already_deactivated
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA11200
		jne short .no_reset_z80

		test al, 1
		jnz short .no_reset

		push rdx
		mov ecx, M_Z80
		call z80_Reset
		or byte [Z80_State], 4
		call YM2612_Reset
		pop rdx
;		pop rcx
;		pop rbx
		ret

	.no_reset
		and byte [Z80_State], ~4
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_reset_z80
		cmp ebx, 0xA15100
		jae near .32X_Reg

		cmp ebx, 0xA130F0
		jae short .Genesis_Bank

		cmp ebx, 0xA1000F
		ja near .bad

		and ebx, 0x00000E
		jmp [.Table_IO_WB + ebx * 2]

	ALIGN4

	.Table_IO_WB
		dd .bad, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.Pad_1
		push rax
		call WR_Controller_1
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Pad_2
		push rax
		call WR_Controller_2
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov [Controller_1_COM], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov [Controller_2_COM], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Genesis_Bank
		cmp ebx, 0xA130F2
		jb short .bank_0
		cmp ebx, 0xA130FF
		ja near .bad

		and ebx, 0xF
		and eax, 0x1F
		shr ebx, 1
		mov ecx, [Genesis_M68K_Read_Byte_Table + eax * 4]
		mov [M68K_Read_Byte_Table + ebx * 4], ecx
		mov ecx, [Genesis_M68K_Read_Word_Table + eax * 4]
		mov [M68K_Read_Word_Table + ebx * 4], ecx

;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bank_0
		test al, 1
		setnz [SRAM_ON]
		test al, 2
		setz [SRAM_Write]
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_Reg
		cmp ebx, 0xA15180
		jae near .32X_VDP_Reg

		and ebx, 0x3F
		jmp [.Table_32X_Reg + ebx * 4]

	ALIGN4

	.Table_32X_Reg
		dd .32X_ACR_H, .32X_ACR_L, .32X_bad, .32X_Int		; 00-03
		dd .32X_bad, .32X_Bank, .32X_bad, .32X_DREQ_C		; 04-07
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 08-0B
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 0C-0F

		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 10-13
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 14-17
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 18-1B
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 1C-1F

		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm		; 20
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm

		dd .32X_bad, .32X__PWM_Cont_L
		dd .32X__PWM_Cycle_H, .32X__PWM_Cycle_L
		dd .32X__PWM_Pulse_L_H, .32X__PWM_Pulse_L_L
		dd .32X__PWM_Pulse_R_H, .32X__PWM_Pulse_R_L
		dd .32X__PWM_Pulse_C_H, .32X__PWM_Pulse_C_L
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad
		dd .32X_bad, .32X_bad

	ALIGN32

	.32X_ACR_H
		and al, 0x80
		mov ah, [_32X_FM]
;		pop rcx
		xor ah, al
;		pop rbx
		mov [_32X_FM], al
		jnz near _32X_Set_FB
		ret

	ALIGN32

	.32X_ACR_L
		mov ah, [_32X_RES]
		mov bl, al
		and al, 2
		mov [_32X_RES], al
		cmp ax, byte 2
		jne short .no_SH2_reset

		push rdx
		mov ecx, M_SH2
		mov edx, 1
		call SH2_Reset
		mov ecx, S_SH2
		mov edx, 1
		call SH2_Reset
		pop rdx

	.no_SH2_reset
		mov al, [_32X_ADEN]
		and bl, 1
		xor al, bl
		jz short .no_32X_change

		mov [_32X_ADEN], bl
		call M68K_32X_Mode

	.no_32X_change
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_Int
		mov bl, al
		mov ah, [_32X_MINT]
		add al, al
		and bl, 2
		and al, 2
		mov bh, [_32X_SINT]
		test ah, al
		push rdx
		jz short .no_MINT

		mov edx, 8
		mov ecx, M_SH2
		call SH2_Interrupt

	.no_MINT
		test bh, bl
		jz short .no_SINT

		mov edx, 8
		mov ecx, S_SH2
		call SH2_Interrupt

	.no_SINT
		pop rdx
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_Bank
		and al, 3
		mov [Bank_SH2], al
		call M68K_Set_32X_Rom_Bank
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_DREQ_C
		mov bl, al
		mov ah, [_32X_RV]
		and al, 1
		mov bh, [_32X_DREQ_ST]
		and bl, 4
		xor ah, al
		jz short .RV_not_changed

		mov [_32X_RV], al
		call M68K_32X_Mode

	.RV_not_changed
		cmp bx, 0x0004
		jne short .No_DREQ

		xor al, al
		mov byte [_32X_DREQ_ST + 1], 0x40
		mov [_32X_FIFO_Block], al
		mov [_32X_FIFO_Read], al
		mov [_32X_FIFO_Write], al

	.No_DREQ
		mov [_32X_DREQ_ST], bl
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	.32X_Comm
		mov [_32X_Comm + ebx - 0x20], al
;		pop rcx
;		pop rbx
		ret


	ALIGN4

	.32X__PWM_Cont_L
		mov [PWM_Mode + 0], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X__PWM_Cycle_H
		mov cl, [PWM_Cycle_Tmp + 0]
		mov [PWM_Cycle_Tmp + 1], al
		mov ch, al
		call PWM_Set_Cycle
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X__PWM_Cycle_L
		mov ch, [PWM_Cycle_Tmp + 1]
		mov [PWM_Cycle_Tmp + 0], al
		mov cl, al
		call PWM_Set_Cycle
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_L_H
		mov [PWM_FIFO_L_Tmp + 1], al
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_L_L
		mov ecx, [PWM_RP_L]
		mov ebx, [PWM_WP_L]
		mov ah, [PWM_FIFO_L_Tmp + 1]
		test byte [PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_L_full

		mov [PWM_FIFO_L + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [PWM_WP_L], ebx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X__PWM_Pulse_L_full
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_R_H
		mov [PWM_FIFO_R_Tmp + 1], al
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_R_L
		mov ecx, [PWM_RP_R]
		mov ebx, [PWM_WP_R]
		mov ah, [PWM_FIFO_R_Tmp + 1]
		test byte [PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_R_full

		mov [PWM_FIFO_R + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [PWM_WP_R], ebx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X__PWM_Pulse_R_full
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_C_H
		mov [PWM_FIFO_L_Tmp + 1], al
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_C_L
		mov ecx, [PWM_RP_L]
		mov ebx, [PWM_WP_L]
		mov ah, [PWM_FIFO_L_Tmp + 1]
		test byte [PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_C_full

		mov [PWM_FIFO_L + ebx * 2], ax
		mov [PWM_FIFO_R + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [PWM_WP_L], ebx
		mov [PWM_WP_R], ebx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X__PWM_Pulse_C_full
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	.32X_VDP_Reg
		test byte [_32X_FM], 0xFF
		jnz near .32X_bad
		cmp ebx, 0xA15200
		jae near .32X_bad

		and ebx, 0xF
		jmp [.Table_32X_VDP_Reg + ebx * 4]

	ALIGN4

	.Table_32X_VDP_Reg
		dd .32X_bad, .32X_VDP_Mode, .32X_bad, .32X_VDP_Shift
		dd .32X_bad, .32X_VDP_AF_Len, .32X_bad, .32X_bad
		dd .32X_bad, .32X_bad, .32X_bad, .32X_VDP_State
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad

	ALIGN32

	.32X_VDP_Mode
		mov [_32X_VDP + vx.Mode], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_VDP_Shift
		mov [_32X_VDP + vx.Mode + 2], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_VDP_AF_Len
		mov [_32X_VDP + vx.AF_Len], al
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_VDP_State
		mov bh, [_32X_VDP + vx.Mode + 0]
		mov bl, [_32X_VDP + vx.State + 1]
		test bh, 3
		mov [_32X_VDP + vx.State + 2], al
		jz short .32X_VDP_blank

		test bl, bl
		jns short .32X_VDP_State_nvb

	.32X_VDP_blank
		mov [_32X_VDP + vx.State + 0], al
		call _32X_Set_FB

	.32X_VDP_State_nvb
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_bad
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	DECL M68K_Write_Byte_32X_FB0
		and ebx, 0x1FFFF
		test al, al
		jz short .blank

		xor ebx, byte 1
		mov [_32X_VDP_Ram + ebx], al

	.blank
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	DECL M68K_Write_Byte_32X_FB1
		and ebx, 0x1FFFF
		test al, al
		jz short .blank

		xor ebx, byte 1
		mov [_32X_VDP_Ram + ebx + 0x20000], al

	.blank
;		pop rcx
;		pop rbx
		ret



	; 32X extended Write Word
	; *******************************************


	ALIGN32

	M68K_Write_Word_Misc_32X:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_ram

		test byte [Z80_State], 6
		jnz near .bad

		push rdx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		mov dh, al
		shr ebx, 10
		mov dl, al
		call [Z80_WriteB_Table + ebx]
		pop rdx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_Z80_ram
		cmp ebx, 0xA11100
		jne near .no_busreq

		xor ecx, ecx
		mov al, [Z80_State]
		mov dword [Controller_1_Counter], ecx
		test ah, 1
		mov dword [Controller_1_Delay], ecx
		mov dword [Controller_2_Counter], ecx
		mov dword [Controller_2_Delay], ecx
		jnz short .deactivated

		test al, 2
		jnz short .already_actived

		or al, 2
		push rdx
		mov [Z80_State], al
		mov ebx, [Cycles_M68K]
		call main68k_readOdometer
		sub ebx, eax
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Set_Odo
		pop rdx

	.already_actived
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.deactivated
		call main68k_readOdometer
		mov cl, [Z80_State]
		mov [Last_BUS_REQ_Cnt], eax
		test cl, 2
		setnz [Last_BUS_REQ_St]
		jz short .already_deactivated

		push rdx
		mov ebx, [Cycles_M68K]
		and cl, ~2
		sub ebx, eax
		mov [Z80_State], cl
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Exec
		pop rdx

	.already_deactivated
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA11200
		jne short .no_reset_z80

		test ah, 1
		jnz short .no_reset

		push rdx
		mov ecx, M_Z80
		call z80_Reset
		or byte [Z80_State], 4
		call YM2612_Reset
		pop rdx
;		pop rcx
;		pop rbx
		ret

	.no_reset
		and byte [Z80_State], ~4
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.no_reset_z80
		cmp ebx, 0xA15100
		jae near .32X_Reg

		cmp ebx, 0xA130F0
		jae short .Genesis_Bank

		cmp ebx, 0xA1000F
		ja short .bad

		and ebx, 0x00000E
		jmp [.Table_IO_WW + ebx * 2]

	ALIGN4

	.Table_IO_WW
		dd .bad, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.Pad_1
		push rax
		call WR_Controller_1
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Pad_2
		push rax
		call WR_Controller_2
		pop rax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_1
		mov [Controller_1_COM], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.CT_Pad_2
		mov [Controller_2_COM], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bad
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.Genesis_Bank
		cmp ebx, 0xA130F2
		jb short .bank_0
		cmp ebx, 0xA130FF
		ja near .bad

;		and ebx, 0xF
;		and eax, 0x1F
;		shr ebx, 1
;		mov ecx, [Genesis_M68K_Read_Byte_Table + eax * 4]
;		mov [M68K_Read_Byte_Table + ebx * 4], ecx
;		mov ecx, [Genesis_M68K_Read_Word_Table + eax * 4]
;		mov [M68K_Read_Word_Table + ebx * 4], ecx

;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.bank_0
		test al, 1
		setnz [SRAM_ON]
		test al, 2
		setz [SRAM_Write]
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_Reg
		cmp ebx, 0xA15180
		jae near .32X_VDP_Reg

		and ebx, 0x3E
		jmp [.Table_32X_Reg + ebx * 2]

	ALIGN4

	.Table_32X_Reg
		dd .32X_ACR, .32X_INT, .32X_Bank, .32X_DREQ_C		; 00-07
		dd .32X_DREQ_Src_H, .32X_DREQ_Src_L,				; 08-0B
		dd .32X_DREQ_Dest_H, .32X_DREQ_Dest_L,				; 0C-0F

		dd .32X_DREQ_Len, .32X_FIFO, .32X_bad, .32X_bad		; 10-17
		dd .32X_bad, .32X_bad, .32X_bad, .32X_bad			; 18-1F

		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm		; 20
		dd .32X_Comm, .32X_Comm, .32X_Comm, .32X_Comm

		dd .32X__PWM_Cont, .32X__PWM_Cycle
		dd .32X__PWM_Pulse_L, .32X__PWM_Pulse_R
		dd .32X__PWM_Pulse_C, .32X_bad
		dd .32X_bad, .32X_bad

	ALIGN32

	.32X_ACR
		mov bh, [_32X_FM]
		and ah, 0x80
		mov bl, al
		xor bh, ah
		mov [_32X_FM], ah
		jz short .no_update_FB

		call _32X_Set_FB

	.no_update_FB
		mov al, bl
		mov ah, [_32X_RES]
		and al, 2
		mov [_32X_RES], al
		cmp ax, byte 2
		jne short .no_SH2_reset

		push rdx
		mov ecx, M_SH2
		mov edx, 1
		call SH2_Reset
		mov ecx, S_SH2
		mov edx, 1
		call SH2_Reset
		pop rdx

	.no_SH2_reset
		mov al, [_32X_ADEN]
		and bl, 1
		xor al, bl
		jz short .no_32X_change

		mov [_32X_ADEN], bl
		call M68K_32X_Mode

	.no_32X_change
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_INT
		mov bl, al
		mov ah, [_32X_MINT]
		add al, al
		and bl, 2
		and al, 2
		mov bh, [_32X_SINT]
		test ah, al
		push rdx
		jz short .no_MINT

		mov edx, 8
		mov ecx, M_SH2
		call SH2_Interrupt

	.no_MINT
		test bh, bl
		jz short .no_SINT

		mov edx, 8
		mov ecx, S_SH2
		call SH2_Interrupt

	.no_SINT
		pop rdx
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_Bank
		and al, 3
		mov [Bank_SH2], al
		call M68K_Set_32X_Rom_Bank
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_DREQ_C
		mov bl, al
		mov ah, [_32X_RV]
		and al, 1
		mov bh, [_32X_DREQ_ST]
		and bl, 4
		xor ah, al
		jz short .RV_not_changed

		mov [_32X_RV], al
		call M68K_32X_Mode

	.RV_not_changed
		cmp bx, 0x0004
		jne short .No_DREQ

		xor al, al
		mov byte [_32X_DREQ_ST + 1], 0x40
		mov [_32X_FIFO_Block], al
		mov [_32X_FIFO_Read], al
		mov [_32X_FIFO_Write], al

	.No_DREQ
		mov [_32X_DREQ_ST], bl
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_DREQ_Src_H
		mov [_32X_DREQ_SRC + 2], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_DREQ_Src_L
		and ax, byte ~1
;		pop rcx
		mov [_32X_DREQ_SRC], ax
;		pop rbx
		ret

	ALIGN4

	.32X_DREQ_Dest_H
		mov [_32X_DREQ_DST + 2], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_DREQ_Dest_L
		mov [_32X_DREQ_DST], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_DREQ_Len
		and ax, byte ~3
;		pop rcx
		mov [_32X_DREQ_LEN], ax
;		pop rbx
		ret

	ALIGN4

	.32X_FIFO
		mov cx, [_32X_DREQ_ST]
		mov ebx, [_32X_FIFO_Write]
		and cx, 0x8004
		cmp cx, 0x0004
		mov ecx, [_32X_FIFO_Block]
		jne short .32X_FIFO_End

		mov [_32X_FIFO_A + ecx + ebx * 2], ax
		inc ebx
		cmp ebx, 4
		jae short .32X_FIFO_Full_A

		mov [_32X_FIFO_Write], ebx

	.32X_FIFO_End
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_FIFO_Full_A
		mov bl, [_32X_DREQ_ST + 1]
		push rdx
		test bl, 0x40
		jz short .32X_FIFO_Full_B

		xor al, al
		xor ecx, byte (4 * 2)
		mov [_32X_DREQ_ST + 1], al
		mov [_32X_FIFO_Write], al
		mov [_32X_FIFO_Read], al
		mov [_32X_FIFO_Block], ecx

		mov dl, 1
		mov ecx, M_SH2
		call SH2_DMA0_Request

		pop rdx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_FIFO_Full_B
		mov byte [_32X_DREQ_ST + 1], 0x80

		mov dl, 1
		mov ecx, M_SH2
		call SH2_DMA0_Request

		pop rdx
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	.32X_Comm
		mov [_32X_Comm + ebx - 0x20 + 0], ah
		mov [_32X_Comm + ebx - 0x20 + 1], al
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	.32X__PWM_Cont
		and al, 0x0F
;		pop rcx
		mov [PWM_Mode], al
;		pop rbx
		ret

	ALIGN32

	.32X__PWM_Cycle
		mov ecx, eax
		call PWM_Set_Cycle
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_L
		mov ecx, [PWM_RP_L]
		mov ebx, [PWM_WP_L]
		test byte [PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_L_full

		mov [PWM_FIFO_L + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [PWM_WP_L], ebx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X__PWM_Pulse_L_full
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_R
		mov ecx, [PWM_RP_R]
		mov ebx, [PWM_WP_R]
		test byte [PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_R_full

		mov [PWM_FIFO_R + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [PWM_WP_R], ebx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X__PWM_Pulse_R_full
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X__PWM_Pulse_C
		mov ecx, [PWM_RP_L]
		mov ebx, [PWM_WP_L]
		test byte [PWM_FULL_TAB + ecx * _PWM_BUF_SIZE + ebx], 0x80
		jnz short .32X__PWM_Pulse_C_full

		mov [PWM_FIFO_L + ebx * 2], ax
		mov [PWM_FIFO_R + ebx * 2], ax
		inc ebx
		and ebx, byte (_PWM_BUF_SIZE - 1)
		mov [PWM_WP_L], ebx
		mov [PWM_WP_R], ebx
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X__PWM_Pulse_C_full
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	.32X_VDP_Reg
		test byte [_32X_FM], 0xFF
		jnz near .32X_bad
		cmp ebx, 0xA15200
		jae near .32X_CRAM

		and ebx, 0xE
		jmp [.Table_32X_VDP_Reg + ebx * 2]

	ALIGN4

	.Table_32X_VDP_Reg
		dd .32X_VDP_Mode, .32X_VDP_Shift, .32X_VDP_AF_Len, .32X_VDP_AF_St
		dd .32X_VDP_AF_Data, .32X_VDP_State, .32X_bad, .32X_bad

	ALIGN32

	.32X_VDP_Mode
		mov [_32X_VDP + vx.Mode], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_VDP_Shift
		mov [_32X_VDP + vx.Mode + 2], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_VDP_AF_Len
		mov [_32X_VDP + vx.AF_Len], al
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_VDP_AF_St
		mov [_32X_VDP + vx.AF_St], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_VDP_AF_Data
		push rdi
		mov [_32X_VDP + vx.AF_Data], ax
		mov bx, ax
		mov edi, [_32X_VDP + vx.State]
		shl eax, 16
		and edi, byte 1
		mov ax, bx
		xor edi, byte 1
		mov ebx, [_32X_VDP + vx.AF_St]
		mov ecx, [_32X_VDP + vx.AF_Len]
		shl edi, 17
		inc ecx
		shr ecx, 1
		lea edi, [edi + _32X_VDP_Ram]
		jz short .Spec_Fill
		jnc short .Loop

		mov [edi + ebx * 2], ax
		inc bl
		jmp short .Loop

	ALIGN32

	.Loop
		mov [edi + ebx * 2], eax
		add bl, byte 2
		dec ecx
		jns short .Loop

		mov [_32X_VDP + vx.AF_St], ebx
		pop rdi
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.Spec_Fill
		mov [edi + ebx * 2], ax
		inc bl
		pop rdi
		mov [_32X_VDP + vx.AF_St], ebx
;		pop rcx
;		pop rbx
		ret


	ALIGN4

	.32X_VDP_State
		mov bh, [_32X_VDP + vx.Mode + 0]
		mov bl, [_32X_VDP + vx.State + 1]
		test bh, 3
		mov [_32X_VDP + vx.State + 2], al
		jz short .32X_VDP_blank

		test bl, bl
		jns short .32X_VDP_State_nvb

	.32X_VDP_blank
		mov [_32X_VDP + vx.State + 0], al
		call _32X_Set_FB

	.32X_VDP_State_nvb
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.32X_bad
;		pop rcx
;		pop rbx
		ret

	ALIGN32

	.32X_CRAM
		cmp ebx, 0xA15400
		jae short .32X_bad
		push rdx

		and eax, 0xFFFF
		mov cx, [_32X_Palette_16B + eax * 2]
		mov edx, [_32X_Palette_32B + eax * 4]
		mov [_32X_VDP_CRam + ebx - 0xA15200], ax
		mov [_32X_VDP_CRam_Ajusted + ebx - 0xA15200], cx
		mov [_32X_VDP_CRam_Ajusted32 + (ebx - 0xA15200) * 2],edx

		pop rdx
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	DECL M68K_Write_Word_32X_FB0
		and ebx, 0x3FFFE
		test ebx, 0x20000
		jnz short .overwrite

		mov [_32X_VDP_Ram + ebx], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.overwrite
		test al, al
		jz short .blank1

		mov [_32X_VDP_Ram + ebx - 0x20000 + 0], al

	.blank1
		test ah, ah
		jz short .blank2

		mov [_32X_VDP_Ram + ebx - 0x20000 + 1], ah

	.blank2
;		pop rcx
;		pop rbx
		ret


	ALIGN32

	DECL M68K_Write_Word_32X_FB1
		and ebx, 0x3FFFE
		test ebx, 0x20000
		jnz short .overwrite

		mov [_32X_VDP_Ram + ebx + 0x20000], ax
;		pop rcx
;		pop rbx
		ret

	ALIGN4

	.overwrite
		test al, al
		jz short .blank1

		mov [_32X_VDP_Ram + ebx - 0x20000 + 0x20000 + 0], al

	.blank1
		test ah, ah
		jz short .blank2

		mov [_32X_VDP_Ram + ebx - 0x20000 + 0x20000 + 1], ah

	.blank2
;		pop rcx
;		pop rbx
		ret



