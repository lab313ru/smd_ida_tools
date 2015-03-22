//
//	Registers of M68000 (definitions)
//

#include "m68k.hpp"
#include <entry.hpp>
#include <srarea.hpp>
#include <diskio.hpp>
#include <auto.hpp>
#include <name.hpp>

#define SEGNAME_ROM "ROM"
#define SEGNAME_RAM "RAM"
#define SEGNAME_VDP "VDP"
#define SEGNAME_IOP "IOP"
#define SEGNAME_Z80 "Z80"
#define SEGTYPE_CODE "CODE"
#define SEGTYPE_DATA "DATA"
#define SEGTYPE_XTRN "XTRN"
#define NONEPROC "NONE"

netnode helper;

static char *RegNames[] = {
	"d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
	"pc", "ccr", "sr", "usp", "sp", "cs", "ds"
};

static char *shnames[] = { "M68000", NULL };
static char *lnames[] = { "Motorola MC68000 (M68K)", NULL };

static const uchar rtrM68K[] = { 0x4E, 0x77 };
static const uchar rtsM68K[] = { 0x4E, 0x75 };
static const uchar rteM68K[] = { 0x4E, 0x73 };
static const bytes_t retcodes[] =
{
	{ sizeof(rtrM68K), rtrM68K },
	{ sizeof(rtsM68K), rtsM68K },
	{ sizeof(rteM68K), rteM68K },
	{ 0, NULL }
};

static int idaapi notify(processor_t::idp_notify msgid, ...);
static const char *idaapi set_idp_options(const char *keyword, int value_type, const void *value);
static void setup_device();
static void create_mappings();

segment_t* segROM() { return get_segm_by_name(SEGNAME_ROM); }
segment_t* segRAM() { return get_segm_by_name(SEGNAME_RAM); }
segment_t* segVDP() { return get_segm_by_name(SEGNAME_VDP); }
segment_t* segIOP() { return get_segm_by_name(SEGNAME_IOP); }
segment_t* segZ80() { return get_segm_by_name(SEGNAME_Z80); }

static int idaapi notify(processor_t::idp_notify msgid, ...) {
	va_list va;

	va_start(va, msgid);
	int code = invoke_callbacks(HT_IDP, msgid, va);
	if (code) return code;

	switch (msgid)
	{
	case processor_t::init:
		inf.mf = true;
		//batch = 1;
		helper.create("$ mc68k");

		//setup_device();
		//create_mappings();
		break;

	case processor_t::is_sane_insn:
		return is_sane_insn(va_arg(va, int));
	}

	va_end(va);

	return 1;
}

static void setup_device()
{
	segment_t *pSegment;
	ea_t ea;

	noUsed(0, BADADDR);

	pSegment = segROM();
	if (!pSegment)
	{
		ea = 0;
		add_segm(0, ea, ea + 0x3FFFFF + 1, SEGNAME_ROM, SEGTYPE_CODE);

		pSegment = getseg(ea);
		set_segm_end(pSegment->startEA, pSegment->startEA + 0x00400000, SEGMOD_KILL);
	}

	pSegment = segRAM();
	if (!pSegment)
	{
		for (int i = 0; i < 0x20; i++)
		{
			ea = 0x00E00000 + i * 0x10000;
			add_segm(0, ea, ea + 0xFFFF + 1, SEGNAME_RAM, SEGTYPE_DATA);

			pSegment = getseg(ea);
			set_segm_end(ea, ea + 0xFFFF + 1, SEGMOD_KILL);
			set_segment_cmt(pSegment, "RAM mirror", false);
		}

		ea = 0xFFFF0000;
		add_segm(0, ea, ea + 0xFFFE, SEGNAME_RAM, SEGTYPE_DATA);

		pSegment = getseg(ea);
		set_segm_end(ea, ea + 0xFFFE, SEGMOD_KILL);
		set_segment_cmt(pSegment, "RAM mirror", false);
	}

	pSegment = segVDP();
	if (!pSegment)
	{
		for (int i = 0; i < 0x1; i++)
		{
			ea = 0x00C00000 + i * 0x20;
			add_segm(0, ea, ea + 0x1F + 1, SEGNAME_VDP, SEGTYPE_XTRN);

			pSegment = getseg(ea);
			set_segm_end(ea, ea + 0x1F + 1, SEGMOD_KILL);
			set_segment_cmt(pSegment, "VDP mirror", false);
		}
	}

	pSegment = segIOP();
	if (!pSegment)
	{
		ea = 0x00A10000;
		add_segm(0, ea, 0x00BFFFFF + 1, SEGNAME_IOP, SEGTYPE_XTRN);

		pSegment = getseg(ea);
		set_segm_end(ea, 0x00BFFFFF + 1, SEGMOD_KILL);
		set_segment_cmt(pSegment, "I/O Ports", false);
	}

	pSegment = segZ80();
	if (!pSegment)
	{
		ea = 0x00A00000;
		add_segm(0, ea, ea + 0xFFFF + 1, SEGNAME_Z80, SEGTYPE_DATA);

		pSegment = getseg(ea);
		set_segm_end(ea, ea + 0xFFFF + 1, SEGMOD_KILL);
		set_segment_cmt(pSegment, "Z80 Memory", false);
	}
}

