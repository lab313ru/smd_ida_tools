//
//	Registers of M68000 (enum)
//

#ifndef M68K_HPP_INCLUDED
#define M68K_HPP_INCLUDED

#define PLFM_M68K 0x8666

#include <loader.hpp>
#include "ins.hpp"

enum regno_t ENUM_SIZE(uint16) {
	r_d0 = 0, r_d1, r_d2, r_d3, r_d4, r_d5, r_d6, r_d7,
		r_a0, r_a1, r_a2, r_a3, r_a4, r_a5, r_a6, r_a7,
		r_pc, r_ccr, r_sr, r_usp, r_sp, r_Vcs, r_Vds
};

extern netnode helper;

segment_t* segROM();
segment_t* segRAM();
segment_t* segIOP();

// general regs mask
#define REG_MASK 0x7

// ext reg flags (pre-, post- addressing)
#define REG_EXT_FLAG    0x38
#define   REG_PRE_DECR  0x20
#define   REG_POST_INCR 0x18

void idaapi header(void);
void idaapi footer(void);

void idaapi segstart(ea_t ea);

int idaapi ana(void);
int idaapi emu(void);
void idaapi out(void);
bool idaapi outop(op_t &x);
void idaapi data(ea_t ea);

bool idaapi can_have_type(op_t &op);
int idaapi is_align_insn(ea_t ea);
int idaapi is_sp_based(const op_t &x);
int idaapi is_sane_insn(int nocrefs);

#endif
