//
// Emulator module for M68000
//

#include "m68k.hpp"

static bool fFlow;

int idaapi is_sp_based(const op_t &op)
{
	return ((op.type == o_phrase) && ((op.reg & 7 + r_a0) == r_a7)) || ((op.type == o_displ) && (op.reg == r_a7));
}

int idaapi emu(void) {
	uint32 dwFeature = cmd.get_canon_feature();
	fFlow = !(dwFeature & CF_STOP);

	if (fFlow) ua_add_cref(0, cmd.ea + cmd.size, fl_F);

	return 1;
}
