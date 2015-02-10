/*
*      Interactive disassembler (IDA).
*      Copyright (c) 1990-2000 by Ilfak Guilfanov, <ig@datarescue.com>
*      ALL RIGHTS RESERVED.
*
*/

#define VERSION "1.0.5"
/*
*      SEGA MEGA DRIVE/GENESIS ROMs Loader (Modified/Updated HardwareMan's source)
*      Author: Dr. MefistO [Lab 313] <meffi@lab313.ru>
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
	if (res)
	{
		res = res;
	}
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

#define SET 1
#define CLEAR 0

static unsigned int form_value(unsigned char bit_idx, unsigned int bit, bool lower_word)
{
	return (bit << bit_idx) << (lower_word ? 0 : 16);
}

static void add_vdp_reg_mask(enum_t send_enum, unsigned char reg_idx, const char *name_lo, const char *name_hi)
{
	set_bmask_name(send_enum, 0x9F00, "REG_MASK_LO"); set_bmask_name(send_enum, 0x9F00 << 16, "REG_MASK_HI");
	set_bmask_name(send_enum, 0x00FF, "VAL_MASK_LO"); set_bmask_name(send_enum, 0x00FF << 16, "VAL_MASK_HI");

	add_enum_member_with_mask(send_enum, name_lo, (0x8000 + (reg_idx << 8)) << 0, 0x9F00 << 0);
	add_enum_member_with_mask(send_enum, name_hi, (0x8000 + (reg_idx << 8)) << 16, 0x9F00 << 16);
}

static void add_vdp_reg_bits(enum_t send_enum, unsigned char reg_idx, const char *name_lo, const char *name_hi, unsigned char bit_idx, unsigned int bits)
{
	add_enum_member_with_mask(send_enum, name_lo, form_value(bit_idx, bits, true), 0xFF << 0);
	add_enum_member_with_mask(send_enum, name_hi, form_value(bit_idx, bits, false), 0xFF << 16);
}

static void add_vdp_reg_0()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg00", hexflag());
	set_enum_bf(send_enum, true);

	add_vdp_reg_mask(send_enum, 0, "REG_00", "REG__00");

	add_vdp_reg_bits(send_enum, 0, "DISPLAY_OFF", "DISPLAY__OFF", 0, SET);
	add_vdp_reg_bits(send_enum, 0, "DISPLAY_ON", "DISPLAY__ON", 0, CLEAR);

	add_vdp_reg_bits(send_enum, 0, "PAUSE_HV_WHEN_EXT_INT_HAPPENS", "PAUSE_HV_WHEN_EXT_INT__HAPPENS", 1, SET);
	add_vdp_reg_bits(send_enum, 0, "NORMAL_HV_COUNTER", "NORMAL_HV__COUNTER", 1, CLEAR);

	add_vdp_reg_bits(send_enum, 0, "EIGHT_COLORS_MODE", "EIGHT_COLORS__MODE", 2, SET);
	add_vdp_reg_bits(send_enum, 0, "FULL_COLORS_MODE", "FULL_COLORS__MODE", 2, CLEAR);

	add_vdp_reg_bits(send_enum, 0, "ENABLE_HBLANK", "ENABLE__HBLANK", 4, SET);
	add_vdp_reg_bits(send_enum, 0, "DISABLE_HBLANK", "DISABLE__HBLANK", 4, CLEAR);
}

static void add_vdp_reg_1()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg01", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 1, "REG_01", "REG__01");

	add_vdp_reg_bits(send_enum, 1, "GENESIS_DISP_MODE", "GENESIS_DISP__MODE", 2, SET);
	add_vdp_reg_bits(send_enum, 1, "SMS_DISP_MODE", "SMS_DISP__MODE", 2, CLEAR);

	add_vdp_reg_bits(send_enum, 1, "SET_PAL_MODE", "SET_PAL__MODE", 3, SET);
	add_vdp_reg_bits(send_enum, 1, "SET_NTSC_MODE", "SET_NTSC__MODE", 3, CLEAR);

	add_vdp_reg_bits(send_enum, 1, "ENABLE_DMA", "ENABLE__DMA", 4, SET);
	add_vdp_reg_bits(send_enum, 1, "DISABLE_DMA", "DISABLE__DMA", 4, CLEAR);

	add_vdp_reg_bits(send_enum, 1, "ENABLE_VBLANK", "ENABLE__VBLANK", 5, SET);
	add_vdp_reg_bits(send_enum, 1, "DISABLE_VBLANK", "DISABLE__VBLANK", 5, CLEAR);

	add_vdp_reg_bits(send_enum, 1, "ENABLE_DISPLAY", "ENABLE__DISPLAY", 6, SET);
	add_vdp_reg_bits(send_enum, 1, "DISABLE_DISPLAY", "DISABLE__DISPLAY", 6, CLEAR);

	add_vdp_reg_bits(send_enum, 1, "TMS9918_DISP_MODE", "TMS9918_DISP__MODE", 7, SET);
	add_vdp_reg_bits(send_enum, 1, "GENESIS__DISP_MODE", "GENESIS__DISP__MODE", 7, CLEAR);
}

static void add_vdp_reg_2()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg02", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 2, "REG_02", "REG__02");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 3); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_PLANE_A_ADDR_$%.4X", (i << 3) * 0x400);
		qsnprintf(name_hi, sizeof(name_hi), "SET_PLANE_A_ADDR__$%.4X", (i << 3) * 0x400);
		add_vdp_reg_bits(send_enum, 2, name_lo, name_hi, 3, i);
	}
}

static void add_vdp_reg_3()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg03", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 3, "REG_03", "REG__03");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 3); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_WINDOW_PLANE_ADDR_$%.4X", (i << 3) * 0x400);
		qsnprintf(name_hi, sizeof(name_hi), "SET_WINDOW_PLANE_ADDR__$%.4X", (i << 3) * 0x400);
		add_vdp_reg_bits(send_enum, 3, name_lo, name_hi, 3, i);
	}
}

static void add_vdp_reg_4()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg04", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 4, "REG_04", "REG__04");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 3); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_PLANE_B_ADDR_$%.4X", i * 0x2000);
		qsnprintf(name_hi, sizeof(name_hi), "SET_PLANE_B_ADDR__$%.4X", i * 0x2000);
		add_vdp_reg_bits(send_enum, 4, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_5()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg05", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 5, "REG_05", "REG__05");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 7); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_SPRITE_TBL_ADDR_$%.4X", i * 0x200);
		qsnprintf(name_hi, sizeof(name_hi), "SET_SPRITE_TBL_ADDR__$%.4X", i * 0x200);
		add_vdp_reg_bits(send_enum, 5, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_6()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg06", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 6, "REG_06", "REG__06");

	add_vdp_reg_bits(send_enum, 6, "ENABLE_SPRITES_REBASE", "ENABLE_SPRITES__REBASE", 5, SET);
	add_vdp_reg_bits(send_enum, 6, "DISABLE_SPRITES_REBASE", "DISABLE_SPRITES__REBASE", 5, CLEAR);
}

static void add_vdp_reg_7()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg07", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 7, "REG_07", "REG__07");

	char name_lo[100], name_hi[100];
	for (int xx = 0; xx < (1 << 2); xx++)
	{
		for (int yyyy = 0; yyyy < (1 << 4); yyyy++)
		{
			qsnprintf(name_lo, sizeof(name_lo), "SET_BG_AS_%dPAL_%dTH_COLOR", xx + 1, yyyy + 1);
			qsnprintf(name_hi, sizeof(name_hi), "SET_BG_AS__%dPAL_%dTH_COLOR", xx + 1, yyyy + 1);
			add_vdp_reg_bits(send_enum, 7, name_lo, name_hi, 0, ((xx << 4) | yyyy));
		}
	}
}

static void add_vdp_reg_a()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg0A", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x0A, "REG_0A", "REG__0A");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 8); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_HBLANK_COUNTER_VALUE_$%.2X", i);
		qsnprintf(name_hi, sizeof(name_hi), "SET_HBLANK_COUNTER_VALUE__$%.2X", i);
		add_vdp_reg_bits(send_enum, 0x0A, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_b()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg0B", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x0B, "REG_0B", "REG__0B");

	add_vdp_reg_bits(send_enum, 0x0B, "SET_HSCROLL_TYPE_AS_FULLSCREEN", "SET_HSCROLL_TYPE_AS__FULLSCREEN", 0, 0 /*00*/);
	add_vdp_reg_bits(send_enum, 0x0B, "SET_HSCROLL_TYPE_AS_LINE_SCROLL", "SET_HSCROLL_TYPE_AS__LINE_SCROLL", 0, 1 /*01*/);
	add_vdp_reg_bits(send_enum, 0x0B, "SET_HSCROLL_TYPE_AS_CELL_SCROLL", "SET_HSCROLL_TYPE_AS__CELL_SCROLL", 0, 2 /*10*/);
	add_vdp_reg_bits(send_enum, 0x0B, "SET_HSCROLL_TYPE_AS_LINE__SCROLL", "SET_HSCROLL_TYPE_AS__LINE__SCROLL", 0, 3 /*11*/);

	add_vdp_reg_bits(send_enum, 0x0B, "_2CELLS_COLUMN_VSCROLL_MODE", "_2CELLS_COLUMN_VSCROLL__MODE", 2, SET);
	add_vdp_reg_bits(send_enum, 0x0B, "FULLSCREEN_VSCROLL_MODE", "FULLSCREEN_VSCROLL__MODE", 2, CLEAR);

	add_vdp_reg_bits(send_enum, 0x0B, "ENABLE_EXT_INTERRUPT", "ENABLE_EXT__INTERRUPT", 3, SET);
	add_vdp_reg_bits(send_enum, 0x0B, "DISABLE_EXT_INTERRUPT", "DISABLE_EXT__INTERRUPT", 3, CLEAR);
}

