/*
*      Interactive disassembler (IDA).
*      Copyright (c) 1990-2000 by Ilfak Guilfanov, <ig@datarescue.com>
*      ALL RIGHTS RESERVED.
*
*/

#define VERSION "1.21"
/*
*      SEGA MEGA DRIVE/GENESIS ROMs Loader (Modified/Updated HardwareMan's source)
*      Author: Dr. MefistO [Lab 313] <meffi@lab313.ru>
*/

#define _CRT_SECURE_NO_WARNINGS

#include <ida.hpp>
#include <idp.hpp>
#include <diskio.hpp>
#include <loader.hpp>
#include <auto.hpp>
#include <name.hpp>
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
static const char XTRN[] = "XTRN";
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
static const char SRAM[] = "SRAM";
static const char VECTORS[] = "vectors";
static const char VECTORS_STRUCT[] = "struct_vectors";
static const char HEADER[] = "header";
static const char HEADER_STRUCT[] = "struct_header";
static const char SPRITE_STRUCT[] = "struct_sprite";

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
static void add_byte_field(struc_t *st, const char *name, const char *cmt = NULL)
{
    add_struc_member(st, name, BADADDR, byteflag() | hexflag(), NULL, 1);
    member_t *mm = get_member_by_name(st, name);
    set_member_cmt(mm, cmt, true);
}

//------------------------------------------------------------------------
static void add_short_field(struc_t *st, const char *name, const char *cmt = NULL)
{
	add_struc_member(st, name, BADADDR, wordflag() | hexflag(), NULL, 2);
    member_t *mm = get_member_by_name(st, name);
    set_member_cmt(mm, cmt, true);
}

//------------------------------------------------------------------------
static void add_segment(ea_t start, ea_t end, const char *name, const char *class_name, const char *cmnt, uchar perm)
{
	segment_t s;
	s.sel = 0;
	s.startEA = start;
	s.endEA = end;
	s.align = saRelByte;
	s.comb = scPub;
	s.bitness = 1; // 32-bit
    s.perm = perm;

    int flags = ADDSEG_NOSREG | ADDSEG_NOTRUNC | ADDSEG_QUIET;

	if (!add_segm_ex(&s, name, class_name, flags)) loader_failure();
	segment_t *segm = getseg(start);
	set_segment_cmt(segm, cmnt, false);
	doByte(start, 1);
	segm->update();
}

//------------------------------------------------------------------------
static void add_vector_subs(gen_vect *table, unsigned int rom_size)
{
    doDwrd(0, 4);
	for (int i = 1; i < _countof(VECTOR_NAMES); i++) // except SPP pointer
	{
		add_sub(table->vectors[i], VECTOR_NAMES[i], rom_size);
        doDwrd(i * 4, 4);
	}
}

//------------------------------------------------------------------------
static void get_vector_addrs(gen_vect *table)
{
	for (int i = 0; i < _countof(table->vectors); i++)
	{
		table->vectors[i] = SWAP_BYTES_32(table->vectors[i]);
	}
}

//--------------------------------------------------------------------------
static void add_enum_member_with_mask(enum_t id, const char *name, unsigned int value, unsigned int mask = DEFMASK, const char *cmt = NULL)
{
	int res = add_enum_member(id, name, value, mask); // we have to use old name, because of IDA v5.2
	if (cmt != NULL) set_enum_member_cmt(get_enum_member_by_name(name), cmt, false);
}

//------------------------------------------------------------------------
static void make_array(ea_t addr, int datatype, const char *name, asize_t size)
{
    const array_parameters_t array_params = {AP_ARRAY, 0, 0};

    switch (datatype)
    {
    case 1: doByte(addr, size); break;
    case 2: doWord(addr, size); break;
    case 4: doWord(addr, size); break;
    }
    set_array_parameters(addr, &array_params);
    set_name(addr, name);
}

//------------------------------------------------------------------------
static void define_header()
{
    make_array(0x100, 1, "CopyRights", 0x20);
    make_array(0x120, 1, "DomesticName", 0x30);
    make_array(0x150, 1, "OverseasName", 0x30);
    make_array(0x180, 1, "ProductCode", 0x0E);
    make_array(0x18E, 2, "Checksum", 0x02);
    make_array(0x190, 1, "Peripherials", 0x10);
    make_array(0x1A0, 4, "RomStart", 0x04);
    make_array(0x1A4, 4, "RomEnd", 0x04);
    make_array(0x1A8, 4, "RamStart", 0x04);
    make_array(0x1AC, 4, "RamEnd", 0x04);
    make_array(0x1B0, 1, "SramCode", 0x0C);
    make_array(0x1BC, 1, "ModemCode", 0x0C);
    make_array(0x1C8, 1, "Reserved", 0x28);
    make_array(0x1F0, 1, "CountryCode", 0x10);
}

