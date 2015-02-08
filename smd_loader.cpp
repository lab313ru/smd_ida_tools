/*
*      Interactive disassembler (IDA).
*      Copyright (c) 1990-2000 by Ilfak Guilfanov, <ig@datarescue.com>
*      ALL RIGHTS RESERVED.
*
*/

//
//      SEGA MEGA DRIVE/GENESIS ROMs Loader (Modified/Updated HardwareMan's source)
//      Author: Dr. MefistO [Lab 313] <meffi@lab313.ru>, v1.0, 07/02/2015
//

#define NO_OBSOLETE_FUNCS
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

#include "smd_loader.h"

static gen_hdr _hdr;
static gen_vect _vect;

static const char RESET[] = "RESET";
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
static const char M68K_RAM_[] = "M68K_RAM_";
static const char SRAM[] = "SRAM";
static const char VECTORS[] = "vectors";
static const char VECTORS_STRUCT[] = "struct_vectors";
static const char HEADER[] = "header";
static const char HEADER_STRUCT[] = "struct_header";
static const char SETUP[] = "setup";

// Swap the bytes in a 32 bit word in order to convert LE encoded data to BE
// and vice versa.
static unsigned int SWAP_BYTES_32(unsigned int a)
{
	return ((a >> 24) & 0x000000FF) | ((a >> 8) & 0x0000FF00) |	((a << 8) & 0x00FF0000) | ((a << 24) & 0xFF000000);
}

// Read BE word
static unsigned short READ_BE_WORD(unsigned char *addr)
{
	return (addr[0] << 8) | addr[1];
}

// Read BE unsigned int by pointer
static unsigned int READ_BE_UINT(unsigned char *addr)
{
	return (READ_BE_WORD(&addr[0]) << 16) | READ_BE_WORD(&addr[2]);
}

//------------------------------------------------------------------------
static void add_sub(unsigned int addr, const char *name, unsigned int min, unsigned int max)
{
	if ((addr >= min) && (addr < max))
	{
		ea_t e_addr = toEA(ask_selector(0), addr);
		auto_make_proc(e_addr);
		set_name(e_addr, name);
	}
}

//------------------------------------------------------------------------
static void add_offset_field(struc_t *st, segment_t *code_segm, const char *name)
{
	opinfo_t info = { 0 };
	info.ri.init(REF_OFF32, BADADDR);
	add_struc_member(st, name, BADADDR, dwrdflag() | offflag(), &info, 4);
}

static void add_dword_field(struc_t *st, const char *name)
{
	add_struc_member(st, name, BADADDR, dwrdflag() | hexflag(), NULL, 4);
}

static void add_string_field(struc_t *st, const char *name, asize_t size)
{
	opinfo_t info = { 0 };
	info.strtype = ASCSTR_C;
	add_struc_member(st, name, BADADDR, asciflag(), &info, size);
}

static void add_short_field(struc_t *st, const char *name)
{
	add_struc_member(st, name, BADADDR, wordflag() | hexflag(), NULL, 2);
}

static void add_dword_array(struc_t *st, const char *name, asize_t length)
{
	add_struc_member(st, name, BADADDR, dwrdflag() | hexflag(), NULL, 4 * length);
}

