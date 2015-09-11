/*
*      Interactive disassembler (IDA).
*      Copyright (c) 1990-2000 by Ilfak Guilfanov, <ig@datarescue.com>
*      ALL RIGHTS RESERVED.
*
*/

#define VERSION "1.0.7"
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
static void add_other_enum()
{
    enum_t ot = add_enum(BADADDR, "consts_other", hexflag());
	add_enum_member_with_mask(ot, "ROM_START", 0x200);
	add_enum_member_with_mask(ot, "IO_PCBVER_REF", 0x10FF);
	add_enum_member_with_mask(ot, "IO_TMSS_REF", 0x2F00); // IO_TMSS
}

//--------------------------------------------------------------------------
static void add_vdp_status_enum()
{
    enum_t vdp_status = add_enum(BADADDR, "vdp_status", hexflag());
	add_enum_member_with_mask(vdp_status, "FIFO_EMPTY", 9);
	add_enum_member_with_mask(vdp_status, "FIFO_FULL", 8);
	add_enum_member_with_mask(vdp_status, "VBLANK_PENDING", 7);
	add_enum_member_with_mask(vdp_status, "SPRITE_OVERFLOW", 6);
	add_enum_member_with_mask(vdp_status, "SPRITE_COLLISION", 5);
	add_enum_member_with_mask(vdp_status, "ODD_FRAME", 4);
	add_enum_member_with_mask(vdp_status, "VBLANKING", 3);
	add_enum_member_with_mask(vdp_status, "HBLANKING", 2);
	add_enum_member_with_mask(vdp_status, "DMA_IN_PROGRESS", 1);
	add_enum_member_with_mask(vdp_status, "PAL_MODE", 0);
}

static void add_version_reg_enum()
{
    enum_t ver_enum = add_enum(BADADDR, "version_reg", hexflag());
    add_enum_member_with_mask(ver_enum, "USA_EUROPE_JAPAN", 7);
    add_enum_member_with_mask(ver_enum, "PAL_OR_NTSC", 6);
    add_enum_member_with_mask(ver_enum, "SEGA_CD_CONNECTED", 5);
    add_enum_member_with_mask(ver_enum, "HARDWARE_VERSION", 0);
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
	if (ph.id != PLFM_68K) {
		set_processor_type(M68K, SETPROC_ALL | SETPROC_FATAL); // Motorola 68000
	}

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

    add_other_enum();
    add_vdp_status_enum();
    add_version_reg_enum();

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
