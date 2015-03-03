//
// Analyzer module for M68000
//

#include "m68k.hpp"
#include <ua.hpp>

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

// displ flags
#define WSIZE_SET	((1<<11))

static bool get_ea_2(int mode, op_t *op, char idx)
{
	if (mode > MODE_IMM) return false;

	if (mode == MODE_ABSW) mode += idx;

	op->reg = r_a0 + idx;
	switch (mode)
	{
	case MODE_DN: // D0
	{
		op->reg = idx;
	}
	case MODE_AN: // A0
	{
		op->type = o_reg;
		return true;
	}
	case MODE_iAN: // (A0)
	case MODE_ANp: // (A0)+
	case MODE_pAN: // -(A0)
	{
		op->type = o_phrase;
		op->phrase = idx | (8 * mode);
		return true;
	}
	case MODE_dAN: // $dddd(A0)
	{
		op->type = o_displ;
		op->specflag1 = 0;
		op->offb = (char)cmd.size;
		op->addr = (short)ua_next_word();
		return true;
	}
	case MODE_dPCXI:
	{
		op->reg = r_pc;
	}
	case MODE_dANXI: // $dd(A0,[AD]0)
	{
		op->type = o_displ;
		ea_t next_ip = cmd.ip + cmd.size;
		op->specflag2 = OF_NUMBER;

		uint16 w2 = ua_next_word();
		op->specflag1 = DISPL(w2); // displacement register
		op->specflag1 |= ((w2 & WSIZE_SET) ? 0 : SPEC1_WSIZE);

		op->offb = cmd.size - 1;
		op->addr = (char)w2;

		op->specflag2 |= ((op->addr) ? 0 : OF_NO_BASE_DISP);
		op->flags |= (op->specflag2 & OF_NO_BASE_DISP);

		if (!(op->specflag2 & OF_NO_BASE_DISP) || (op->reg == r_pc))
		{
			op->addr += next_ip;
		}

		return true;
	}
	case MODE_ABSW: // $dddd.w
	{
		op->type = o_mem;
		op->specflag1 = idx;
		op->offb = (char)cmd.size;
		op->addr = (short)ua_next_word();
		return true;
	}
	case MODE_ABSL:
	{
		op->addr = ua_next_long();
		op->specflag1 = idx;
		op->offb = (char)cmd.size;
		op->type = o_mem;
		return true;
	}
	case MODE_dPC:
	{
		op->addr = cmd.ip + cmd.size;
		op->addr += (short)ua_next_word();
		op->specflag1 = idx;
		op->offb = (char)cmd.size;
		op->type = o_mem;
		return true;
	}
	case MODE_IMM:
	{
		int imm_size = 4;
		op->type = o_imm;
		op->specflag1 = 0;

		switch (op->dtyp)
		{
		case dt_byte: op->value = (char)ua_next_word(); break;
		case dt_word: op->value = (short)ua_next_word(); break;
		case dt_dword: op->value = ua_next_long(); break;
		default: warning("ana: %a: bad opsize %d", cmd.ip, op->dtyp); break;
		}

		return true;
	}
	}
}

static char set_dtype_op1_op2(char sz)
{
	cmd.Op2.dtyp = sz;
	cmd.Op1.dtyp = sz;
	return sz + 1;
}

static void exchange_Op1_Op2()
{
	uchar n = cmd.Op1.n;
	ea_t addr = cmd.Op1.addr;
	ea_t specval = cmd.Op1.specval;
	uchar flags = cmd.Op1.flags;
	char specflag1 = cmd.Op1.specflag1;
	uval_t value = cmd.Op1.value;

	cmd.Op1.n = cmd.Op2.n;
	cmd.Op1.flags = cmd.Op2.flags;
	cmd.Op1.value = cmd.Op2.value;
	cmd.Op1.addr = cmd.Op2.addr;
	cmd.Op1.specval = cmd.Op2.specval;
	cmd.Op1.specflag1 = cmd.Op2.specflag1;

	cmd.Op2.n = n;
	cmd.Op2.flags = addr;
	cmd.Op2.value = value;
	cmd.Op2.addr = addr;
	cmd.Op2.specval = specval;
	cmd.Op2.specflag1 = specflag1;

	cmd.Op1.n = 0;
	cmd.Op2.n = 1;
}

