//
//	Instruction set for M68000 (enum)
//	Link: http://info.sonicretro.org/SCHG:68000_Instruction_Set
//

#ifndef __INST_H
#define __INST_H

#include <idp.hpp>

enum m68k_opcodes {
	opc_null, opc_abcd, opc_add, opc_adda, opc_addi, opc_addq, opc_addx, opc_and, opc_andi, opc_asl, opc_asr, opc_b, opc_bchg, opc_bclr, opc_bra,
	opc_bset, opc_bsr, opc_btst, opc_chk, opc_clr, opc_cmp, opc_cmpa, opc_cmpi, opc_cmpm, opc_db, opc_divs, opc_divu, opc_eor, opc_eori, opc_exg,
	opc_ext, opc_illegal, opc_jmp, opc_jsr, opc_lea, opc_link, opc_lsl, opc_lsr, opc_move, opc_movea, opc_movem, opc_movep, opc_moveq, opc_muls,
	opc_mulu, opc_nbcd, opc_neg, opc_negx, opc_nop, opc_not, opc_or, opc_ori, opc_pea, opc_reset, opc_rol, opc_ror, opc_roxl, opc_roxr, opc_rte, opc_rtr,
	opc_rts, opc_sbcd, opc_s, opc_stop, opc_sub, opc_suba, opc_subi, opc_subq, opc_subx, opc_swap, opc_tas, opc_trap, opc_trapv, opc_tst, opc_unlk, opc_last
};

extern instruc_t Instructions[];

#endif
