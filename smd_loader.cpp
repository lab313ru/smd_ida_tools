/*
*      Interactive disassembler (IDA).
*      Copyright (c) 1990-2000 by Ilfak Guilfanov, <ig@datarescue.com>
*      ALL RIGHTS RESERVED.
*
*/

#define VERSION "1.0.3"
/*
*      SEGA MEGA DRIVE/GENESIS ROMs Loader (Modified/Updated HardwareMan's source)
*      Author: Dr. MefistO [Lab 313] <meffi@lab313.ru>, v1.0, 07/02/2015
*/

#define _CRT_SECURE_NO_WARNINGS

#include <ida.hpp>
#include <idp.hpp>
#include <entry.hpp>
#include <diskio.hpp>
#include <loader.hpp>
#include <auto.hpp>
#include <name.hpp>
#include <bytes.hpp>
#include <struct.hpp>
#include <enum.hpp>

#include "smd_loader.h"

static gen_hdr _hdr;
static gen_vect _vect;

struct reg {
	asize_t size;
	ea_t addr;
	char *name;
};

static const char *VECTOR_NAMES[] = {
	"SSP", "Reset", "BusErr", "AdrErr", "InvOpCode", "DivBy0", "Check", "TrapV", "GPF", "Trace", "Reserv0", "Reserv1", "Reserv2", "Reserv3",
	"Reserv4", "BadInt", "Reserv10", "Reserv11", "Reserv12", "Reserv13", "Reserv14", "Reserv15", "Reserv16", "Reserv17", "BadIRQ", "IRQ1",
	"EXT", "IRQ3", "HBLANK", "IRQ5", "VBLANK", "IRQ7", "Trap0", "Trap1", "Trap2", "Trap3", "Trap4", "Trap5", "Trap6", "Trap7", "Trap8",
	"Trap9", "Trap10", "Trap11", "Trap12", "Trap13", "Trap14", "Trap15", "Reserv30", "Reserv31", "Reserv32", "Reserv33", "Reserv34",
	"Reserv35", "Reserv36", "Reserv37", "Reserv38", "Reserv39", "Reserv3A", "Reserv3B", "Reserv3C", "Reserv3D", "Reserv3E", "Reserv3F"
};

static const reg spec_regs[] = {
	{ 4, 0xA04000, "Z80_YM2612" }, { 2, 0xA10000, "IO_PCBVER" }, { 2, 0xA10002, "IO_CT1_DATA" }, { 2, 0xA10004, "IO_CT2_DATA" },
	{ 2, 0xA10006, "IO_EXT_DATA" }, { 2, 0xA10008, "IO_CT1_CTRL" }, { 2, 0xA1000A, "IO_CT2_CTRL" }, { 2, 0xA1000C, "IO_EXT_CTRL" },
	{ 2, 0xA1000E, "IO_CT1_RX" }, { 2, 0xA10010, "IO_CT1_TX" }, { 2, 0xA10012, "IO_CT1_SMODE" }, { 2, 0xA10014, "IO_CT2_RX" },
	{ 2, 0xA10016, "IO_CT2_TX" }, { 2, 0xA10018, "IO_CT2_SMODE" }, { 2, 0xA1001A, "IO_EXT_RX" }, { 2, 0xA1001C, "IO_EXT_TX" },
	{ 2, 0xA1001E, "IO_EXT_SMODE" }, { 2, 0xA11000, "IO_RAMMODE" }, { 2, 0xA11100, "IO_Z80BUS" }, { 2, 0xA11200, "IO_Z80RES" },
	{ 0x100, 0xA12000, "IO_FDC" }, { 0x100, 0xA13000, "IO_TIME" }, { 4, 0xA14000, "IO_TMSS" }, { 2, 0xC00000, "VDP_DATA" },
	{ 2, 0xC00002, "VDP__DATA" }, { 2, 0xC00004, "VDP_CTRL" }, { 2, 0xC00006, "VDP__CTRL" }, { 2, 0xC00008, "VDP_CNTR" },
	{ 2, 0xC0000A, "VDP__CNTR" }, { 2, 0xC0000C, "VDP___CNTR" }, { 2, 0xC0000E, "VDP____CNTR" }, { 2, 0xC00011, "VDP_PSG" },
};

