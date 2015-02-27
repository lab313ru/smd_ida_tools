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
	uint8 v82 = (opcode >> 9) & 7;
	uint8 v81_lo = (opcode >> 3) & 7;
	uint16 v83 = opcode;
	uint8 v2 = (opcode >> 6) & 7;
	uint8 v3 = (opcode >> 6) & 7;
	uint8 v81_hi = opcode & 7;

	switch (opcode >> 12)
	{
	case 0x4:
		if (v2 == 7)
		{
		}
	}

	return 0;
};
