//
// Emulator module for M68000
//

#include "m68k.hpp"
#include <offset.hpp>

bool no_stop_flag = false;

static ea_t get_reg_base_addr(uint16 reg)
{
	insn_t cmd_copy;

	memcpy(&cmd_copy, &cmd, sizeof(cmd));
	ea_t ea = cmd.ea;
	cmd_copy.flags = cmd.flags;

	ea_t retn = BADADDR;

	if (decode_prev_insn(ea) != BADADDR && cmd.Op1.type == o_mem)
	{
		if (cmd.itype == lea && cmd.Op2.type == o_reg && cmd.Op2.phrase == reg || cmd.itype == pea && reg == 95 /*TODO: What is 95?*/)
			retn = cmd.Op1.addr + 16 * cmd.cs;
	}
	memcpy(&cmd, &cmd_copy, sizeof(cmd));
	cmd.flags = cmd_copy.flags;
	return retn;
}

static void doImmdValue(op_t *op)
{
	doImmd(cmd.ea);

	if (cmd.itype == adda)
	{
		if (op->n == 0 && op->type == o_imm && !isDefArg(uFlag, 0) && cmd.Op2.type == o_reg)
		{
			ea_t reg_base_addr = get_reg_base_addr(cmd.Op2.reg);

			if (reg_base_addr != BADADDR)
				op_offset(cmd.ea, op->n, REF_OFF32 | REFINFO_NOBASE, BADADDR, reg_base_addr);
		}
	}
	else if (cmd.itype == addi && !isDefArg(uFlag, op->n) && op->type == o_imm)
	{
		ea_t base_95 = get_reg_base_addr(95);
		if ((op->n != 0) || !is_sp_based(cmd.Op2) || (base_95 != BADADDR))
		{
			if (op->value < 0 && !is_invsign(cmd.ea, uFlag, op->n))
				toggle_sign(cmd.ea, op->n);
		}
		else
		{
			op_offset(cmd.ea, op->n, REF_OFF32 | REFINFO_NOBASE, BADADDR, base_95);
		}
	}
}

static void DataSet(ea_t mem_base, ea_t ea, op_t *op, bool isLoad)
{
	if (cmd.itype != lea && cmd.itype != pea)
	{
		ua_dodata2(mem_base, ea, op->dtyp);
		ua_add_dref(mem_base, ea, (isLoad) ? dr_R : dr_W);
	}
	else if (op_adds_xrefs(uFlag, op->n))
	{
	}
}

static void TouchArg(op_t *op, bool isAlt, bool isLoad)
{
	switch (op->type)
	{
	case o_imm:
	{
		doImmdValue(op);

		if (!op->specflag1 && !isAlt && !(op->flags & OF_NUMBER) && isOff(uFlag, op->n))
		{
			ea_t offbase = get_offbase(cmd.ea, op->n);

			if (offbase != BADADDR)
				ua_add_dref(op->offb, offbase + op->value, dr_O);
		}
		return;
	}
	case o_mem:
	{
		if (cmd.itype != jmp && cmd.itype != jsr)
		{
			if (!isAlt)
			{
			}
		}
	}
	case o_near:
	{
		if (!isAlt)
		{
			ea_t offbase_addr;
			if (isOff(uFlag, op->n))
			{
				ea_t offbase = get_offbase(cmd.ea, op->n);

				if (offbase != BADADDR)
					offbase_addr = offbase + op->addr;
			}
			if (cmd.itype == jsr || cmd.itype == bsr)
			{
				ua_add_cref(op->offb, offbase_addr, fl_CN);

				if (!func_does_return(offbase_addr))
					no_stop_flag = false;
			}
			else
			{
				ua_add_cref(op->offb, offbase_addr, fl_JN);
			}
		}
		return;
	}
	}
}

int idaapi is_sp_based(const op_t &op)
{
	return ((op.type == o_phrase) && ((op.reg & 7 + r_a0) == r_a7)) || ((op.type == o_displ) && (op.reg == r_a7));
}

int idaapi emu(void) {
	uint32 feature = 0;

	int instr_start = ph.instruc_start;
	uint16 itype = cmd.itype;

	if (itype < instr_start || itype >= ph.instruc_end)
		feature = 0;
	else
		feature = ph.instruc[cmd.itype - instr_start].feature;

	ea_t ea = cmd.ea;
	no_stop_flag = ~(feature & CF_STOP);
	bool forced_op1 = is_forced_operand(ea, 0);
	bool forced_op2 = is_forced_operand(ea, 1);
	bool forced_op3 = is_forced_operand(ea, 2);

	if (feature & CF_USE1)
}