static void add_vdp_reg_c()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg0C", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x0C, "REG_0C", "REG__0C");

	add_vdp_reg_bits(send_enum, 0x0C, "SET_32_TILES_WIDTH_MODE", "SET_32_TILES_WIDTH__MODE", 0, ((0 << 7) | (0 << 0)));
	add_vdp_reg_bits(send_enum, 0x0C, "SET_40_TILES_WIDTH_MODE", "SET_40_TILES_WIDTH__MODE", 0, ((1 << 7) | (1 << 0)));

	add_vdp_reg_bits(send_enum, 0x0C, "ENABLE_SHADOW_HIGHLIGHT_MODE", "ENABLE_SHADOW_HIGHLIGHT__MODE", 3, SET);
	add_vdp_reg_bits(send_enum, 0x0C, "DISABLE_SHADOW_HIGHLIGHT_MODE", "DISABLE_SHADOW_HIGHLIGHT__MODE", 3, CLEAR);

	add_vdp_reg_bits(send_enum, 0x0C, "NO_INTERLACE_MODE", "NO_INTERLACE__MODE", 1, 0 /*00*/);
	add_vdp_reg_bits(send_enum, 0x0C, "ENABLE_SIMPLE_INTERLACE_MODE", "ENABLE_SIMPLE_INTERLACE__MODE", 1, 1 /*01*/);
	add_vdp_reg_bits(send_enum, 0x0C, "ENABLE_DOUBLE_INTERLACE_MODE", "ENABLE_DOUBLE_INTERLACE__MODE", 1, 3 /*11*/);

	add_vdp_reg_bits(send_enum, 0x0C, "ENABLE_EXTERNAL_PIXEL_BUS", "ENABLE_EXTERNAL_PIXEL__BUS", 4, SET);
	add_vdp_reg_bits(send_enum, 0x0C, "DISABLE_EXTERNAL_PIXEL_BUS", "DISABLE_EXTERNAL_PIXEL__BUS", 4, CLEAR);

	add_vdp_reg_bits(send_enum, 0x0C, "DO_PIXEL_CLOCK_INSTEAD_OF_VSYNC", "DO_PIXEL_CLOCK_INSTEAD_OF__VSYNC", 6, SET);
	add_vdp_reg_bits(send_enum, 0x0C, "DO_VSYNC_INSTEAD_OF_PIXEL_CLOCK", "DO_VSYNC_INSTEAD_OF_PIXEL__CLOCK", 6, CLEAR);
}