static const char M68K[] = "68000";
static const char CODE[] = "CODE";
static const char DATA[] = "DATA";
static const char ROM[] = "ROM";
static const char EPA[] = "EPA";
static const char S32X[] = "S32X";
static const char Z80[] = "Z80";
static const char Z80_RAM[] = "Z80_RAM";
static const char REGS[] = "REGS";
static const char Z80C[] = "Z80C";
static const char ASSR[] = "ASSR";
static const char UNLK[] = "UNLK";
static const char VDP[] = "VDP";
static const char RAM[] = "RAM";
static const char M68K_RAM[] = "M68K_RAM";
static const char M68K_RAM_[] = "M68K__RAM";
static const char SRAM[] = "SRAM";
static const char VECTORS[] = "vectors";
static const char VECTORS_STRUCT[] = "struct_vectors";
static const char HEADER[] = "header";
static const char HEADER_STRUCT[] = "struct_header";
static const char SETUP[] = "setup";

//------------------------------------------------------------------------
static unsigned int SWAP_BYTES_32(unsigned int a)
{
	return ((a >> 24) & 0x000000FF) | ((a >> 8) & 0x0000FF00) | ((a << 8) & 0x00FF0000) | ((a << 24) & 0xFF000000); // Swap dword LE to BE
}

//------------------------------------------------------------------------
static unsigned short READ_BE_WORD(unsigned char *addr)
{
	return (addr[0] << 8) | addr[1]; // Read BE word
}

//------------------------------------------------------------------------
static unsigned int READ_BE_UINT(unsigned char *addr)
{
	return (READ_BE_WORD(&addr[0]) << 16) | READ_BE_WORD(&addr[2]); // Read BE unsigned int by pointer
}

//------------------------------------------------------------------------
static void add_sub(unsigned int addr, const char *name, unsigned int max)
{
	if (!((addr >= 0x200) && (addr < max))) return;

	ea_t e_addr = toEA(ask_selector(0), addr);
	auto_make_proc(e_addr);
	set_name(e_addr, name);
}

//------------------------------------------------------------------------
static void add_offset_field(struc_t *st, segment_t *code_segm, const char *name)
{
	opinfo_t info = { 0 };
	info.ri.init(REF_OFF32, BADADDR);
	add_struc_member(st, name, BADADDR, dwrdflag() | offflag(), &info, 4);
}

//------------------------------------------------------------------------
static void add_dword_field(struc_t *st, const char *name)
{
	add_struc_member(st, name, BADADDR, dwrdflag() | hexflag(), NULL, 4);
}

//------------------------------------------------------------------------
static void add_string_field(struc_t *st, const char *name, asize_t size)
{
	opinfo_t info = { 0 };
	info.strtype = ASCSTR_C;
	add_struc_member(st, name, BADADDR, asciflag(), &info, size);
}

//------------------------------------------------------------------------
static void add_short_field(struc_t *st, const char *name)
{
	add_struc_member(st, name, BADADDR, wordflag() | hexflag(), NULL, 2);
}

//------------------------------------------------------------------------
static void add_dword_array(struc_t *st, const char *name, asize_t length)
{
	add_struc_member(st, name, BADADDR, dwrdflag() | hexflag(), NULL, 4 * length);
}

//------------------------------------------------------------------------
static void add_segment(ea_t start, ea_t end, const char *name, const char *class_name, const char *cmnt)
{
	if (!add_segm(0, start, end, name, class_name)) loader_failure();
	segment_t *segm = getseg(start);
	set_segment_cmt(segm, cmnt, false);
	doByte(start, 1);
}

//------------------------------------------------------------------------
static void add_subroutines(gen_vect *table, unsigned int rom_size)
{
	for (int i = 1; i < 64; i++) // except SPP pointer
	{
		add_sub(table->vectors[i], VECTOR_NAMES[i], rom_size);
	}
}

//------------------------------------------------------------------------
static void convert_vector_addrs(gen_vect *table)
{
	for (int i = 0; i < 64; i++)
	{
		table->vectors[i] = SWAP_BYTES_32(table->vectors[i]);
	}
}

//------------------------------------------------------------------------
static void define_vectors_struct()
{
	segment_t *code_segm = getseg(0);
	tid_t vec_id = add_struc(BADADDR, VECTORS_STRUCT);
	struc_t *vectors = get_struc(vec_id);

	for (int i = 0; i < 64; i++)
	{
		add_offset_field(vectors, code_segm, VECTOR_NAMES[i]);
	}

	doStruct(0, sizeof(gen_vect), vec_id);
	set_name(0, VECTORS);
}