//------------------------------------------------------------------------
static void define_sprite_struct()
{
    tid_t sprite_id = add_struc(BADADDR, SPRITE_STRUCT);
    struc_t *sprite = get_struc(sprite_id);

    add_short_field(sprite, "y", "------yy yyyyyyyy (Vertical coordinate of sprite)");
    add_byte_field(sprite, "hsize_vsize", "----hhvv (HSize and VSize in cells)");
    add_byte_field(sprite, "link", "-lllllll (Link field)");
    add_short_field(sprite, "pcvhn", "pccvhnnn nnnnnnnn\n"
    "(p - Priority, cc - Color palette,\n"
    "v - VF, h - HF, n - Start tile index)");
    add_short_field(sprite, "x", "------xx xxxxxxxx (Horizontal coordinate of sprite)");
}

//------------------------------------------------------------------------
static void make_segments(size_t romsize)
{
	add_segment(0x00000000, qmin(romsize, 0x003FFFFF + 1), ROM, CODE, "ROM segment", SEGPERM_EXEC | SEGPERM_READ);

    if (ASKBTN_YES == askyn_c(ASKBTN_NO, "Create Sega CD segment?"))
	    add_segment(0x00400000, 0x007FFFFF + 1, EPA, DATA, "Expansion Port Area (used by the Sega CD)", SEGPERM_READ | SEGPERM_WRITE);
    
    if (ASKBTN_YES == askyn_c(ASKBTN_NO, "Create Sega 32X segment?"))
	    add_segment(0x00800000, 0x009FFFFF + 1, S32X, DATA, "Unallocated (used by the Sega 32X)", SEGPERM_READ | SEGPERM_WRITE);
	
    add_segment(0x00A00000, 0x00A0FFFF + 1, Z80, DATA, "Z80 Memory", SEGPERM_READ | SEGPERM_WRITE);
    make_array(0xA04000, 4, "Z80_YM2612", 4);

    add_segment(0xA10000, 0xA10020, "SYS1", XTRN, "System regs 1", SEGPERM_WRITE);
    make_array(0xA10000, 2, "IO_PCBVER", 2);
    make_array(0xA10002, 2, "IO_CT1_DATA", 2);
    make_array(0xA10004, 2, "IO_CT2_DATA", 2);
    make_array(0xA10006, 2, "IO_EXT_DATA", 2);
    make_array(0xA10008, 2, "IO_CT1_CTRL", 2);
    make_array(0xA1000A, 2, "IO_CT2_CTRL", 2);
    make_array(0xA1000C, 2, "IO_EXT_CTRL", 2);
    make_array(0xA1000E, 2, "IO_CT1_RX", 2);
    make_array(0xA10010, 2, "IO_CT1_TX", 2);
    make_array(0xA10012, 2, "IO_CT1_SMODE", 2);
    make_array(0xA10014, 2, "IO_CT2_RX", 2);
    make_array(0xA10016, 2, "IO_CT2_TX", 2);
    make_array(0xA10018, 2, "IO_CT2_SMODE", 2);
    make_array(0xA1001A, 2, "IO_EXT_RX", 2);
    make_array(0xA1001C, 2, "IO_EXT_TX", 2);
    make_array(0xA1001E, 2, "IO_EXT_SMODE", 2);

    add_segment(0xA11000, 0xA11002, "Z801", XTRN, "System regs 1", SEGPERM_WRITE);
    make_array(0xA11000, 2, "IO_RAMMODE", 2);

    add_segment(0xA11100, 0xA11102, "Z802", XTRN, "Z80 control 2", SEGPERM_WRITE);
    make_array(0xA11100, 2, "IO_Z80BUS", 2);

    add_segment(0xA11200, 0xA11202, "Z803", XTRN, "Z80 control 3", SEGPERM_WRITE);
    make_array(0xA11200, 2, "IO_Z80RES", 2);

    add_segment(0xA12000, 0xA12100, "FDC", XTRN, "IO FDC", SEGPERM_WRITE);
    make_array(0xA12000, 1, "IO_FDC", 0x100);

    add_segment(0xA13000, 0xA13100, "TIME", XTRN, "IO TIME", SEGPERM_WRITE);
    make_array(0xA13000, 1, "IO_TIME", 0x100);

    add_segment(0xA14000, 0xA14004, "TMSS", XTRN, "IO TMSS", SEGPERM_WRITE);
    make_array(0xA14000, 4, "IO_TMSS", 4);

    add_segment(0xC00000, 0xC00013, "VDP", XTRN, "VDP registers", SEGPERM_WRITE);
    make_array(0xC00000, 2, "VDP_DATA", 2);
    make_array(0xC00002, 2, "VDP__DATA", 2);
    make_array(0xC00004, 2, "VDP_CTRL", 2);
    make_array(0xC00006, 2, "VDP__CTRL", 2);
    make_array(0xC00008, 2, "VDP_CNTR", 2);
    make_array(0xC0000A, 2, "VDP__CNTR", 2);
    make_array(0xC0000C, 2, "VDP___CNTR", 2);
    make_array(0xC0000E, 2, "VDP____CNTR", 2);
    make_array(0xC000011, 2, "VDP_PSG", 2);

    add_segment(0x00FF0000, 0x00FFFFFF + 1, RAM, CODE, "RAM segment", SEGPERM_MAXVAL);

	set_name(0x00A00000, Z80_RAM);
	set_name(0x00FF0000, M68K_RAM);

	// create SRAM segment
	if ((READ_BE_WORD(&(_hdr.SramCode[0])) == 0x5241) && (_hdr.SramCode[2] == 0x20))
	{
		unsigned int sram_s = READ_BE_UINT(&(_hdr.SramCode[4])); // read SRAM start addr
		unsigned int sram_e = READ_BE_UINT(&(_hdr.SramCode[8])); // read SRAM end addr

		if ((sram_s >= 0x400000) && (sram_e <= 0x9FFFFF) && (sram_s < sram_e))
		{
			add_segment(sram_s, sram_e, SRAM, DATA, "SRAM memory", SEGPERM_READ | SEGPERM_WRITE);
		}
	}
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
    if (!strneq((const char *)(_hdr.CopyRights), "SEGA", 4)) return 0;
	qstrncpy(fileformatname, "Sega Genesis/MegaDrive ROM file v.2", MAX_FILE_FORMAT_NAME);

	return 1;
}