static void add_vdp_reg_d()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg0D", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x0D, "REG_0D", "REG__0D");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 6); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_HSCROLL_DATA_ADDR_$%.2X", i * 0x400);
		qsnprintf(name_hi, sizeof(name_hi), "SET_HSCROLL_DATA_ADDR__$%.2X", i * 0x400);
		add_vdp_reg_bits(send_enum, 0x0D, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_e()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg0E", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x0E, "REG_0E", "REG__0E");

	add_vdp_reg_bits(send_enum, 0x0E, "ENABLE_PLANE_A_REBASE", "ENABLE_PLANE_A__REBASE", 0, SET);
	add_vdp_reg_bits(send_enum, 0x0E, "DISABLE_PLANE_A_REBASE", "DISABLE_PLANE_A__REBASE", 0, CLEAR);

	add_vdp_reg_bits(send_enum, 0x0E, "ENABLE_PLANE_B_REBASE", "ENABLE_PLANE_B__REBASE", 4, SET);
	add_vdp_reg_bits(send_enum, 0x0E, "DISABLE_PLANE_B_REBASE", "DISABLE_PLANE_B__REBASE", 4, CLEAR);
}

static void add_vdp_reg_f()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg0F", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x0F, "REG_0F", "REG__0F");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 8); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_VDP_AUTO_INC_VALUE_$%.2X", i * 0x400);
		qsnprintf(name_hi, sizeof(name_hi), "SET_VDP_AUTO_INC_VALUE__$%.2X", i * 0x400);
		add_vdp_reg_bits(send_enum, 0x0F, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_10()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg10", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x10, "REG_10", "REG__10");

	add_vdp_reg_bits(send_enum, 0x10, "SET_PLANEA_PLANEB_WIDTH_TO_32_TILES", "SET_PLANEA_PLANEB_WIDTH_TO_32__TILES", 0, 0 /*00*/);
	add_vdp_reg_bits(send_enum, 0x10, "SET_PLANEA_PLANEB_WIDTH_TO_64_TILES", "SET_PLANEA_PLANEB_WIDTH_TO_64__TILES", 0, 1 /*01*/);
	add_vdp_reg_bits(send_enum, 0x10, "SET_PLANEA_PLANEB_WIDTH_TO_128_TILES", "SET_PLANEA_PLANEB_WIDTH_TO_128__TILES", 0, 3 /*11*/);

	add_vdp_reg_bits(send_enum, 0x10, "SET_PLANEA_PLANEB_HEIGHT_TO_32_TILES", "SET_PLANEA_PLANEB_HEIGHT_TO_32__TILES", 4, 0 /*00*/);
	add_vdp_reg_bits(send_enum, 0x10, "SET_PLANEA_PLANEB_HEIGHT_TO_64_TILES", "SET_PLANEA_PLANEB_HEIGHT_TO_64__TILES", 4, 1 /*01*/);
	add_vdp_reg_bits(send_enum, 0x10, "SET_PLANEA_PLANEB_HEIGHT_TO_128_TILES", "SET_PLANEA_PLANEB_HEIGHT_TO_128__TILES", 4, 3 /*11*/);
}