//--------------------------------------------------------------------------
static void add_enum_member_with_mask(enum_t id, const char *name, unsigned int value, unsigned int mask = DEFMASK, const char *cmt = NULL)
{
	int res = add_const(id, name, value, mask); // we have to use old name, because of IDA v5.2
	if (cmt != NULL) set_enum_member_cmt(get_const_by_name(name), cmt, false);
}

//------------------------------------------------------------------------
static void set_spec_register_names()
{
	for (int i = 0; i <= 31; i++)
	{
		if (spec_regs[i].size == 2)
		{
			doWord(spec_regs[i].addr, 2);
		}
		else if (spec_regs[i].size == 4)
		{
			doDwrd(spec_regs[i].addr, 4);
		}
		else
		{
			doByte(spec_regs[i].addr, spec_regs[i].size);
		}
		set_name(spec_regs[i].addr, spec_regs[i].name);
	}
}

//------------------------------------------------------------------------
static void define_header_struct()
{
	tid_t head_id = add_struc(BADADDR, HEADER_STRUCT);
	struc_t *header = get_struc(head_id);

	add_string_field(header, "CopyRights", 32);
	add_string_field(header, "DomesticName", 48);
	add_string_field(header, "OverseasName", 48);
	add_string_field(header, "ProductCode", 14);
	add_short_field(header, "Checksum");
	add_string_field(header, "Peripherials", 16);
	add_dword_field(header, "RomStart");
	add_dword_field(header, "RomEnd");
	add_dword_field(header, "RamStart");
	add_dword_field(header, "RamEnd");
	add_string_field(header, "SramCode", 12);
	add_string_field(header, "ModemCode", 12);
	add_string_field(header, "Reserved", 40);
	add_string_field(header, "CountryCode", 16);
	doStruct(0x100, sizeof(gen_hdr), head_id);
	set_name(0x100, HEADER);
}

//------------------------------------------------------------------------
static void make_segments()
{
	add_segment(0x00000000, 0x003FFFFF + 1, ROM, CODE, "ROM segment");
	add_segment(0x00400000, 0x007FFFFF + 1, EPA, DATA, "Expansion Port Area (used by the Sega CD)");
	add_segment(0x00800000, 0x009FFFFF + 1, S32X, DATA, "Unallocated (used by the Sega 32X)");
	add_segment(0x00A00000, 0x00A0FFFF + 1, Z80, DATA, "Z80 Memory");
	add_segment(0x00A10000, 0x00A10FFF + 1, REGS, DATA, "System registers");
	add_segment(0x00A11000, 0x00A11FFF + 1, Z80C, DATA, "Z80 control (/BUSREQ and /RESET lines)");
	add_segment(0x00A12000, 0x00AFFFFF + 1, ASSR, DATA, "Assorted registers");
	add_segment(0x00B00000, 0x00BFFFFF + 1, UNLK, DATA, "Unallocated");

	for (int i = 0; i < 0x1; i++)
	{
		add_segment(0x00C00000 + i * 0x20, 0x00C00000 + i * 0x20 + 0x1F + 1, VDP, DATA, "VDP mirror");
	}

	for (int i = 0; i < 0x20; i++)
	{
		add_segment(0x00E00000 + i * 0x10000, 0x00E00000 + i * 0x10000 + 0xFFFF + 1, RAM, DATA, "RAM mirror");
	}
	add_segment(0xFFFF0000, 0xFFFFFFFE, RAM, DATA, "RAM mirror");

	set_name(0x00A00000, Z80_RAM);
	set_name(0x00FF0000, M68K_RAM);
	set_name(0xFFFF0000, M68K_RAM_);

	// create SRAM segment
	if ((READ_BE_WORD(&(_hdr.SramCode[0])) == 0x5241) && (_hdr.SramCode[2] == 0x20))
	{
		unsigned int sram_s = READ_BE_UINT(&(_hdr.SramCode[4])); // read SRAM start addr
		unsigned int sram_e = READ_BE_UINT(&(_hdr.SramCode[8])); // read SRAM end addr

		if ((sram_s >= 0x400000) && (sram_e <= 0x9FFFFF) && (sram_s < sram_e))
		{
			add_segment(sram_s, sram_e, SRAM, DATA, "SRAM memory");
		}
	}
}

