//
// Output module for M68000
//

#include "m68k.hpp"
#include <ints.hpp>

static void print_reg(uint16 reg)
{
	if (reg == r_a7) reg = r_sp;
	return out_line(ph.regNames[reg], COLOR_REG);
}

static void check_print_op1_hex_prefix()
{
	if (cmd.Op1.type == o_idpspec4)
		if (cmd.Op1.specflag1 & SPEC1_HEX)
			out_symbol('&');
}

static bool OutVarName(op_t &op)
{
	return (isDefArg(uFlag, op.n) ? 0 : out_name_expr(op, op.addr + 16 * cmd.cs, op.addr));
}

bool idaapi outop(op_t &op) {
	switch (op.type)
	{
	case o_void: return 0;
	case o_reg: print_reg(op.reg); break;
	case o_phrase:
	{
		if ((op.reg & REG_EXT_FLAG) == REG_PRE_DECR) out_symbol('-');
		if (isDefArg(uFlag, op.n)) OutValue(op);

		out_symbol('(');
		print_reg(op.reg & REG_MASK + r_a0);
		out_symbol(')');

		if ((op.reg & REG_EXT_FLAG) == REG_POST_INCR) out_symbol('+');

		check_print_op1_hex_prefix();
	}
	case o_imm:
	{
		out_symbol('#');

		switch (op.specflag1)
		{
		case 0: OutValue(op); break;
		case 1: OutValue(op, OOFW_16 | OOFS_NOSIGN); break;
		case 4:
		case 5:
		{
			OutValue(op, OOFW_8 | OOFS_NOSIGN);

			if (op.specflag1 != 4)
			{
				out_symbol(',');
				print_reg(op.reg);
			}
		} break;
		default: warning("out: %a: bad imm subtype %d", cmd.ip, op.specflag1); break;
		}
	}
	case o_mem:
	{
		if (op.specflag1 < 2) out_symbol('(');
		if (cmd.itype == lea || cmd.itype == pea || !OutVarName(op)) OutValue(op, OOF_ADDR | OOFW_32);

		if (op.specflag1 < 2)
		{
			out_symbol(')');

			if (ash.uflag & 1) // TODO: what the flag?
			{
				out_symbol('.');
				out_symbol((op.specflag1 != 0) ? 'l' : 'w');
			}
		}
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