static void define_vectors_struct()
{
	segment_t *code_segm = getseg(0);
	tid_t vec_id = add_struc(BADADDR, VECTORS_STRUCT);
	struc_t *vectors = get_struc(vec_id);
	add_offset_field(vectors, code_segm, "SSP");
	add_offset_field(vectors, code_segm, "Reset");
	add_offset_field(vectors, code_segm, "BusErr");
	add_offset_field(vectors, code_segm, "AdrErr");
	add_offset_field(vectors, code_segm, "InvOpCode");
	add_offset_field(vectors, code_segm, "DivBy0");
	add_offset_field(vectors, code_segm, "Check");
	add_offset_field(vectors, code_segm, "TrapV");
	add_offset_field(vectors, code_segm, "GPF");
	add_offset_field(vectors, code_segm, "Trace");
	add_offset_field(vectors, code_segm, "Reserv0");
	add_offset_field(vectors, code_segm, "Reserv1");
	add_offset_field(vectors, code_segm, "Reserv2");
	add_offset_field(vectors, code_segm, "Reserv3");
	add_offset_field(vectors, code_segm, "Reserv4");
	add_offset_field(vectors, code_segm, "BadInt");
	add_offset_field(vectors, code_segm, "Reserv10");
	add_offset_field(vectors, code_segm, "Reserv11");
	add_offset_field(vectors, code_segm, "Reserv12");
	add_offset_field(vectors, code_segm, "Reserv13");
	add_offset_field(vectors, code_segm, "Reserv14");
	add_offset_field(vectors, code_segm, "Reserv15");
	add_offset_field(vectors, code_segm, "Reserv16");
	add_offset_field(vectors, code_segm, "Reserv17");
	add_offset_field(vectors, code_segm, "BadIRQ");
	add_offset_field(vectors, code_segm, "IRQ1");
	add_offset_field(vectors, code_segm, "EXT");
	add_offset_field(vectors, code_segm, "IRQ3");
	add_offset_field(vectors, code_segm, "HBLANK");
	add_offset_field(vectors, code_segm, "IRQ5");
	add_offset_field(vectors, code_segm, "VBLANK");
	add_offset_field(vectors, code_segm, "IRQ7");
	add_offset_field(vectors, code_segm, "Trap0");
	add_offset_field(vectors, code_segm, "Trap1");
	add_offset_field(vectors, code_segm, "Trap2");
	add_offset_field(vectors, code_segm, "Trap3");
	add_offset_field(vectors, code_segm, "Trap4");
	add_offset_field(vectors, code_segm, "Trap5");
	add_offset_field(vectors, code_segm, "Trap6");
	add_offset_field(vectors, code_segm, "Trap7");
	add_offset_field(vectors, code_segm, "Trap8");
	add_offset_field(vectors, code_segm, "Trap9");
	add_offset_field(vectors, code_segm, "Trap10");
	add_offset_field(vectors, code_segm, "Trap11");
	add_offset_field(vectors, code_segm, "Trap12");
	add_offset_field(vectors, code_segm, "Trap13");
	add_offset_field(vectors, code_segm, "Trap14");
	add_offset_field(vectors, code_segm, "Trap15");
	add_offset_field(vectors, code_segm, "Reserv30");
	add_offset_field(vectors, code_segm, "Reserv31");
	add_offset_field(vectors, code_segm, "Reserv32");
	add_offset_field(vectors, code_segm, "Reserv33");
	add_offset_field(vectors, code_segm, "Reserv34");
	add_offset_field(vectors, code_segm, "Reserv35");
	add_offset_field(vectors, code_segm, "Reserv36");
	add_offset_field(vectors, code_segm, "Reserv37");
	add_offset_field(vectors, code_segm, "Reserv38");
	add_offset_field(vectors, code_segm, "Reserv39");
	add_offset_field(vectors, code_segm, "Reserv3A");
	add_offset_field(vectors, code_segm, "Reserv3B");
	add_offset_field(vectors, code_segm, "Reserv3C");
	add_offset_field(vectors, code_segm, "Reserv3D");
	add_offset_field(vectors, code_segm, "Reserv3E");
	add_offset_field(vectors, code_segm, "Reserv3F");
	doStruct(0, sizeof(gen_vect), vec_id);
	set_name(0, VECTORS);
}

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

