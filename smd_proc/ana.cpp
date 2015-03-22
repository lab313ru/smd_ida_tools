//
// Analyzer module for M68000
//

#include "m68k.hpp"

enum addr_modes
{
	MODE_DN = 0,
	MODE_AN,
	MODE_iAN,
	MODE_ANp,
	MODE_pAN,
	MODE_dAN,
	MODE_dANXI,
	MODE_ABSW,
	MODE_ABSL,
	MODE_dPC,
	MODE_dPCXI,
	MODE_IMM
};

typedef struct
{
	int w;
	int reg9;
	int mode3;
	int mode6;
	int reg0;
	int line;
	int opsz;
} line;

#define REG0(W)		(((W))&7)
#define REG9(W)		(((W)>>9)&7)
#define OPSZ(W)		(((W)>>6)&3)
#define LINE(W)		(((W)>>12)&15)
#define DISPL(W)	(((W)>>12)&15)
#define MODE3(W)	(((W)>>3)&7)
#define MODE6(W)	(((W)>>6)&7)

bool idaapi can_have_type(op_t &op) {
	switch (op.type)
	{
	case o_void:
		return false;
	case o_mem:
	case o_phrase:
	case o_displ:
		return true;
	case o_reg:
		return ((op.reg & REG_EXT_FLAG) != REG_PRE_DECR) && ((op.reg & REG_EXT_FLAG) != REG_POST_INCR);
	}
	return false;
}

int idaapi is_align_insn(ea_t ea) {
	return (get_byte(ea) == 0);
}

int idaapi is_sane_insn(int nocrefs)
{
	return (cmd.ip & 1);
}

//----------------------------------------------------------------------
static bool desa_check_movep(line &d)
{
	uint16 val = d.w & 0170770;
	switch (val)
	{
	case 0000410:
	case 0000510:
		cmd.itype = (val == 0000410) ? opc_movepw : opc_movepl;

		cmd.Op1.type = o_displ;
		cmd.Op2.type = o_reg;

		cmd.Op1.specflag1 = 0;
		cmd.Op2.specflag1 = 0;

		cmd.Op1.offb = (char)cmd.size;
		cmd.Op2.offb = (char)cmd.size;

		cmd.Op1.phrase = r_a0 + d.reg0;
		cmd.Op2.reg = r_d0 + d.reg9;

		cmd.Op1.addr = (short)ua_next_word();

		return true;

	case 0000610:
	case 0000710:
		cmd.itype = (val == 0000610) ? opc_movepw : opc_movepl;

		cmd.Op2.type = o_displ;
		cmd.Op1.type = o_reg;

		cmd.Op2.specflag1 = 0;
		cmd.Op1.specflag1 = 0;

		cmd.Op2.offb = (char)cmd.size;
		cmd.Op1.offb = (char)cmd.size;

		cmd.Op2.phrase = r_a0 + d.reg0;
		cmd.Op1.reg = r_d0 + d.reg9;

		cmd.Op2.addr = (short)ua_next_word();

		return true;
	}

	return false;
}

static bool desa_check_bitop(line &d)
{
	uint16 val = d.w & 0170700;
	switch (val)
	{
	case 0000400:
	case 0000500:
	case 0000600:
	case 0000700:
		cmd.Op1.type = o_reg;
		cmd.Op1.reg = r_d0 + d.reg9;

		switch (val)
		{
		case 0000400:
			cmd.itype = opc_btst;
		case 0000500:
			cmd.itype = opc_bchg;
		case 0000600:
			cmd.itype = opc_bclr;
		case 0000700:
			cmd.itype = opc_bset;
		}

		break;
	default:
		return false;
	}

	val = d.w & 0177700;
	switch (val)
	{
	case 0004000:
		cmd.itype = opc_btst;
		break;
	case 0004100:
		cmd.itype = opc_bchg;
		break;
	case 0004200:
		cmd.itype = opc_bclr;
		break;
	case 0004300:
		cmd.itype = opc_bset;
		break;
	default:
		return false;
	}

	/*
	*{"btst", 2,	one(0000400),	one(0170700), "Dd;b", m68000up | mcfisa_a },
	{"btst", 4,	one(0004000),	one(0177700), "#b@s", m68000up },
	*
	*{"bchg", 2,	one(0000500),	one(0170700), "Dd$s", m68000up | mcfisa_a },
	{"bchg", 4,	one(0004100),	one(0177700), "#b$s", m68000up },
	*
	*{"bclr", 2,	one(0000600),	one(0170700), "Dd$s", m68000up | mcfisa_a },
	{"bclr", 4,	one(0004200),	one(0177700), "#b$s", m68000up },
	*
	*{"bset", 2,	one(0000700),	one(0170700), "Dd$s", m68000up | mcfisa_a },
	{"bset", 4,	one(0004300),	one(0177700), "#b$s", m68000up },
	*
	*/
}

static uint16 desa_line0(line &d)
{
	if (desa_check_movep(d))
		return cmd.size;

	return 0;
}

//----------------------------------------------------------------------
int idaapi ana(void) {
	cmd.Op1.dtyp = dt_word;
	cmd.Op2.dtyp = dt_word;
	cmd.Op3.dtyp = dt_word;

	line d;
	d.w = ua_next_word();
	d.reg9 = REG9(d.w);
	d.mode3 = MODE3(d.w);
	d.mode6 = MODE6(d.w);
	d.reg0 = REG0(d.w);
	d.line = LINE(d.w);
	d.opsz = OPSZ(d.w);

	switch (d.line)
	{
	case 0x0:
		return desa_line0(d);
	}

	return 0;
};
