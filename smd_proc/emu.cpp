//
// Emulator module for M68000
//

#include "m68k.hpp"

int idaapi is_sp_based(const op_t &op)
{
	return ((op.type == o_phrase) && ((op.reg & 7 + r_a0) == r_a7)) || ((op.type == o_displ) && (op.reg == r_a7));
}

int idaapi emu(void) {
	return 1;
}