//--------------------------------------------------------------------------
static void add_other_enum(enum_t ot)
{
	add_enum_member_with_mask(ot, "ROM_START", 0x200);
	add_enum_member_with_mask(ot, "IO_PCBVER_REF", 0x10FF);
	add_enum_member_with_mask(ot, "IO_TMSS_REF", 0x2F00); // IO_TMSS
}

//--------------------------------------------------------------------------
static void add_ccr_enum(enum_t ccr)
{
	// bits 7..5 is clear
	add_enum_member_with_mask(ccr, "X", (1 << 4), (1 << 4), "Extend flag");
	add_enum_member_with_mask(ccr, "N", (1 << 3), (1 << 3), "Negative flag");
	add_enum_member_with_mask(ccr, "Z", (1 << 2), (1 << 2), "Zero flag");
	add_enum_member_with_mask(ccr, "V", (1 << 1), (1 << 1), "Overflow flag");
	add_enum_member_with_mask(ccr, "C", (1 << 0), (1 << 0), "Carry flag");
}

//--------------------------------------------------------------------------
static void add_sr_enum(enum_t sr)
{
	add_enum_member_with_mask(sr, "T1", (1 << 15), (1 << 15), "Trace bit 2. If set, trace is allowed on any instruction");
	add_enum_member_with_mask(sr, "T0", (1 << 14), (1 << 14), "Trace bit 1. If set, trace on change of program flow");
	add_enum_member_with_mask(sr, "SF", (1 << 13), (1 << 13), "Supervisor Mode flag. If clear, SP refers to USP.\nIf set, look at M to determine what stack SP points to");
	add_enum_member_with_mask(sr, "MF", (1 << 12), (1 << 12), "Always clear (???)");
	// bit 11 is clear
	add_enum_member_with_mask(sr, "DISABLE_ALL_INTERRUPTS", (7 << 8) /*111*/, (7 << 8));
	add_enum_member_with_mask(sr, "ENABLE_NO_INTERRUPTS", (6 << 8)  /*110*/, (7 << 8));

	add_enum_member_with_mask(sr, "DISABLE_ALL_INTERRUPTS_EXCEPT_VBLANK", (5 << 8)  /*101*/, (7 << 8));
	add_enum_member_with_mask(sr, "ENABLE_ONLY_VBLANK_INTERRUPT", (4 << 8)  /*100*/, (7 << 8));

	add_enum_member_with_mask(sr, "DISABLE_ALL_INTERRUPTS_EXCEPT_VBLANK_HBLANK", (3 << 8)  /*011*/, (7 << 8));
	add_enum_member_with_mask(sr, "ENABLE_ONLY_VBLANK_HBLANK_INTERRUPTS", (2 << 8)  /*010*/, (7 << 8));

	add_enum_member_with_mask(sr, "DISABLE_NO_INTERRUPTS", (1 << 8)  /*001*/, (7 << 8));
	add_enum_member_with_mask(sr, "ENABLE_ALL_INTERRUPTS", (0 << 8)  /*000*/, (7 << 8));
}

static void add_vdp_status_enum(enum_t vdp_status)
{
	add_enum_member_with_mask(vdp_status, "FIFO_EMPTY", (1 << 9), (1 << 9));
	add_enum_member_with_mask(vdp_status, "FIFO_FULL", (1 << 8), (1 << 8));
	add_enum_member_with_mask(vdp_status, "VBLANK_PENDING", (1 << 7), (1 << 7));
	add_enum_member_with_mask(vdp_status, "SPRITE_OVERFLOW", (1 << 6), (1 << 6));
	add_enum_member_with_mask(vdp_status, "SPRITE_COLLISION", (1 << 5), (1 << 5));
	add_enum_member_with_mask(vdp_status, "ODD_FRAME", (1 << 4), (1 << 4));
	add_enum_member_with_mask(vdp_status, "VBLANKING", (1 << 3), (1 << 3));
	add_enum_member_with_mask(vdp_status, "HBLANKING", (1 << 2), (1 << 2));
	add_enum_member_with_mask(vdp_status, "DMA_IN_PROGRESS", (1 << 1), (1 << 1));
	add_enum_member_with_mask(vdp_status, "PAL_MODE", (1 << 0), (1 << 0));
}

