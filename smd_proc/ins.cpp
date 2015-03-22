//
//	Instruction set for M68000 (definitions)
//	Link: http://info.sonicretro.org/SCHG:68000_Instruction_Set
//

#include "ins.hpp"

instruc_t Instructions[] = {
	{ "", 0 },
	{ "abcd", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "adda.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "adda.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "addi.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "addi.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "addi.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "addq.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "addq.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "addq.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "add.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "add.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "add.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "addx.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "addx.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "addx.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "andi.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "andi.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "andi.l", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "andi", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "and.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "and.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "and.l", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "and", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "asl.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "asl.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "asl.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "asr.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "asr.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "asr.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "bhi.w", CF_USE1 },
	{ "bls.w", CF_USE1 },
	{ "bcc.w", CF_USE1 },
	{ "bcs.w", CF_USE1 },
	{ "bne.w", CF_USE1 },
	{ "beq.w", CF_USE1 },
	{ "bvc.w", CF_USE1 },
	{ "bvs.w", CF_USE1 },
	{ "bpl.w", CF_USE1 },
	{ "bmi.w", CF_USE1 },
	{ "bge.w", CF_USE1 },
	{ "blt.w", CF_USE1 },
	{ "bgt.w", CF_USE1 },
	{ "ble.w", CF_USE1 },

	{ "bhi.s", CF_USE1 },
	{ "bls.s", CF_USE1 },
	{ "bcc.s", CF_USE1 },
	{ "bcs.s", CF_USE1 },
	{ "bne.s", CF_USE1 },
	{ "beq.s", CF_USE1 },
	{ "bvc.s", CF_USE1 },
	{ "bvs.s", CF_USE1 },
	{ "bpl.s", CF_USE1 },
	{ "bmi.s", CF_USE1 },
	{ "bge.s", CF_USE1 },
	{ "blt.s", CF_USE1 },
	{ "bgt.s", CF_USE1 },
	{ "ble.s", CF_USE1 },

	{ "bchg", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "bclr", CF_CHG2 | CF_USE1 },

	{ "bra.w", CF_STOP | CF_USE1 },
	{ "bra.s", CF_STOP | CF_USE1 },

	{ "bset", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "bsr.w", CF_CALL | CF_USE1 },
	{ "bsr.s", CF_CALL | CF_USE1 },

	{ "btst", CF_USE1 | CF_USE2 },

	{ "chk.l", CF_CALL | CF_USE1 | CF_USE2 },
	{ "chk.w", CF_CALL | CF_USE1 | CF_USE2 },

	{ "clr.b", CF_CHG1 },
	{ "clr.w", CF_CHG1 },
	{ "clr.l", CF_CHG1 },

	{ "cmpa.w", CF_USE1 | CF_USE2 },
	{ "cmpa.l", CF_USE1 | CF_USE2 },

	{ "cmpi.b", CF_USE1 | CF_USE2 },
	{ "cmpi.w", CF_USE1 | CF_USE2 },
	{ "cmpi.l", CF_USE1 | CF_USE2 },

	{ "cmpm.b", CF_USE1 | CF_USE2 },
	{ "cmpm.w", CF_USE1 | CF_USE2 },
	{ "cmpm.l", CF_USE1 | CF_USE2 },

	{ "cmp.b", CF_USE1 | CF_USE2 },
	{ "cmp.w", CF_USE1 | CF_USE2 },
	{ "cmp.l", CF_USE1 | CF_USE2 },

	{ "dbcc", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbcs", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbeq", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbf", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbge", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbgt", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbhi", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dble", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbls", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dblt", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbmi", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbne", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbpl", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbt", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbvc", CF_CHG1 | CF_USE1 | CF_USE2 },
	{ "dbvs", CF_CHG1 | CF_USE1 | CF_USE2 },

	{ "divs.w", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "divu.w", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "eori.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "eori.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "eori.l", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "eori", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "eor.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "eor.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "eor.l", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "eor", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "exg", CF_CHG1 | CF_CHG2 | CF_USE2 },

	{ "ext.w", CF_CHG1 | CF_USE1 },
	{ "ext.l", CF_CHG1 | CF_USE1 },

	{ "illegal", 0 },

	{ "jmp", CF_STOP | CF_USE1 | CF_JUMP },

	{ "jsr", CF_CALL | CF_USE1 },

	{ "lea", CF_CHG2 | CF_USE1 },

	{ "link.w", CF_CHG1 | CF_USE2 },
	{ "link", CF_CHG1 | CF_USE2 },

	{ "lsl.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "lsl.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "lsl.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "lsr.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "lsr.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "lsr.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "movea.l", CF_CHG2 | CF_USE1 },
	{ "movea.w", CF_CHG2 | CF_USE1 },

	{ "movem.w", CF_CHG1 | CF_CHG2 },
	{ "movem.l", CF_CHG1 | CF_CHG2 },

	{ "movep.w", CF_CHG2 | CF_USE1 },
	{ "movep.l", CF_CHG2 | CF_USE1 },

	{ "moveq", CF_CHG2 | CF_USE1 },

	{ "move.b", CF_CHG2 | CF_USE1 },
	{ "move.w", CF_CHG2 | CF_USE1 },
	{ "move.l", CF_CHG2 | CF_USE1 },
	{ "move", CF_CHG2 | CF_USE1 },

	{ "muls.w", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "mulu.w", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "nbcd", CF_CHG1 | CF_USE1 },

	{ "neg.b", CF_CHG1 | CF_USE1 },
	{ "neg.w", CF_CHG1 | CF_USE1 },
	{ "neg.l", CF_CHG1 | CF_USE1 },

	{ "negx.b", CF_CHG1 | CF_USE1 },
	{ "negx.w", CF_CHG1 | CF_USE1 },
	{ "negx.l", CF_CHG1 | CF_USE1 },

	{ "nop", 0 },

	{ "not.b", CF_CHG1 | CF_USE1 },
	{ "not.w", CF_CHG1 | CF_USE1 },
	{ "not.l", CF_CHG1 | CF_USE1 },

	{ "ori.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "ori.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "ori.l", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "ori", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "or.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "or.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "or.l", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "or", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "pea", CF_USE1 },

	{ "reset", 0 },

	{ "rol.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "rol.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "rol.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "ror.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "ror.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "ror.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "roxl.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "roxl.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "roxl.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "roxr.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "roxr.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "roxr.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "rte", CF_STOP },

	{ "rtr", CF_STOP },

	{ "rts", CF_STOP },

	{ "sbcd", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "trapv", 0 },

	{ "scc", CF_CHG1 },
	{ "scs", CF_CHG1 },
	{ "seq", CF_CHG1 },
	{ "sf", CF_CHG1 },
	{ "sge", CF_CHG1 },
	{ "sgt", CF_CHG1 },
	{ "shi", CF_CHG1 },
	{ "sle", CF_CHG1 },
	{ "sls", CF_CHG1 },
	{ "slt", CF_CHG1 },
	{ "smi", CF_CHG1 },
	{ "sne", CF_CHG1 },
	{ "spl", CF_CHG1 },
	{ "st", CF_CHG1 },
	{ "svc", CF_CHG1 },
	{ "svs", CF_CHG1 },

	{ "stop", CF_STOP },

	{ "suba.l", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "suba.w", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "subi.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "subi.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "subi.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "subq.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "subq.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "subq.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "sub.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "sub.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "sub.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "subx.b", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "subx.w", CF_CHG2 | CF_USE1 | CF_USE2 },
	{ "subx.l", CF_CHG2 | CF_USE1 | CF_USE2 },

	{ "swap", CF_CHG1 | CF_USE1 },

	{ "tas", CF_CHG1 | CF_USE1 },

	{ "trap", CF_USE1 },

	{ "tst.b", CF_USE1 },
	{ "tst.w", CF_USE1 },
	{ "tst.l", CF_USE1 },

	{ "unlk", CF_USE1 },
};