static void add_port(const char *name, ea_t addr, int size, const char *cmt = NULL)
{
	set_name(addr, name, SN_NOWARN);
	set_cmt(addr, cmt, true);

	switch (size)
	{
	case 2:
		doWord(addr, 2);
		break;
	case 4:
		doDwrd(addr, 4);
		break;
	default:
		doByte(addr, size);
		break;
	}
}

static void create_mappings()
{
	// VDP
	add_port("VDP_DATA", 0x00C00000, 2, "VDP Data");
	add_port("VDP_DATA_MIRROR", 0x00C00002, 2, "VDP Data mirror");
	add_port("VDP_CTRL", 0x00C00004, 2, "VDP Control");
	add_port("VDP_CTRL_MIRROR", 0x00C00006, 2, "VDP Control mirror");
	add_port("VDP_HVCNT", 0x00C00008, 2, "VDP H / V Counter");
	add_port("VDP_HVCNT_MIRROR", 0x00C0000A, 2, "VDP H / V Counter mirror");
	add_port("VDP_DEBUGREG", 0x00C0001C, 2, "VDP Debug Register");
	add_port("VDP_DEBUGREG_MIRROR", 0x00C0001E, 2, "VDP Debug Register mirror");

	add_port("PSG_REG", 0x00C00011, 2, "PSG");
	add_port("PSG_REG_MIRROR1", 0x00C00013, 2, "PSG Mirror 1");
	add_port("PSG_REG_MIRROR2", 0x00C00015, 2, "PSG Mirror 2");
	add_port("PSG_REG_MIRROR3", 0x00C00017, 2, "PSG Mirror 3");

	// I/O Ports
	add_port("IO_VERSION", 0x00A10000, 2, "Megadrive version number");
	add_port("TIME_REG", 0x00A13000, 0x100, "TIME Register");
	add_port("TMSS_REG", 0x00A14000, 2, "TMSS Register");

	// Port 1:
	add_port("IO_P1_CTRL", 0x00A10008, 2, "Port 1 Control");
	add_port("IO_P1_DATA", 0x00A10002, 2, "Port 1 Data");
	add_port("IO_P1_TXDATA", 0x00A1000E, 2, "Port 1 Tx Data");
	add_port("IO_P1_RXDATA", 0x00A10010, 2, "Port 1 Rx Data");
	add_port("IO_P1_SMODE", 0x00A10012, 2, "Port 1 S - Mode");

	// Port 2:
	add_port("IO_P2_CTRL", 0x00A1000A, 2, "Port 2 Control");
	add_port("IO_P2_DATA", 0x00A10004, 2, "Port 2 Data");
	add_port("IO_P2_TXDATA", 0x00A10014, 2, "Port 2 Tx Data");
	add_port("IO_P2_RXDATA", 0x00A10016, 2, "Port 2 Rx Data");
	add_port("IO_P2_SMODE", 0x00A10018, 2, "Port 2 S - Mode");

	// Port 3:
	// External Port found on early Megadrive 1 machines
	add_port("IO_P3_CTRL", 0x00A1000C, 2, "Port 3 Control");
	add_port("IO_P3_DATA", 0x00A10006, 2, "Port 3 Data");
	add_port("IO_P3_TXDATA", 0x00A1001A, 2, "Port 3 Tx Data");
	add_port("IO_P3_RXDATA", 0x00A1001C, 2, "Port 3 Rx Data");
	add_port("IO_P3_SMODE", 0x00A1001E, 2, "Port 3 S - Mode");

	// Control Area Ports :
	add_port("CTRL_MEMMODE", 0x00A11000, 2, "Control - Area Memory - Mode");
	add_port("CTRL_Z80BUSREQ", 0x00A11100, 2, "Control - Area Z80 Bus - Request");
	add_port("CTRL_Z80RESET", 0x00A11200, 2, "Control - Area Z80 Reset");
}

