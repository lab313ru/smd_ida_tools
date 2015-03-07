//
//	Registers of M68000 (definitions)
//

#include "m68k.hpp"

static netnode helper;
ushort idpflags = MC68K_UNSIGNED_OPS;

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

static int idaapi notify(processor_t::idp_notify msgid, ...) {
	va_list va;

	va_start(va, msgid);
	int code = invoke_callbacks(HT_IDP, msgid, va);
	if (code) return code;

	code = 1;
	switch (msgid)
	{
	case processor_t::init:
		inf.mf = true;
		helper.create("$ mc68k");
		break;
	default: break;
	}

	va_end(va);
	return code;
}

static bool idaapi can_have_type(op_t &op) {
	switch (op.type)
	{
	case o_void:
	case o_mem:
	case o_phrase:
		return true;
	case o_reg:
		return ((op.reg & REG_EXT_FLAG) != REG_PRE_DECR) && ((op.reg & REG_EXT_FLAG) != REG_POST_INCR);
	}
	return false;
}

static bool idaapi is_switch(switch_info_ex_t *si) {
	return (cmd.itype == opc_jmp);
}

static const char *idaapi set_idp_options(const char *keyword, int value_type, const void *value) {
	if (keyword == NULL)
	{
		AskUsingForm_c(
			"HELP\n"
			"MC68K specific analyzer options\n"
			"Immediate operands are unsigned by default\n\n"

			"       If this option is off, then\n"
			"         a 8bit operand like 0xF0 will be represented as -0x10\n"
			"         a 16bit operand like 0xFF00 will be represented as -0x100\n"
			"         a 32bit operand like 0xFFFF0000 will be represented as -0x10000\n"
			"ENDHELP\n"

			"MC68K specific analyzer options\n\n"
			" <Immediate operands are ~u~nsigned by default:C>>\n\n\n",
			&idpflags
			);
	}
	else if (!strcmp(keyword, "MC68K_UNSIGNED_OPS"))
	{
		if (value_type != IDPOPT_BIT) return IDPOPT_BADTYPE;

		if (*(uint32 *)value)
			idpflags |= MC68K_UNSIGNED_OPS;
		else
			idpflags &= ~MC68K_UNSIGNED_OPS;
	}
	else return IDPOPT_BADKEY;

	helper.altset(-1, (nodeidx_t)idpflags);
	return IDPOPT_OK;
}

static int idaapi is_align_insn(ea_t ea) {
	return (get_byte(ea) == 0);
}

static char *shnames[] = {
	"M68000",
	NULL
};

static char *lnames[] = {
	"Motorola MC68000 (M68K)",
	NULL
};

static char *RegNames[] = {
	"d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
	"pc", "ccr", "sr", "usp", "sp", "cs", "ds"
};

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

//-----------------------------------------------------------------------
//      Processor Definition
//-----------------------------------------------------------------------
processor_t LPH =
{
	IDP_INTERFACE_VERSION,     //kernel version
	PLFM_68K,               //id
	PRN_HEX | PR_WORD_INS | PR_NO_SEGMOVE,
	8,                  // 8 bits in a byte for code segments
	8,                  // 8 bits in a byte for other segments

	shnames,
	lnames,

	asms,

	notify,

	header,
	footer,

	segstart,
	segend,

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

	r_cs, r_ds,
	0,//segreg_size
	r_cs, r_ds,

	NULL,//codestart
	retcodes,

	opc_null, opc_last,
	Instructions,

	NULL,//is_far_jump
	NULL,//translate
	0,//tbyte_size
	NULL,//reatcvt
	{ 0, 0, 0, 0 },//real_width

	is_switch,

	NULL,//gen_map_file
	NULL, //get_ref_addr,//extract_address

	is_sp_based,

	NULL,//create_func_frame
	NULL,//get_frame_retsize
	NULL,//gen_stkvar_def
	NULL,

	opc_rts,

	set_idp_options,
	NULL,
	NULL,
	0,
};

//-----------------------------------------------------------------------