static void add_vdp_regs_send_enum(enum_t vdp_regs_send_enum)
{
	char buf[100];

	unsigned int reg_value = (((2/*10*/ << 1 /*ANY BIT*/) << 5 /*REG NUM BITS*/) | 0 /*REG IDX*/) << 8 /*REG SEND DATA BITS*/; // REG $00
	add_enum_member_with_mask(vdp_regs_send_enum, "DISPLAY_OFF", (reg_value | (1 /*SET*/ << 0 /*BIT 0*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISPLAY__OFF", (reg_value | (1 /*SET*/ << 0 /*BIT 0*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISPLAY_ON", (reg_value | (0 /*CLEAR*/ << 0 /*BIT 0*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISPLAY__ON", (reg_value | (0 /*CLEAR*/ << 0 /*BIT 0*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	add_enum_member_with_mask(vdp_regs_send_enum, "PAUSE_HV_WHEN_EXT_INT_HAPPENS", (reg_value | (1 /*SET*/ << 1 /*BIT 1*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "PAUSE_HV_WHEN_EXT_INT__HAPPENS", (reg_value | (1 /*SET*/ << 1 /*BIT 1*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	add_enum_member_with_mask(vdp_regs_send_enum, "NORMAL_HV_COUNTER", (reg_value | (0 /*CLEAR*/ << 1 /*BIT 1*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "NORMAL_HV__COUNTER", (reg_value | (0 /*CLEAR*/ << 1 /*BIT 1*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	add_enum_member_with_mask(vdp_regs_send_enum, "EIGHT_COLORS_MODE", (reg_value | (1 /*SET*/ << 2 /*BIT 2*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "EIGHT_COLORS__MODE", (reg_value | (1 /*SET*/ << 2 /*BIT 2*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	add_enum_member_with_mask(vdp_regs_send_enum, "FULL_COLORS_MODE", (reg_value | (0 /*CLEAR*/ << 2 /*BIT 2*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "FULL_COLORS__MODE", (reg_value | (0 /*CLEAR*/ << 2 /*BIT 2*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	add_enum_member_with_mask(vdp_regs_send_enum, "ENABLE_HBLANK", (reg_value | (1 /*SET*/ << 4 /*BIT 4*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "ENABLE__HBLANK", (reg_value | (1 /*SET*/ << 4 /*BIT 4*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISABLE_HBLANK", (reg_value | (0 /*CLEAR*/ << 4 /*BIT 4*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISABLE__HBLANK", (reg_value | (0 /*CLEAR*/ << 4 /*BIT 4*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	reg_value = (((2/*10*/ << 1 /*ANY BIT*/) << 5 /*REG NUM BITS*/) | 1 /*REG IDX*/) << 8 /*REG SEND DATA BITS*/; // REG $01
	add_enum_member_with_mask(vdp_regs_send_enum, "GENESIS_DISP_MODE", (reg_value | (1 /*SET*/ << 2 /*BIT 2*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "GENESIS_DISP__MODE", (reg_value | (1 /*SET*/ << 2 /*BIT 2*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	add_enum_member_with_mask(vdp_regs_send_enum, "SMS_DISP_MODE", (reg_value | (0 /*CLEAR*/ << 2 /*BIT 2*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "SMS_DISP__MODE", (reg_value | (0 /*CLEAR*/ << 2 /*BIT 2*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	add_enum_member_with_mask(vdp_regs_send_enum, "SET_PAL_MODE", (reg_value | (1 /*SET*/ << 3 /*BIT 3*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "SET_PAL__MODE", (reg_value | (1 /*SET*/ << 3 /*BIT 3*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	add_enum_member_with_mask(vdp_regs_send_enum, "SET_NTSC_MODE", (reg_value | (0 /*CLEAR*/ << 3 /*BIT 3*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "SET_NTSC__MODE", (reg_value | (0 /*CLEAR*/ << 3 /*BIT 3*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	add_enum_member_with_mask(vdp_regs_send_enum, "ENABLE_DMA", (reg_value | (1 /*SET*/ << 4 /*BIT 4*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "ENABLE__DMA", (reg_value | (1 /*SET*/ << 4 /*BIT 4*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISABLE_DMA", (reg_value | (0 /*CLEAR*/ << 4 /*BIT 4*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISABLE__DMA", (reg_value | (0 /*CLEAR*/ << 4 /*BIT 4*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	add_enum_member_with_mask(vdp_regs_send_enum, "ENABLE_VBLANK", (reg_value | (1 /*SET*/ << 5 /*BIT 5*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "ENABLE__VBLANK", (reg_value | (1 /*SET*/ << 5 /*BIT 5*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISABLE_VBLANK", (reg_value | (0 /*CLEAR*/ << 5 /*BIT 5*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISABLE__VBLANK", (reg_value | (0 /*CLEAR*/ << 5 /*BIT 5*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	add_enum_member_with_mask(vdp_regs_send_enum, "ENABLE_DISPLAY", (reg_value | (1 /*SET*/ << 6 /*BIT 6*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "ENABLE__DISPLAY", (reg_value | (1 /*SET*/ << 6 /*BIT 6*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISABLE_DISPLAY", (reg_value | (0 /*CLEAR*/ << 6 /*BIT 6*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "DISABLE__DISPLAY", (reg_value | (0 /*CLEAR*/ << 6 /*BIT 6*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	add_enum_member_with_mask(vdp_regs_send_enum, "TMS9918_DISP_MODE", (reg_value | (1 /*SET*/ << 7 /*BIT 7*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "TMS9918_DISP__MODE", (reg_value | (1 /*SET*/ << 7 /*BIT 7*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	add_enum_member_with_mask(vdp_regs_send_enum, "GENESIS__DISP_MODE", (reg_value | (0 /*CLEAR*/ << 7 /*BIT 7*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "GENESIS__DISP__MODE", (reg_value | (0 /*CLEAR*/ << 7 /*BIT 7*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	reg_value = (((2/*10*/ << 1 /*ANY BIT*/) << 5 /*REG NUM BITS*/) | 2 /*REG IDX*/) << 8 /*REG SEND DATA BITS*/; // REG $02
	for (int i = 0; i < (1 << 3); i++)
	{
		qsnprintf(buf, sizeof(buf), "SET_PLANE_A_ADDR_0x%.4X", i * 0x400);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | (i << 3)) << 0, 0x9FFF /*10?XXXXX11111111*/);
		qsnprintf(buf, sizeof(buf), "SET_PLANE_A_ADDR__0x%.4X", i * 0x400);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | (i << 3)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	}

	reg_value = (((2/*10*/ << 1 /*ANY BIT*/) << 5 /*REG NUM BITS*/) | 3 /*REG IDX*/) << 8 /*REG SEND DATA BITS*/; // REG $03
	for (int i = 0; i < (1 << 3); i++)
	{
		qsnprintf(buf, sizeof(buf), "SET_WND_PLANE_ADDR_0x%.4X", i * 0x400);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | (i << 3)) << 0, 0x9FFF /*10?XXXXX11111111*/);
		qsnprintf(buf, sizeof(buf), "SET_WND_PLANE_ADDR__0x%.4X", i * 0x400);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | (i << 3)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	}

	reg_value = (((2/*10*/ << 1 /*ANY BIT*/) << 5 /*REG NUM BITS*/) | 4 /*REG IDX*/) << 8 /*REG SEND DATA BITS*/; // REG $04
	for (int i = 0; i < (1 << 3); i++)
	{
		qsnprintf(buf, sizeof(buf), "SET_PLANE_B_ADDR_0x%.4X", i * 0x2000);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | i) << 0, 0x9FFF /*10?XXXXX11111111*/);
		qsnprintf(buf, sizeof(buf), "SET_PLANE_B_ADDR__0x%.4X", i * 0x2000);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | i) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	}

	reg_value = (((2/*10*/ << 1 /*ANY BIT*/) << 5 /*REG NUM BITS*/) | 5 /*REG IDX*/) << 8 /*REG SEND DATA BITS*/; // REG $05
	for (int i = 0; i < (1 << 7); i++)
	{
		qsnprintf(buf, sizeof(buf), "SET_SPRITE_TBL_ADDR_0x%.4X", i * 0x200);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | i) << 0, 0x9FFF /*10?XXXXX11111111*/);
		qsnprintf(buf, sizeof(buf), "SET_SPRITE_TBL_ADDR__0x%.4X", i * 0x200);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | i) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	}

	reg_value = (((2/*10*/ << 1 /*ANY BIT*/) << 5 /*REG NUM BITS*/) | 6 /*REG IDX*/) << 8 /*REG SEND DATA BITS*/; // REG $06
	add_enum_member_with_mask(vdp_regs_send_enum, "ENABLE_SPRITES_REBASE", (reg_value | (1 /*SET*/ << 5 /*BIT 5*/)) << 0, 0x9FFF /*10?XXXXX11111111*/);
	add_enum_member_with_mask(vdp_regs_send_enum, "ENABLE_SPRITES__REBASE", (reg_value | (1 /*SET*/ << 5 /*BIT 5*/)) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);

	reg_value = (((2/*10*/ << 1 /*ANY BIT*/) << 5 /*REG NUM BITS*/) | 7 /*REG IDX*/) << 8 /*REG SEND DATA BITS*/; // REG $07
	for (int xx = 0; xx < (1 << 2); xx++)
	{
		for (int yyyy = 0; yyyy < (1 << 4); yyyy++)
		{
			qsnprintf(buf, sizeof(buf), "SET_BG_AS_%dPAL_%dTH_COLOR", xx + 1, yyyy + 1);
			add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | ((xx << 4) | yyyy) /*00XXYYYY*/) << 0, 0x9FFF /*10?XXXXX11111111*/);
			qsnprintf(buf, sizeof(buf), "SET_BG_AS__%dPAL_%dTH_COLOR", xx + 1, yyyy + 1);
			add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | ((xx << 4) | yyyy) /*00XXYYYY*/) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
		}
	}

	reg_value = (((2/*10*/ << 1 /*ANY BIT*/) << 5 /*REG NUM BITS*/) | 0x0A /*REG IDX*/) << 8 /*REG SEND DATA BITS*/; // REG $0A
	for (int i = 0; i < (1 << 8); i++)
	{
		qsnprintf(buf, sizeof(buf), "SET_HBLANK_COUNTER_VALUE_0x%.2X", i);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | i) << 0, 0x9FFF /*10?XXXXX11111111*/);
		qsnprintf(buf, sizeof(buf), "SET_HBLANK_COUNTER_VALUE__0x%.2X", i);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | i) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	}

	reg_value = (((2/*10*/ << 1 /*ANY BIT*/) << 5 /*REG NUM BITS*/) | 0x0F /*REG IDX*/) << 8 /*REG SEND DATA BITS*/; // REG $0F
	for (int i = 0; i < (1 << 8); i++)
	{
		qsnprintf(buf, sizeof(buf), "SET_VDP_AUTO_INC_VALUE_0x%.2X", i);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | i) << 0, 0x9FFF /*10?XXXXX11111111*/);
		qsnprintf(buf, sizeof(buf), "SET_VDP_AUTO_INC_VALUE__0x%.2X", i);
		add_enum_member_with_mask(vdp_regs_send_enum, buf, (reg_value | i) << 16, 0x9FFF /*10?XXXXX11111111*/ << 16);
	}
}

