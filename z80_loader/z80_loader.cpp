#include <ida.hpp>
#include <loader.hpp>
#include <idp.hpp>
#include <diskio.hpp>
#include <name.hpp>
#include <bytes.hpp>
#include <auto.hpp>

#include "z80_loader.h"

struct reg {
	asize_t size;
	ea_t addr;
	char *name;
};

static const reg SPEC_REGS[] = {
	{ 1, 0x4000, "YM2612_A0" },
	{ 1, 0x4001, "YM2612_D0" },
	{ 1, 0x4002, "YM2612_A1" },
	{ 1, 0x4003, "YM2612_D1" },

	{ 1, 0x6000, "Z80_BANK" },

	{ 1, 0x7F11, "SN76489_PSG" },
};

static void print_version()
{
	static const char format[] = "Sega Genesis/Megadrive Z80 drivers loader v%s;\nAuthor: Dr. MefistO [Lab 313] <meffi@lab313.ru>.";
	info(format, VERSION);
	msg(format, VERSION);
}

static void add_segment(ea_t start, ea_t end, const char *name, const char *class_name, const char *cmnt, uchar perm)
{
	segment_t s;
	s.sel = 0;
	s.start_ea = start;
	s.end_ea = end;
	s.align = saRelByte;
	s.comb = scPub;
	s.bitness = 1; // 32-bit
	s.perm = perm;

	int flags = ADDSEG_NOSREG | ADDSEG_NOTRUNC | ADDSEG_QUIET;

	if (!add_segm_ex(&s, name, class_name, flags)) loader_failure();
	segment_t *segm = getseg(start);
	set_segment_cmt(segm, cmnt, false);
	create_byte(start, 1);
	segm->update();
}

static void make_segments()
{
	add_segment(0x0000, 0x001FFF + 1, "ROM", "CODE", "Main segment", SEGPERM_EXEC | SEGPERM_READ);
	add_segment(0x2000, 0x003FFF + 1, "RESV1", "DATA", "Reserved", SEGPERM_READ | SEGPERM_WRITE);

	add_segment(0x4000, 0x004003 + 1, "YM2612_REGS", "XTRN", "YM2612 Regs", SEGPERM_WRITE);

	add_segment(0x4004, 0x005FFF + 1, "RESV2", "DATA", "Reserved", SEGPERM_READ | SEGPERM_WRITE);

	add_segment(0x6000, 0x006000 + 1, "BANK_REG", "XTRN", "Z80 Bank Reg", SEGPERM_WRITE);

	add_segment(0x6001, 0x007F10 + 1, "RESV3", "DATA", "Reserved", SEGPERM_READ | SEGPERM_WRITE);

	add_segment(0x7F11, 0x007F11 + 1, "PSG_REG", "XTRN", "SN76489 PSG", SEGPERM_WRITE);

	add_segment(0x7F12, 0x007FFF + 1, "RESV4", "DATA", "Reserved", SEGPERM_READ | SEGPERM_WRITE);
	add_segment(0x8000, 0x00FFFF + 1, "BANK", "DATA", "Reserved", SEGPERM_READ | SEGPERM_WRITE);
}

static void set_spec_register_names()
{
	for (int i = 0; i < _countof(SPEC_REGS); i++)
	{
		if (SPEC_REGS[i].size == 2)
		{
			create_word(SPEC_REGS[i].addr, 2);
		}
		else if (SPEC_REGS[i].size == 4)
		{
			create_dword(SPEC_REGS[i].addr, 4);
		}
		else
		{
			create_byte(SPEC_REGS[i].addr, SPEC_REGS[i].size);
		}
		set_name(SPEC_REGS[i].addr, SPEC_REGS[i].name);
	}
}

static void add_sub(unsigned int addr, const char *name)
{
	ea_t e_addr = addr;

	auto_make_proc(e_addr);
	set_name(e_addr, name);
}

static int idaapi accept_file(qstring *fileformatname, qstring *processor, linput_t *li, const char *filename)
{
	qlseek(li, 0, SEEK_SET);
	if (qlsize(li) >= 0x2000) return 0;

	fileformatname->sprnt("%s", "Z80 drivers loader");

	return 1;
}

static void idaapi load_file(linput_t *li, ushort neflags, const char *fileformatname)
{
	if (ph.id != PLFM_Z80) {
		set_processor_type("z80", SETPROC_LOADER); // Z80
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

		| AF_JUMPTBL  //    0x0001          // Locate and create jump tables
					   //| AF2_DODATA  //     0x0002          // Coagulate data segs at the final pass
					   //| AF2_HFLIRT  //     0x0004          // Automatically hide library functions
		| AF_STKARG  //     0x0008          // Propagate stack argument information
		| AF_REGARG  //     0x0010          // Propagate register argument information
					  //| AF2_CHKUNI  //     0x0020          // Check for unicode strings
					  //| AF2_SIGCMT  //     0x0040          // Append a signature name comment for recognized anonymous library functions
		| AF_SIGMLT  //     0x0080          // Allow recognition of several copies of the same function
		| AF_FTAIL  //      0x0100          // Create function tails
		| AF_DATOFF  //     0x0200          // Automatically convert data to offsets
					  //| AF2_ANORET  //     0x0400          // Perform 'no-return' analysis
					  //| AF2_VERSP  //      0x0800          // Perform full SP-analysis (ph.verify_sp)
					  //| AF2_DOCODE  //     0x1000          // Coagulate code segs at the final pass
		| AF_TRFUNC  //     0x2000          // Truncate functions upon code deletion
		| AF_PURDAT  //     0x4000          // Control flow to data segment is ignored
					  //| AF2_MEMFUNC //    0x8000          // Try to guess member function types
		;

	unsigned int size = qlsize(li); // size of driver

	qlseek(li, 0, SEEK_SET);
	if (size > 0x2000) loader_failure();
	file2base(li, 0, 0x0000, size, FILEREG_NOTPATCHABLE); // load driver to database

	make_segments(); // create segments
	set_spec_register_names(); // apply names for special addresses of registers

	del_items(0x0000, DELIT_SIMPLE);
	add_sub(0x0000, "start");

	inf.start_ip = inf.start_ea = inf.main = 0;

	print_version();
}

loader_t LDSC = {
	IDP_INTERFACE_VERSION,
	0,
	accept_file,
	load_file,
	NULL,
	NULL,
	NULL,
};