//--------------------------------------------------------------------------
void idaapi load_file(linput_t *li, ushort neflags, const char *fileformatname)
{
	if (ph.id != PLFM_68K) {
		set_processor_type(M68K, SETPROC_ALL | SETPROC_FATAL); // Motorola 68000
		set_target_assembler(0);
	}

    inf.af = 0
		//| AF_FIXUP //        0x0001          // Create offsets and segments using fixup info
		//| AF_MARKCODE  //     0x0002          // Mark typical code sequences as code
		| AF_UNK //          0x0004          // Delete instructions with no xrefs
		| AF_CODE //         0x0008          // Trace execution flow
		| AF_PROC //         0x0010          // Create functions if call is present
		| AF_USED //         0x0020          // Analyze and create all xrefs
		//| AF_FLIRT //        0x0040          // Use flirt signatures
		//| AF_PROCPTR //      0x0080          // Create function if data xref data->code32 exists
		| AF_JFUNC //        0x0100          // Rename jump functions as j_...
		| AF_NULLSUB //      0x0200          // Rename empty functions as nullsub_...
		//| AF_LVAR //         0x0400          // Create stack variables
		//| AF_TRACE //        0x0800          // Trace stack pointer
		//| AF_ASCII //        0x1000          // Create ascii string if data xref exists
		| AF_IMMOFF //       0x2000          // Convert 32bit instruction operand to offset
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
		//| AF2_FTAIL  //      0x0100          // Create function tails
		| AF2_DATOFF  //     0x0200          // Automatically convert data to offsets
		//| AF2_ANORET  //     0x0400          // Perform 'no-return' analysis
		//| AF2_VERSP  //      0x0800          // Perform full SP-analysis (ph.verify_sp)
		//| AF2_DOCODE  //     0x1000          // Coagulate code segs at the final pass
		| AF2_TRFUNC  //     0x2000          // Truncate functions upon code deletion
		| AF2_PURDAT  //     0x4000          // Control flow to data segment is ignored
		//| AF2_MEMFUNC //    0x8000          // Try to guess member function types
		;

	unsigned int size = qlsize(li); // size of rom

	qlseek(li, 0, SEEK_SET);
	if (qlread(li, &_vect, sizeof(_vect)) != sizeof(_vect)) loader_failure(); // trying to read rom vectors
	if (qlread(li, &_hdr, sizeof(_hdr)) != sizeof(_hdr)) loader_failure(); // trying to read rom's header

	file2base(li, 0, 0x0000000, size, FILEREG_NOTPATCHABLE); // load rom to database

	make_segments(size); // create ROM, RAM, Z80 RAM and etc. segments
	get_vector_addrs(&_vect); // convert addresses of vectors from LE to BE
	define_header(); // add definition of header struct
    define_sprite_struct(); // add definition of sprite struct
	add_vector_subs(&_vect, size); // mark vector subroutines as procedures

    add_vdp_status_enum();
    add_version_reg_enum();

	inf.beginEA = get_name_ea(BADADDR, VECTOR_NAMES[1]); // Reset

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