static uint16 check_desa_adda_suba(line *d)
{
	if (d->opsz != 3) return 0;

	cmd.itype = ((d->line == 0xD) ? adda : suba);
	cmd.Op2.reg += 8;

	if (d->mode6 & 4)
	{
		cmd.Op1.dtyp = dt_dword;
		cmd.Op2.dtyp = dt_dword;
		cmd.segpref = 3;
	}
	else
	{
		cmd.segpref = 2;
	}

	return (get_ea_2(d->mode3, &cmd.Op1, d->reg0) ? cmd.size : 0);
}

static uint16 check_desa_addx_subx(line *d)
{
	if (d->mode6 < 4 && d->mode3 > 1) return 0;

	cmd.itype = ((d->line == 0xD) ? addx : subx);
	cmd.Op1.reg = d->reg0;
	cmd.Op1.type = o_reg;
	return cmd.size;
}

static uint16 check_desa_add_sub(line *d)
{
	if (!get_ea_2(d->mode3, &cmd.Op1, d->reg0)) return 0;
	if (!(d->mode6 & 4)) return cmd.size;

	exchange_Op1_Op2();
	return cmd.size;
}

static uint16 check_desa_abcd_sbcd(line *d)
{
	if (d->mode6 != 4) return 0;

	cmd.itype = ((d->line == 0xC) ? abcd : sbcd);
	cmd.Op2.dtyp = dt_byte;
	cmd.Op1.dtyp = dt_byte;

	cmd.Op1.reg = d->reg0;
	cmd.Op1.type = o_reg;
	return cmd.size;
}

static uint16 check_desa_mul_div(line *d)
{
	if (d->opsz != 3 || d->mode3 == 1) return 0;

	cmd.itype = (((d->line == 0xC) ? mulu : divu) - ((d->w & 0x100) ? 1 : 0));
	return (get_ea_2(d->mode3, &cmd.Op1, d->reg0) ? cmd.size : 0);
}

static uint16 check_desa_exg(line *d)
{
	cmd.itype = exg;
	cmd.Op1.type = o_reg;
	cmd.Op2.reg = d->reg0;
	cmd.Op1.reg = d->reg9;
	cmd.Op2.dtyp = dt_dword;
	cmd.Op1.dtyp = dt_dword;

	if (d->mode3)
	{
		cmd.Op2.reg += 8;

		if (d->mode6 == 5)
		{
			cmd.Op1.reg += 8;
			return cmd.size;
		}
		return ((d->mode6 == 6) ? cmd.size : 0);
	}
	else return ((d->mode6 == 5) ? cmd.size : 0);
}

static uint16 check_desa_and_or(line *d)
{
	if (d->mode6 >= 4 || d->mode3 <= 1) return 0;
	
	cmd.itype = ((d->line == 0xC) ? and : or);
	set_dtype_op1_op2(d->opsz);

	if (!get_ea_2(d->mode3, &cmd.Op1, d->reg0)) return 0;
	if (!(d->mode6 & 4)) return cmd.size;

	exchange_Op1_Op2();
	return cmd.size;
}

static uint16 check_desa_cmpa(line *d)
{
	if (d->opsz != 3) return 0;

	cmd.itype = cmpa;
	cmd.Op2.reg += 8;

	if (d->mode6 & 4)
	{
		cmd.Op1.dtyp = dt_dword;
		cmd.Op2.dtyp = dt_dword;
		cmd.segpref = 3;
	}
	else cmd.segpref = 2;

	return (get_ea_2(d->mode3, &cmd.Op1, d->reg0) ? cmd.size : 0);
}

