/*
*      Interactive disassembler (IDA).
*      Copyright (c) 1990-2000 by Ilfak Guilfanov, <ig@datarescue.com>
*      ALL RIGHTS RESERVED.
*
*/

#define VERSION "1.13"
/*
*      SEGA MEGA DRIVE/GENESIS Z80 Drivers Loader
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
#include <fixup.hpp>

#include "z80_loader.h"

struct reg {
	asize_t size;
	ea_t addr;
	char *name;
};

static const reg spec_regs[] = {
	{ 1, 0x4000, "YM2612_A0" },
    { 1, 0x4001, "YM2612_D0" },
    { 1, 0x4002, "YM2612_A1" },
    { 1, 0x4003, "YM2612_D1" },

    { 1, 0x6000, "Z80_BANK" },

    { 1, 0x7F11, "SN76489_PSG" },
};

static const char Z80[] = "Z80";
static const char ROM[] = "ROM";
static const char YM2612_REGS[] = "YM2612_REGS";
static const char BANK_REG[] = "BANK_REG";
static const char PSG_REG[] = "PSG_REG";
static const char RESV1[] = "RESV1";
static const char RESV2[] = "RESV2";
static const char RESV3[] = "RESV3";
static const char RESV4[] = "RESV4";
static const char BANK[] = "BANK";

static const char CODE[] = "CODE";
static const char DATA[] = "DATA";
static const char XTRN[] = "XTRN";

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
static void add_sub(unsigned int addr, const char *name)
{
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
static void add_dword_array(struc_t *st, const char *name, asize_t length)
{
	add_struc_member(st, name, BADADDR, dwrdflag() | hexflag(), NULL, 4 * length);
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
static void set_spec_register_names()
{
	for (int i = 0; i < _countof(spec_regs); i++)
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
static void make_segments()
{
    add_segment(0x0000, 0x001FFF + 1, ROM, CODE, "Main segment", SEGPERM_EXEC | SEGPERM_READ);
    add_segment(0x2000, 0x003FFF + 1, RESV1, DATA, "Reserved", SEGPERM_READ | SEGPERM_WRITE);

    add_segment(0x4000, 0x004003 + 1, YM2612_REGS, XTRN, "YM2612 Regs", SEGPERM_WRITE);

    add_segment(0x4004, 0x005FFF + 1, RESV2, DATA, "Reserved", SEGPERM_READ | SEGPERM_WRITE);

    add_segment(0x6000, 0x006000 + 1, BANK_REG, XTRN, "Z80 Bank Reg", SEGPERM_WRITE);

    add_segment(0x6001, 0x007F10 + 1, RESV3, DATA, "Reserved", SEGPERM_READ | SEGPERM_WRITE);

    add_segment(0x7F11, 0x007F11 + 1, PSG_REG, XTRN, "SN76489 PSG", SEGPERM_WRITE);

    add_segment(0x7F12, 0x007FFF + 1, RESV4, DATA, "Reserved", SEGPERM_READ | SEGPERM_WRITE);
    add_segment(0x8000, 0x00FFFF + 1, BANK, DATA, "Reserved", SEGPERM_READ | SEGPERM_WRITE);
}

//--------------------------------------------------------------------------
static void print_version()
{
	static const char format[] = "Sega Genesis/Megadrive Z80 drivers loader v%s;\nAuthor: Dr. MefistO [Lab 313] <meffi@lab313.ru>.";
	info(format, VERSION);
	msg(format, VERSION);
}

//--------------------------------------------------------------------------
int idaapi accept_file(linput_t *li, char fileformatname[MAX_FILE_FORMAT_NAME], int n)
{
	if (n != 0) return 0;
	qlseek(li, 0, SEEK_SET);

    if (qlsize(li) >= 0x2000) return 0;

	qstrncpy(fileformatname, "Z80 drivers loader v1", MAX_FILE_FORMAT_NAME);

	return 1;
}

//--------------------------------------------------------------------------
void idaapi load_file(linput_t *li, ushort neflags, const char *fileformatname)
{
	if (ph.id != PLFM_Z80) {
		set_processor_type(Z80, SETPROC_ALL | SETPROC_FATAL); // Z80
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
		| AF2_FTAIL  //      0x0100          // Create function tails
		| AF2_DATOFF  //     0x0200          // Automatically convert data to offsets
		//| AF2_ANORET  //     0x0400          // Perform 'no-return' analysis
		//| AF2_VERSP  //      0x0800          // Perform full SP-analysis (ph.verify_sp)
		//| AF2_DOCODE  //     0x1000          // Coagulate code segs at the final pass
		| AF2_TRFUNC  //     0x2000          // Truncate functions upon code deletion
		| AF2_PURDAT  //     0x4000          // Control flow to data segment is ignored
		//| AF2_MEMFUNC //    0x8000          // Try to guess member function types
		;

	unsigned int size = qlsize(li); // size of driver

	qlseek(li, 0, SEEK_SET);
	if (size > 0x2000) loader_failure();
	file2base(li, 0, 0x0000, size, FILEREG_NOTPATCHABLE); // load driver to database

	make_segments(); // create segments
	set_spec_register_names(); // apply names for special addresses of registers

	do_unknown(0x0000, DOUNK_SIMPLE);
    add_sub(0x0000, "start");

	inf.beginEA = 0;

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
