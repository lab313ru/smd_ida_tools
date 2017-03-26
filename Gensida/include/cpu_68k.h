#include "star_68k.h"

#ifndef _CPU_68K_H
#define _CPU_68K_H

#ifdef __cplusplus
extern "C" {
#endif

    extern struct S68000CONTEXT Context_68K;

    int M68K_Init();
    int S68K_Init();
    void M68K_Reset(int System_ID, char Hard);
    void S68K_Reset();
    void M68K_Reset_CPU();
    void S68K_Reset_CPU();
    void M68K_32X_Mode();
    void M68K_Set_32X_Rom_Bank();
    void M68K_Set_Prg_Ram();
    void MS68K_Set_Word_Ram();

#ifdef __cplusplus
};
#endif

#endif