static uint16 check_desa_eor_cmp(line *d)
{
	if (d->opsz == 3) return 0;

	cmd.itype = ((d->w & 0x100) ? eor : cmp);
	return (get_ea_2(d->mode3, &cmd.Op2, d->reg0) ? cmd.size : 0);
}

static uint16 desa_lineB(line *d)
{
	cmd.Op2.type = o_reg;
	cmd.Op2.reg = d->reg9;
	
	uint16 cmpa_retn = check_desa_cmpa(d);
	if (cmpa_retn) return cmpa_retn;

	set_dtype_op1_op2(d->opsz);

	uint16 eor_cmp = check_desa_eor_cmp(d);
	if (eor_cmp) return eor_cmp;
}

/**************
*
*   LINE 8 :
*   -OR
*   -SBCD
*   -DIVU
*
*
*   LINE C :
*   -EXG
*   -MULS,MULU
*   -ABCD
*   -AND
*
***************/

static uint16 desa_line8C(line *d)
{
	cmd.Op2.type = o_reg;
	cmd.Op2.reg = d->reg9;

	uint16 abcd_sbcd = check_desa_abcd_sbcd(d);
	if (abcd_sbcd) return abcd_sbcd;

	uint16 mul_div = check_desa_mul_div(d);
	if (mul_div) return mul_div;

	uint16 exg_retn = check_desa_exg(d);
	if (exg_retn) return exg_retn;

	uint16 and_or = check_desa_and_or(d);
	if (and_or) return and_or;

	return 0;
}

/**************
*
*   LINE 9 :
*   -SUB, SUBX, SUBA
*
*   LINE D :
*   -ADD, ADDX, ADDA
*
**************/

static uint16 desa_line9D(line *d)
{
	cmd.Op2.type = o_reg;
	cmd.Op2.reg = d->reg9;

	uint16 adda_suba = check_desa_adda_suba(d);
	if (adda_suba) return adda_suba;

	set_dtype_op1_op2(d->opsz);

	uint16 addx_subx = check_desa_addx_subx(d);
	if (addx_subx) return addx_subx;

	uint16 add_sub = check_desa_add_sub(d);
	if (add_sub) return add_sub;

	return 0;
}

/**************
*
*   LINE E :
*   -Shifting
*
* Format Reg: 1110 VAL D SZ I TY RG0
* Format Mem: 1110 0TY D 11 MODRG0
***************/

static uint16 desa_lineE(line *d)
{
	static const m68k_opcodes shift_instr[] = { asr, lsr, roxr, ror, asl, lsl, roxl, rol };
	
	char shift_reg;

	if (d->opsz == 3){
		if ((d->mode3 <= MODE_AN) || (d->mode3 == MODE_ABSW && d->reg0 > 1 /*dPC, dPCXI, IMM*/) || !get_ea_2(d->mode3, &cmd.Op1, d->reg0)) return false;

		shift_reg = (char)d->reg9;
	}
	else
	{
		set_dtype_op1_op2(d->opsz);
		cmd.Op2.reg = d->reg0;
		cmd.Op2.type = o_reg;

		if (d->mode3 & 4 /*pAN, dAN, dANXI, ABSW*/)
		{
			cmd.Op1.type = o_reg;
			cmd.Op1.reg = d->reg9;
		}
		else
		{
			cmd.Op1.type = o_imm;
			cmd.Op1.value = ((d->reg9) ? d->reg9 : 8);
			cmd.Op1.flags |= OF_NUMBER;
			cmd.Op1.specflag1 = 4;
		}
		shift_reg = (d->mode3 & 3);
	}
	cmd.itype = shift_instr[shift_reg | (d->mode6 & 4)];
	return cmd.size;
}

//----------------------------------------------------------------------
int idaapi ana(void) {
	if (cmd.ip & 1) return 0;

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
	case 0xB: return desa_lineB(&d);
	case 0x8:
	case 0xC: return desa_line8C(&d);
	case 0x9:
	case 0xD: return desa_line9D(&d);
	case 0xE: return desa_lineE(&d);
	}

	return 0;
};