//--------------------------------------------------------------------------
static void add_enums()
{
	enum_t ccr = add_enum(BADADDR, "enum_ccr", hexflag());
	set_enum_bf(ccr, true);
	add_ccr_enum(ccr);

	enum_t sr = add_enum(BADADDR, "enum_sr", hexflag());
	set_enum_bf(sr, true);
	add_sr_enum(sr);

	enum_t ot = add_enum(BADADDR, "enum_other", hexflag());
	add_other_enum(ot);

	enum_t vdp_status = add_enum(BADADDR, "enum_vdp_status", hexflag());
	set_enum_bf(vdp_status, true);
	add_vdp_status_enum(vdp_status);

	enum_t vdp_regs_send_enum = add_enum(BADADDR, "enum_vdp_regs_send", hexflag());
	set_enum_bf(vdp_regs_send_enum, true);
	add_vdp_regs_send_enum(vdp_regs_send_enum);
}

//--------------------------------------------------------------------------
static void print_version()
{
	static const char format[] = "Sega Genesis/Megadrive ROMs loader plugin v%s;\nAuthor: Dr. MefistO [Lab 313] <meffi@lab313.ru>.";
	info(format, VERSION);
	msg(format, VERSION);
}

//--------------------------------------------------------------------------
int idaapi accept_file(linput_t *li, char fileformatname[MAX_FILE_FORMAT_NAME], int n)
{
	if (n != 0) return 0;
	qlseek(li, 0, SEEK_SET);
	if (qlread(li, &_vect, sizeof(_vect)) != sizeof(_vect)) return 0;
	if (qlread(li, &_hdr, sizeof(_hdr)) != sizeof(_hdr)) return 0;
	qstrncpy(fileformatname, "Sega Genesis/MegaDrive ROM file v.2", MAX_FILE_FORMAT_NAME);

	return 1;
}