void add_subroutines(unsigned int size)
{
	add_entry(1, toEA(ask_selector(0), _vect.Reset), RESET, true); // entry point
	add_sub(_vect.BusErr, "BUSERR", 0x200, size);
	add_sub(_vect.AdrErr, "ADRERR", 0x200, size);
	add_sub(_vect.InvOpCode, "INVOPCODE", 0x200, size);
	add_sub(_vect.DivBy0, "DIVBY0", 0x200, size);
	add_sub(_vect.Check, "CHECK", 0x200, size);
	add_sub(_vect.TrapV, "TRAPV", 0x200, size);
	add_sub(_vect.GPF, "GPF", 0x200, size);
	add_sub(_vect.Trace, "TRACE", 0x200, size);
	add_sub(_vect.Reserv0, "RESERV0", 0x200, size);
	add_sub(_vect.Reserv1, "RESERV1", 0x200, size);
	add_sub(_vect.Reserv2, "RESERV2", 0x200, size);
	add_sub(_vect.Reserv3, "RESERV3", 0x200, size);
	add_sub(_vect.Reserv4, "RESERV4", 0x200, size);
	add_sub(_vect.BadInt, "BADINT", 0x200, size);
	add_sub(_vect.Reserv10, "RESERV10", 0x200, size);
	add_sub(_vect.Reserv11, "RESERV11", 0x200, size);
	add_sub(_vect.Reserv12, "RESERV12", 0x200, size);
	add_sub(_vect.Reserv13, "RESERV13", 0x200, size);
	add_sub(_vect.Reserv14, "RESERV14", 0x200, size);
	add_sub(_vect.Reserv15, "RESERV15", 0x200, size);
	add_sub(_vect.Reserv16, "RESERV16", 0x200, size);
	add_sub(_vect.Reserv17, "RESERV17", 0x200, size);
	add_sub(_vect.BadIRQ, "BADIRQ", 0x200, size);
	add_sub(_vect.IRQ1, "IRQ1", 0x200, size);
	add_sub(_vect.EXT, "EXTIRQ", 0x200, size);
	add_sub(_vect.IRQ3, "IRQ3", 0x200, size);
	add_sub(_vect.HBLANK, "HBLANK", 0x200, size);
	add_sub(_vect.IRQ5, "IRQ5", 0x200, size);
	add_sub(_vect.VBLANK, "VBLANK", 0x200, size);
	add_sub(_vect.IRQ7, "IRQ7", 0x200, size);
	add_sub(_vect.Trap0, "TRAP0", 0x200, size);
	add_sub(_vect.Trap1, "TRAP1", 0x200, size);
	add_sub(_vect.Trap2, "TRAP2", 0x200, size);
	add_sub(_vect.Trap3, "TRAP3", 0x200, size);
	add_sub(_vect.Trap4, "TRAP4", 0x200, size);
	add_sub(_vect.Trap5, "TRAP5", 0x200, size);
	add_sub(_vect.Trap6, "TRAP6", 0x200, size);
	add_sub(_vect.Trap7, "TRAP7", 0x200, size);
	add_sub(_vect.Trap8, "TRAP8", 0x200, size);
	add_sub(_vect.Trap9, "TRAP9", 0x200, size);
	add_sub(_vect.Trap10, "TRAP10", 0x200, size);
	add_sub(_vect.Trap11, "TRAP11", 0x200, size);
	add_sub(_vect.Trap12, "TRAP12", 0x200, size);
	add_sub(_vect.Trap13, "TRAP13", 0x200, size);
	add_sub(_vect.Trap14, "TRAP14", 0x200, size);
	add_sub(_vect.Trap15, "TRAP15", 0x200, size);
	add_sub(_vect.Reserv30, "RESERV30", 0x200, size);
	add_sub(_vect.Reserv31, "RESERV31", 0x200, size);
	add_sub(_vect.Reserv32, "RESERV32", 0x200, size);
	add_sub(_vect.Reserv33, "RESERV33", 0x200, size);
	add_sub(_vect.Reserv34, "RESERV34", 0x200, size);
	add_sub(_vect.Reserv35, "RESERV35", 0x200, size);
	add_sub(_vect.Reserv36, "RESERV36", 0x200, size);
	add_sub(_vect.Reserv37, "RESERV37", 0x200, size);
	add_sub(_vect.Reserv38, "RESERV38", 0x200, size);
	add_sub(_vect.Reserv39, "RESERV39", 0x200, size);
	add_sub(_vect.Reserv3A, "RESERV3A", 0x200, size);
	add_sub(_vect.Reserv3B, "RESERV3B", 0x200, size);
	add_sub(_vect.Reserv3C, "RESERV3C", 0x200, size);
	add_sub(_vect.Reserv3D, "RESERV3D", 0x200, size);
	add_sub(_vect.Reserv3E, "RESERV3E", 0x200, size);
	add_sub(_vect.Reserv3F, "RESERV3F", 0x200, size);
}