static void add_vdp_reg_11()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg11", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x11, "REG_11", "REG__11");

	add_vdp_reg_bits(send_enum, 0x11, "MOVE_WINDOW_HORZ_RIGHT", "MOVE_WINDOW_HORZ__RIGHT", 7, SET);
	add_vdp_reg_bits(send_enum, 0x11, "MOVE_WINDOW_HORZ_LEFT", "MOVE_WINDOW_HORZ__LEFT", 7, CLEAR);

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 5); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "MOVE_BY_%d_CELLS", i);
		qsnprintf(name_hi, sizeof(name_hi), "MOVE_BY__%d_CELLS", i);
		add_vdp_reg_bits(send_enum, 0x11, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_12()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg12", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x12, "REG_12", "REG__12");

	add_vdp_reg_bits(send_enum, 0x12, "MOVE_WINDOW_VERT_RIGHT", "MOVE_WINDOW_VERT__RIGHT", 7, SET);
	add_vdp_reg_bits(send_enum, 0x12, "MOVE_WINDOW_VERT_LEFT", "MOVE_WINDOW_VERT__LEFT", 7, CLEAR);

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 5); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "MOVE_BY_%d__CELLS", i);
		qsnprintf(name_hi, sizeof(name_hi), "MOVE_BY__%d__CELLS", i);
		add_vdp_reg_bits(send_enum, 0x12, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_13()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg13", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x13, "REG_13", "REG__13");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 8); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_LOWER_BYTE_OF_DMA_LEN_TO_$%.2X", i);
		qsnprintf(name_hi, sizeof(name_hi), "SET_LOWER_BYTE_OF_DMA_LEN_TO__$%.2X", i);
		add_vdp_reg_bits(send_enum, 0x13, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_14()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg14", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x14, "REG_14", "REG__14");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 8); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_HIGHER_BYTE_OF_DMA_LEN_TO_$%.2X", i);
		qsnprintf(name_hi, sizeof(name_hi), "SET_HIGHER_BYTE_OF_DMA_LEN_TO__$%.2X", i);
		add_vdp_reg_bits(send_enum, 0x14, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_15()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg15", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x15, "REG_15", "REG__15");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 8); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_LOWER_BYTE_OF_DMA_SRC_TO_$%.2X", i);
		qsnprintf(name_hi, sizeof(name_hi), "SET_LOWER_BYTE_OF_DMA_SRC_TO__$%.2X", i);
		add_vdp_reg_bits(send_enum, 0x15, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_16()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg16", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x16, "REG_16", "REG__16");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 8); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_MIDDLE_BYTE_OF_DMA_SRC_TO_$%.2X", i);
		qsnprintf(name_hi, sizeof(name_hi), "SET_MIDDLE_BYTE_OF_DMA_SRC_TO__$%.2X", i);
		add_vdp_reg_bits(send_enum, 0x16, name_lo, name_hi, 0, i);
	}
}

