//
// Analyzer module for M68000
//

#include "m68k.hpp"
#include <ua.hpp>

//----------------------------------------------------------------------
int idaapi ana(void) {
	if (cmd.ip & 1) return 0;

	cmd.Op3.dtyp = 1;
	cmd.Op2.dtyp = 1;
	cmd.Op1.dtyp = 1;
	cmd.segpref = 0x80;

	uint16 opcode = ua_next_word();

	return 0;
};
