//
// Output module for M68000
//

#include "m68k.hpp"

void idaapi header(void) {
	MakeLine("My header line");
}

void idaapi footer(void) {
	MakeLine("My footer line");
}

void idaapi segstart(ea_t ea) {
}

void idaapi segend(ea_t ea) {
}

static void OutReg(uint16 reg)
{
	if (reg == r_a7) reg = r_sp;
	return out_line(ph.regNames[reg], COLOR_REG);
}

void idaapi data(ea_t ea)
{
	gl_name = 1;
	intel_data(ea);
}

bool idaapi outop(op_t &op) {
	switch (op.type)
	{
	case o_void: return 0;
	case o_reg: OutReg(op.reg); break;
	case o_phrase:
	{
		if ((op.reg & REG_EXT_FLAG) == REG_PRE_DECR) out_symbol('-');
		if (isDefArg(uFlag, op.n)) OutValue(op);

		out_symbol('(');
		OutReg(op.reg & REG_MASK + r_a0);
		out_symbol(')');

		if ((op.reg & REG_EXT_FLAG) == REG_POST_INCR) out_symbol('+');
	}
	case o_imm:
	{
	}
	case o_mem:
	{
	}
	}
}

void idaapi out(void) {
	char str[MAXSTR];

	init_output_buffer(str, sizeof(str));

	if (cmd.Op1.type != o_void)
		out_one_operand(0);

	if ((cmd.Op2.type != o_void) && (cmd.Op2.flags & OF_SHOW))
	{
		out_symbol(',');
		if (!(ash.flag & AS_NOSPACE)) OutChar(' ');
		out_one_operand(1);
	}

	if (cmd.Op3.type != o_void)
	{
		out_symbol(',');
		if (!(ash.flag & AS_NOSPACE)) OutChar(' ');
		out_one_operand(2);
	}

	if (cmd.Op4.type != o_void)
	{
		out_symbol(',');
		if (!(ash.flag & AS_NOSPACE)) OutChar(' ');
		out_one_operand(3);
	}

	if (isVoid(cmd.ea, uFlag, 0))
		OutImmChar(cmd.Op1);
	if (isVoid(cmd.ea, uFlag, 1))
		OutImmChar(cmd.Op2);
	if (isVoid(cmd.ea, uFlag, 2))
		OutImmChar(cmd.Op3);
	if (isVoid(cmd.ea, uFlag, 3))
		OutImmChar(cmd.Op4);

	term_output_buffer();
	gl_comm = 1; // enable comments
	MakeLine(str);
}
