#ifndef _CPU_SH2_H
#define _CPU_SH2_H

#ifdef __cplusplus
extern "C" {
#endif

    extern int MSH2_Speed;
    extern int SSH2_Speed;

    int MSH2_Init();
    int SSH2_Init();
    void MSH2_Reset();
    void SSH2_Reset();
    void MSH2_Reset_CPU();
    void SSH2_Reset_CPU();
    void _32X_Set_FB();

#ifdef __cplusplus
};
#endif

#endif