//------------------------------------------------------------------------
static void set_def_names()
{
	doDwrd(0xA04000, 4); set_name(0xA04000, "Z80_YM2612");
	doWord(0xA07F10, 2); set_name(0xA07F10, "Z80_PSG");
	doWord(0xA10000, 2); set_name(0xA10000, "IO_PCBVER");
	doWord(0xA10002, 2); set_name(0xA10002, "IO_CT1_DATA");
	doWord(0xA10004, 2); set_name(0xA10004, "IO_CT2_DATA");
	doWord(0xA10006, 2); set_name(0xA10006, "IO_EXT_DATA");
	doWord(0xA10008, 2); set_name(0xA10008, "IO_CT1_CTRL");
	doWord(0xA1000A, 2); set_name(0xA1000A, "IO_CT2_CTRL");
	doWord(0xA1000C, 2); set_name(0xA1000C, "IO_EXT_CTRL");
	doWord(0xA1000E, 2); set_name(0xA1000E, "IO_CT1_RX");
	doWord(0xA10010, 2); set_name(0xA10010, "IO_CT1_TX");
	doWord(0xA10012, 2); set_name(0xA10012, "IO_CT1_SMODE");
	doWord(0xA10014, 2); set_name(0xA10014, "IO_CT2_RX");
	doWord(0xA10016, 2); set_name(0xA10016, "IO_CT2_TX");
	doWord(0xA10018, 2); set_name(0xA10018, "IO_CT2_SMODE");
	doWord(0xA1001A, 2); set_name(0xA1001A, "IO_EXT_RX");
	doWord(0xA1001C, 2); set_name(0xA1001C, "IO_EXT_TX");
	doWord(0xA1001E, 2); set_name(0xA1001E, "IO_EXT_SMODE");
	doWord(0xA11000, 2); set_name(0xA11000, "IO_RAMMODE");
	doWord(0xA11100, 2); set_name(0xA11100, "IO_Z80BUS");
	doWord(0xA11200, 2); set_name(0xA11200, "IO_Z80RES");
	doByte(0xA12000, 0x100); set_name(0xA12000, "IO_FDC");
	doByte(0xA13000, 0x100); set_name(0xA13000, "IO_TIME");
	doWord(0xC00000, 2); set_name(0xC00000, "VDP_DATA");
	doWord(0xC00002, 2); set_name(0xC00002, "VDP_DATA_");
	doWord(0xC00004, 2); set_name(0xC00004, "VDP_CTRL");
	doWord(0xC00006, 2); set_name(0xC00006, "VDP_CTRL_");
	doWord(0xC00008, 2); set_name(0xC00008, "VDP_CNTR");
	doWord(0xC0000A, 2); set_name(0xC0000A, "VDP_CNTR_");
	doWord(0xC00010, 2); set_name(0xC00010, "VDP_PSG");
}

static void add_segment(ea_t start, ea_t end, const char *name, const char *class_name, const char *cmnt)
{
	if (!add_segm(0, start, end, name, class_name)) loader_failure();
	segment_t *segm = getseg(start);
	set_segment_cmt(segm, cmnt, false);
	doByte(start, 1);
}