//-----------------------------------------------------------------------
//      Assembler Definition
//-----------------------------------------------------------------------
static const asm_t m68k_asm = {
	AS_COLON | ASH_HEXF4 | ASD_DECF0 | ASO_OCTF3 | ASB_BINF2 | AS_ONEDUP | AS_NOSPACE,
	0,
	"680x0 Assembler by Paul McKee",
	0,
	0,
	0,
	"org",
	"END",
	"*",
	'\'',
	'\'',
	"'",
	"dc.b",
	"dc.b",
	"dc.w",
	"dc.l",
	0,
	0,
	0,
	0,
	0,
	0,
	"dcb.#s(b,w,l) #d,#v",
	"ds.b %s",
	"equ",
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	".extern",
	0,
	0,
	0,
	'(',
	')',
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

static const asm_t *asms[] = { &m68k_asm, NULL };

//-----------------------------------------------------------------------
//      Processor Definition
//-----------------------------------------------------------------------
processor_t LPH =
{
	IDP_INTERFACE_VERSION,     //kernel version
	PLFM_68K,               //id
	PR_USE32 | PR_SGROTHER | PR_NO_SEGMOVE | PR_DEFSEG32 | PRN_HEX | PR_WORD_INS | PR_ALIGN | PR_BINMEM,
	8,                  // 8 bits in a byte for code segments
	8,                  // 8 bits in a byte for other segments

	shnames,
	lnames,

	asms,

	notify,

	header,
	footer,

	segstart,
	std_gen_segm_footer,

	NULL,//assumes,

	ana,//u_ana
	emu,//u_emu
	out,//u_out
	outop,//u_outop
	data,//d_out

	NULL,//cmp_opnd
	can_have_type,

	qnumber(RegNames),
	RegNames,
	NULL,//getreg

	0,//rFiles
	NULL,//rFnames
	NULL,//rFdescs
	NULL,//CPUregs

	r_Vcs, r_Vds,
	0,//segreg_size
	r_Vcs, r_Vds,

	NULL,//codestart
	retcodes,

	opc_null, opc_last,
	Instructions,

	NULL,//is_far_jump
	NULL,//translate
	0,//tbyte_size
	NULL,//reatcvt
	{ 0, 0, 0, 0 },//real_width

	NULL,//is_switch

	NULL,//gen_map_file
	NULL, //get_ref_addr,//extract_address

	is_sp_based,

	NULL,//create_func_frame
	NULL,//get_frame_retsize
	NULL,//gen_stkvar_def
	gen_spcdef,

	opc_rts,

	NULL,//set_idp_options,
	is_align_insn,
	NULL,
	0,
};

//-----------------------------------------------------------------------
