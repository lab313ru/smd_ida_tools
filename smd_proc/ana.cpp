//
// Analyzer module for M68000
//

#include "m68k.hpp"

enum adressingword
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
	return 1;
}

//----------------------------------------------------------------------
int idaapi ana(void) {
	if (cmd.ip & 1) return 0;

	cmd.Op1.dtyp = dt_word;
	cmd.Op2.dtyp = dt_word;
	cmd.Op3.dtyp = dt_word;

	return 0;
};