static void add_vdp_reg_17()
{
	enum_t send_enum = add_enum(BADADDR, "enum_vdp_reg17", hexflag());
	set_enum_bf(send_enum, true);
	add_vdp_reg_mask(send_enum, 0x17, "REG_17", "REG__17");

	char name_lo[100], name_hi[100];
	for (int i = 0; i < (1 << 6); i++)
	{
		qsnprintf(name_lo, sizeof(name_lo), "SET_HIGH_BYTE_OF_DMA_SRC_TO_$%.2X", i);
		qsnprintf(name_hi, sizeof(name_hi), "SET_HIGH_BYTE_OF_DMA_SRC_TO__$%.2X", i);
		add_vdp_reg_bits(send_enum, 0x17, name_lo, name_hi, 0, i);
	}

	add_vdp_reg_bits(send_enum, 0x17, "ADD_$800000_TO_DMA_SRC_ADDR", "ADD_$800000_TO_DMA_SRC__ADDR", 7, SET);
	add_vdp_reg_bits(send_enum, 0x17, "SET_COPY_M68K_TO_VRAM_DMA_MODE", "SET_COPY_M68K_TO_VRAM_DMA__MODE", 7, CLEAR);

	add_vdp_reg_bits(send_enum, 0x17, "SET_VRAM_FILL_DMA_MODE", "SET_VRAM_FILL_DMA__MODE", 6, 2 /*10*/);
	add_vdp_reg_bits(send_enum, 0x17, "SET_VRAM_COPY_DMA_MODE", "SET_VRAM_COPY_DMA__MODE", 6, 3 /*11*/);
}

static void add_vdp_reg_send_enums()
{
	add_vdp_reg_0();
	add_vdp_reg_1();
	add_vdp_reg_2();
	add_vdp_reg_3();
	add_vdp_reg_4();
	add_vdp_reg_5();
	add_vdp_reg_6();
	add_vdp_reg_7();
	add_vdp_reg_a();
	add_vdp_reg_b();
	add_vdp_reg_c();
	add_vdp_reg_d();
	add_vdp_reg_e();
	add_vdp_reg_f();
	add_vdp_reg_10();
	add_vdp_reg_11();
	add_vdp_reg_12();
	add_vdp_reg_13();
	add_vdp_reg_14();
	add_vdp_reg_15();
	add_vdp_reg_16();
	add_vdp_reg_17();
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

	add_vdp_reg_send_enums();
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
