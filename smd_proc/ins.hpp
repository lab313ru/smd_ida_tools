//
//	Instruction set for M68000 (enum)
//	Link: http://info.sonicretro.org/SCHG:68000_Instruction_Set
//

#ifndef __INST_H
#define __INST_H

#include <idp.hpp>

enum m68k_opcodes {
	null, abcd, add, adda, addi, addq, addx, and, andi, asl, asr, b, bchg, bclr, bra,
	bset, bsr, btst, chk, clr, cmp, cmpa, cmpi, cmpm, db, divs, divu, eor, eori, exg,
	ext, illegal, jmp, jsr, lea, link, lsl, lsr, move, movea, movem, movep, moveq, muls,
	mulu, nbcd, neg, negx, nop, not, or, ori, pea, reset, rol, ror, roxl, roxr, rte, rtr,
	rts, sbcd, s, stop, sub, suba, subi, subq, subx, swap, tas, trap, trapv, tst, unlk, last
};

extern instruc_t Instructions[];

#endif
