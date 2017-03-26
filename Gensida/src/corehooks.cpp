#include "luascript.h"
#include "ggenie.h"
#include "corehooks.h"
#include "tracer.h"

extern "C" {
    uint32 hook_address;
    uint32 hook_address_cd;
    uint32 hook_value;
    uint32 hook_value_cd;
    uint32 hook_pc;
    uint32 hook_pc_cd;
}

#define defhook(name)\
void hook_##name##()\
{\
	hook_pc &= 0xFFFFFF;\
	trace_##name##();\
}
#define defhook_cd(name)\
void hook_##name##_cd()\
{\
	hook_pc_cd &= 0xFFFFFF;\
	trace_##name##_cd();\
}
#define defhooks(size)\
	defhook(read_##size)\
	defhook(write_##size)\
	defhook_cd(read_##size)\
	defhook_cd(write_##size)

#define defvramhook(name,size)\
void hook_##name##_vram_##size##()\
{\
	trace_##name##_vram_##size##();\
}

#define defvramhooks(size)\
	defvramhook(read,size)\
	defvramhook(write,size)

defhooks(byte)
defhooks(word)
defhooks(dword)

defvramhooks(byte)
defvramhooks(word)

void hook_exec()
{
    hook_pc &= 0xFFFFFF;
    trace_exec_pc();
}
void hook_exec_cd()
{
    hook_pc_cd &= 0xFFFFFF;
    trace_exec_pc_cd();
}