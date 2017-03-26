#ifndef _PWM_H_
#define _PWM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FASTCALL __fastcall

    extern unsigned short PWM_FIFO_R[8];
    extern unsigned short PWM_FIFO_L[8];
    extern unsigned int PWM_RP_R;
    extern unsigned int PWM_WP_R;
    extern unsigned int PWM_RP_L;
    extern unsigned int PWM_WP_L;
    extern unsigned int PWM_Cycles;
    extern unsigned int PWM_Cycle;
    extern unsigned int PWM_Cycle_Cnt;
    extern unsigned int PWM_Int;
    extern unsigned int PWM_Int_Cnt;
    extern unsigned int PWM_Mode;
    extern unsigned int PWM_Enable;
    extern unsigned int PWM_Out_R;
    extern unsigned int PWM_Out_L;
    extern int PWM_Out_L_Tmp;
    extern int PWM_Out_R_Tmp;

    extern unsigned short PWMVol;

    void FASTCALL PWM_Init(void);
    void FASTCALL PWM_Set_Cycle(unsigned int cycle);
    void FASTCALL PWM_Set_Int(unsigned int int_time);
    void FASTCALL PWM_Clear_Timer(void);
    void FASTCALL PWM_Update_Timer(unsigned int cycle);
    void FASTCALL PWM_Update(int **buf, int length);

#ifdef __cplusplus
};
#endif

#endif