//--------------------------------------------------------------------------
void idaapi load_file(linput_t *li, ushort neflags, const char *fileformatname)
{
	set_processor_type(M68K, SETPROC_ALL | SETPROC_FATAL); // Motorola 68000

	unsigned int size = qlsize(li); // size of rom

	qlseek(li, 0, SEEK_SET);
	if (qlread(li, &_vect, sizeof(_vect)) != sizeof(_vect)) loader_failure(); // trying to read rom vectors
	if (qlread(li, &_hdr, sizeof(_hdr)) != sizeof(_hdr)) loader_failure(); // trying to read rom's header

	file2base(li, 0, 0x0000000, size, FILEREG_PATCHABLE); // load rom to database

	make_segments(); // create ROM, RAM, Z80 RAM and etc. segments
	convert_vector_addrs(&_vect); // convert addresses of vectors from LE to BE
	define_vectors_struct(); // add definition of vectors struct
	define_header_struct(); // add definition of header struct
	set_spec_register_names(); // apply names for special addresses of registers
	add_subroutines(&_vect, size); // mark vector subroutines as procedures
	add_enums(); // add definition of sr enum

	inf.beginEA = get_name_ea(BADADDR, VECTOR_NAMES[1]); // Reset

	inf.af = 0
		| AF_FIXUP //        0x0001          // Create offsets and segments using fixup info
		| AF_MARKCODE  //     0x0002          // Mark typical code sequences as code
		| AF_UNK //          0x0004          // Delete instructions with no xrefs
		| AF_CODE //         0x0008          // Trace execution flow
		| AF_PROC //         0x0010          // Create functions if call is present
		| AF_USED //         0x0020          // Analyze and create all xrefs
		//| AF_FLIRT //        0x0040          // Use flirt signatures
		| AF_PROCPTR //      0x0080          // Create function if data xref data->code32 exists
		| AF_JFUNC //        0x0100          // Rename jump functions as j_...
		| AF_NULLSUB //      0x0200          // Rename empty functions as nullsub_...
		//| AF_LVAR //         0x0400          // Create stack variables
		//| AF_TRACE //        0x0800          // Trace stack pointer
		| AF_ASCII //        0x1000          // Create ascii string if data xref exists
		//| AF_IMMOFF //       0x2000          // Convert 32bit instruction operand to offset
		//AF_DREFOFF //      0x4000          // Create offset if data xref to seg32 exists
		//| AF_FINAL //       0x8000          // Final pass of analysis
		;
	inf.af2 = 0
		| AF2_JUMPTBL  //    0x0001          // Locate and create jump tables
		//| AF2_DODATA  //     0x0002          // Coagulate data segs at the final pass
		//| AF2_HFLIRT  //     0x0004          // Automatically hide library functions
		| AF2_STKARG  //     0x0008          // Propagate stack argument information
		| AF2_REGARG  //     0x0010          // Propagate register argument information
		//| AF2_CHKUNI  //     0x0020          // Check for unicode strings
		//| AF2_SIGCMT  //     0x0040          // Append a signature name comment for recognized anonymous library functions
		| AF2_SIGMLT  //     0x0080          // Allow recognition of several copies of the same function
		| AF2_FTAIL  //      0x0100          // Create function tails
		| AF2_DATOFF  //     0x0200          // Automatically convert data to offsets
		//| AF2_ANORET  //     0x0400          // Perform 'no-return' analysis
		//| AF2_VERSP  //      0x0800          // Perform full SP-analysis (ph.verify_sp)
		//| AF2_DOCODE  //     0x1000          // Coagulate code segs at the final pass
		| AF2_TRFUNC  //     0x2000          // Truncate functions upon code deletion
		//| AF2_PURDAT  //     0x4000          // Control flow to data segment is ignored
		//| AF2_MEMFUNC //    0x8000          // Try to guess member function types
		;

	print_version();
}

//--------------------------------------------------------------------------
loader_t LDSC =
{
	IDP_INTERFACE_VERSION,
	0,                            // loader flags
	//
	//      check input file format. if recognized, then return 1
	//      and fill 'fileformatname'.
	//      otherwise return 0
	//
	accept_file,
	//
	//      load file into the database.
	//
	load_file,
	//
	//      create output file from the database.
	//      this function may be absent.
	//
	NULL,
	//      take care of a moved segment (fix up relocations, for example)
	NULL
};
