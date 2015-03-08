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
#define SEGTYPE_DATA "DATA"
#define SEGTYPE_XTRN "XTRN"
#define NONEPROC "NONE"

netnode helper;
char szDevice[MAXSTR] = "";
char szDeviceParams[MAXSTR] = "";

static size_t nIOPorts;
static ioport_t* pIOPorts;
static char szCfgFile[] = "smd.cfg";

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

typedef struct cfg_entry_t
{
	qstring strName;
	qstring strComment;
	ea_t eaLocation;
}
cfg_entry;

static qvector<cfg_entry> qvEntries;
static qvector<cfg_entry> qvAliases;

static int idaapi notify(processor_t::idp_notify msgid, ...);
static const char *idaapi set_idp_options(const char *keyword, int value_type, const void *value);
static const char *idaapi parse_callback(const ioport_t*, size_t, const char* szLine);
static bool parse_config_file();
static void set_device_name(const char* szName);
static void setup_device();
static void create_mappings();

segment_t* segROM() { return get_segm_by_name(SEGNAME_ROM); }
segment_t* segRAM() { return get_segm_by_name(SEGNAME_RAM); }
segment_t* segVDP() { return get_segm_by_name(SEGNAME_VDP); }
segment_t* segIOP() { return get_segm_by_name(SEGNAME_IOP); }

static int idaapi notify(processor_t::idp_notify msgid, ...) {
	va_list va;
	segment_t *pSegment;

	va_start(va, msgid);
	int code = invoke_callbacks(HT_IDP, msgid, va);
	if (code) return code;

	switch (msgid)
	{
	case processor_t::init:
		inf.mf = true;
		helper.create("$ mc68k");
		break;

	case processor_t::term:
		free_ioports(pIOPorts, nIOPorts);
		break;

	case processor_t::newfile:
		pSegment = get_first_seg();
		if (pSegment)
		{
			set_segm_name(pSegment, SEGNAME_ROM);
			helper.altset(-1, pSegment->startEA);
		}
		setup_device();
		create_mappings();
		break;

	case processor_t::oldfile:
		if (helper.supval(-1, szDevice, sizeof(szDevice)) > 0)
			set_device_name(szDevice);
		break;

	case processor_t::is_sane_insn:
		return is_sane_insn(va_arg(va, int));
	}

	va_end(va);

	return 1;
}

static const char* idaapi set_idp_options(const char* szKeyword, int, const void*)
{
	if (szKeyword) return IDPOPT_BADKEY;
	setup_device();
	return IDPOPT_OK;
}

static const char *idaapi parse_callback(const ioport_t*, size_t, const char* szLine)
{
	cfg_entry entry;
	char szId[MAXSTR];
	const char* szComment;
	ea_t ea;
	size_t cch;

	if (qsscanf(szLine, "entry %s %" FMT_EA "i%n", szId, &ea, &cch) == 2)
	{
		szComment = skipSpaces(szLine + cch);
		entry.strName = szId;
		entry.strComment = *szComment ? szComment : "";
		entry.eaLocation = ea;
		qvEntries.push_back(entry);
		return NULL;
	}

	if (qsscanf(szLine, "alias %s %" FMT_EA "i%n", szId, &ea, &cch) == 2)
	{
		szComment = skipSpaces(szLine + cch);
		entry.strName = szId;
		entry.strComment = *szComment ? szComment : "";
		entry.eaLocation = ea;
		qvAliases.push_back(entry);
		return NULL;
	}

	return NULL;
}

static bool parse_config_file()
{
	char szPath[QMAXPATH];

	if (!qstrcmp(szDevice, NONEPROC))
		return true;

	if (!getsysfile(szPath, sizeof(szPath), szCfgFile, CFG_SUBDIR))
	{
		warning("ICON ERROR\nCan not open %s, I/O port definitions are not loaded", szCfgFile);
		return false;
	}

	szDeviceParams[0] = '\0';

	free_ioports(pIOPorts, nIOPorts);
	qvEntries.clear();
	qvAliases.clear();
	pIOPorts = read_ioports(&nIOPorts, szPath, szDevice, sizeof(szDevice), parse_callback);

	return true;
}

static void set_device_name(const char* szName)
{
	if (szName)
	{
		qstrncpy(szDevice, szName, sizeof(szDevice));
		helper.supset(-1, szDevice);
	}
}

static void setup_device()
{
	segment_t *pSegment;
	ea_t ea;

	if (!choose_ioport_device(szCfgFile, szDevice, sizeof(szDevice), NULL))
		return;

	set_device_name(szDevice);
	parse_config_file();

	if (!get_first_seg())
		return;

	noUsed(0, BADADDR);

	pSegment = getseg(helper.altval(-1));
	if (!pSegment) pSegment = get_first_seg();

	if (pSegment)
	{
		set_segm_end(pSegment->startEA, pSegment->startEA + 0x00400000, SEGMOD_KILL);
		set_segm_name(pSegment, SEGNAME_ROM);
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
		add_segm(0, ea, ea + 0x00B00000, SEGNAME_IOP, SEGTYPE_XTRN);
	}

	if (pSegment)
	{
		pSegment = getseg(ea);
		set_segm_end(ea, ea + 0x00B00000, SEGMOD_KILL);
		set_segment_cmt(pSegment, "I/O Ports", false);
	}
}

static void create_mappings()
{
	char szComment[MAXSTR];
	segment_t* pSegment;
	ioport_t* pPort;
	ea_t ea;
	uint8 opcode;
	bool fJmp0, fJmp1;
	size_t i;

	pSegment = segIOP();
	if (pSegment)
	{
		for (i = 0; i < nIOPorts; ++i)
		{
			pPort = pIOPorts + i;
			ea = toEA(pSegment->sel, pPort->address);
			if (isEnabled(ea))
			{
				set_name(ea, pPort->name, SN_NOWARN);
				if (pPort->cmt)
				{
					qsnprintf(szComment, sizeof(szComment), "%0.2Xh/%u %s", pPort->address, pPort->address, pPort->cmt);
					set_cmt(ea, szComment, false);
				}
			}
		}
	}
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
	PLFM_M68K,               //id
	PR_USE32 | PR_DEFSEG32 | PRN_HEX | PR_WORD_INS | PR_ALIGN | PR_BINMEM,
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

	set_idp_options,
	is_align_insn,
	NULL,
	0,
};

//-----------------------------------------------------------------------
