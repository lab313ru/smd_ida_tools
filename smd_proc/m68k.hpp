//
//	Registers of M68000 (enum)
//

#ifndef __M68K_HPP
#define __M68K_HPP

#include <pro.h>
#include <idp.hpp>
#include "ins.hpp"

#pragma pack(1)

enum m68k_regs {
	r_d0, r_d1, r_d2, r_d3, r_d4, r_d5, r_d6, r_d7,
	r_a0, r_a1, r_a2, r_a3, r_a4, r_a5, r_a6, r_a7,
	r_pc, r_ccr, r_sr, r_usp, r_sp, r_cs, r_ds
};

// specflag1 flags
#define SPEC1_WSIZE 0x10

int idaapi ana(void);

#pragma pack()
#endif