static void make_segments()
{
	add_segment(0x00000000, 0x003FFFFF + 1, ROM, CODE, "ROM segment");
	add_segment(0x00400000, 0x007FFFFF + 1, EPA, DATA, "Expansion Port Area (used by the Sega CD)");
	add_segment(0x00800000, 0x009FFFFF + 1, S32X, DATA, "Unallocated (used by the Sega 32X)");
	add_segment(0x00A00000, 0x00A0FFFF + 1, Z80, DATA, "Z80 Memory"); set_name(0x00A00000, Z80_RAM);
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

//------------------------------------------------------------------------
void convert_vector_addrs()
{
	_vect.SSP = SWAP_BYTES_32(_vect.SSP);
	_vect.Reset = SWAP_BYTES_32(_vect.Reset);
	_vect.BusErr = SWAP_BYTES_32(_vect.BusErr);
	_vect.AdrErr = SWAP_BYTES_32(_vect.AdrErr);
	_vect.InvOpCode = SWAP_BYTES_32(_vect.InvOpCode);
	_vect.DivBy0 = SWAP_BYTES_32(_vect.DivBy0);
	_vect.Check = SWAP_BYTES_32(_vect.Check);
	_vect.TrapV = SWAP_BYTES_32(_vect.TrapV);
	_vect.GPF = SWAP_BYTES_32(_vect.GPF);
	_vect.Trace = SWAP_BYTES_32(_vect.Trace);
	_vect.Reserv0 = SWAP_BYTES_32(_vect.Reserv0);
	_vect.Reserv1 = SWAP_BYTES_32(_vect.Reserv1);
	_vect.Reserv2 = SWAP_BYTES_32(_vect.Reserv2);
	_vect.Reserv3 = SWAP_BYTES_32(_vect.Reserv3);
	_vect.Reserv4 = SWAP_BYTES_32(_vect.Reserv4);
	_vect.BadInt = SWAP_BYTES_32(_vect.BadInt);
	_vect.Reserv10 = SWAP_BYTES_32(_vect.Reserv10);
	_vect.Reserv11 = SWAP_BYTES_32(_vect.Reserv11);
	_vect.Reserv12 = SWAP_BYTES_32(_vect.Reserv12);
	_vect.Reserv13 = SWAP_BYTES_32(_vect.Reserv13);
	_vect.Reserv14 = SWAP_BYTES_32(_vect.Reserv14);
	_vect.Reserv15 = SWAP_BYTES_32(_vect.Reserv15);
	_vect.Reserv16 = SWAP_BYTES_32(_vect.Reserv16);
	_vect.Reserv17 = SWAP_BYTES_32(_vect.Reserv17);
	_vect.BadIRQ = SWAP_BYTES_32(_vect.BadIRQ);
	_vect.IRQ1 = SWAP_BYTES_32(_vect.IRQ1);
	_vect.EXT = SWAP_BYTES_32(_vect.EXT);
	_vect.IRQ3 = SWAP_BYTES_32(_vect.IRQ3);
	_vect.HBLANK = SWAP_BYTES_32(_vect.HBLANK);
	_vect.IRQ5 = SWAP_BYTES_32(_vect.IRQ5);
	_vect.VBLANK = SWAP_BYTES_32(_vect.VBLANK);
	_vect.IRQ7 = SWAP_BYTES_32(_vect.IRQ7);
	_vect.Trap0 = SWAP_BYTES_32(_vect.Trap0);
	_vect.Trap1 = SWAP_BYTES_32(_vect.Trap1);
	_vect.Trap2 = SWAP_BYTES_32(_vect.Trap2);
	_vect.Trap3 = SWAP_BYTES_32(_vect.Trap3);
	_vect.Trap4 = SWAP_BYTES_32(_vect.Trap4);
	_vect.Trap5 = SWAP_BYTES_32(_vect.Trap5);
	_vect.Trap6 = SWAP_BYTES_32(_vect.Trap6);
	_vect.Trap7 = SWAP_BYTES_32(_vect.Trap7);
	_vect.Trap8 = SWAP_BYTES_32(_vect.Trap8);
	_vect.Trap9 = SWAP_BYTES_32(_vect.Trap9);
	_vect.Trap10 = SWAP_BYTES_32(_vect.Trap10);
	_vect.Trap11 = SWAP_BYTES_32(_vect.Trap11);
	_vect.Trap12 = SWAP_BYTES_32(_vect.Trap12);
	_vect.Trap13 = SWAP_BYTES_32(_vect.Trap13);
	_vect.Trap14 = SWAP_BYTES_32(_vect.Trap14);
	_vect.Trap15 = SWAP_BYTES_32(_vect.Trap15);
	_vect.Reserv30 = SWAP_BYTES_32(_vect.Reserv30);
	_vect.Reserv31 = SWAP_BYTES_32(_vect.Reserv31);
	_vect.Reserv32 = SWAP_BYTES_32(_vect.Reserv32);
	_vect.Reserv33 = SWAP_BYTES_32(_vect.Reserv33);
	_vect.Reserv34 = SWAP_BYTES_32(_vect.Reserv34);
	_vect.Reserv35 = SWAP_BYTES_32(_vect.Reserv35);
	_vect.Reserv36 = SWAP_BYTES_32(_vect.Reserv36);
	_vect.Reserv37 = SWAP_BYTES_32(_vect.Reserv37);
	_vect.Reserv38 = SWAP_BYTES_32(_vect.Reserv38);
	_vect.Reserv39 = SWAP_BYTES_32(_vect.Reserv39);
	_vect.Reserv3A = SWAP_BYTES_32(_vect.Reserv3A);
	_vect.Reserv3B = SWAP_BYTES_32(_vect.Reserv3B);
	_vect.Reserv3C = SWAP_BYTES_32(_vect.Reserv3C);
	_vect.Reserv3D = SWAP_BYTES_32(_vect.Reserv3D);
	_vect.Reserv3E = SWAP_BYTES_32(_vect.Reserv3E);
	_vect.Reserv3F = SWAP_BYTES_32(_vect.Reserv3F);
}

//--------------------------------------------------------------------------
int idaapi accept_file(linput_t *li, char fileformatname[MAX_FILE_FORMAT_NAME],	int n)
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

	make_segments();

	convert_vector_addrs(); // LittleEndian to BigEndian

	define_vectors_struct();
	define_header_struct();
	set_def_names();

	add_subroutines(size);

	ea_t tt = get_name_ea(BADADDR, RESET);
	inf.startIP = tt;
	inf.beginEA = tt;

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