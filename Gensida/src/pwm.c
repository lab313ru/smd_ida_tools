#include "pwm.h"

#include "sh2.h"
#include "mem_sh2.h"
#include <memory.h>

#define PWM_BUF_SIZE 4
#if PWM_BUF_SIZE == 8
unsigned char PWM_FULL_TAB[] = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40
};
#elif PWM_BUF_SIZE == 4
unsigned char PWM_FULL_TAB[] = {
    0x40, 0x00, 0x00, 0x80,
    0x80, 0x40, 0x00, 0x00,
    0x00, 0x80, 0x40, 0x00,
    0x00, 0x00, 0x80, 0x40
};
#endif
/*unsigned short PWM_FIFO_R[8];
unsigned short PWM_FIFO_L[8];
unsigned int PWM_RP_R;
unsigned int PWM_WP_R;
unsigned int PWM_RP_L;
unsigned int PWM_WP_L;
unsigned int PWM_Cycles;
unsigned int PWM_Cycle;
unsigned int PWM_Cycle_Cnt;
unsigned int PWM_Int;
unsigned int PWM_Int_Cnt;
unsigned int PWM_Mode;
unsigned int PWM_Enable;
unsigned int PWM_Out_R;
unsigned int PWM_Out_L;*/
unsigned int PWM_Cycle_Tmp;
unsigned int PWM_Int_Tmp;
unsigned int PWM_FIFO_L_Tmp;
unsigned int PWM_FIFO_R_Tmp;
unsigned short PWMVol;

void FASTCALL PWM_Init(void)
{
    PWM_Mode = 0;
    PWM_Out_R = PWM_Out_L = 0;
    memset(&PWM_FIFO_R, 0, PWM_BUF_SIZE * 2);
    memset(&PWM_FIFO_L, 0, PWM_BUF_SIZE * 2);
    PWM_RP_R = PWM_WP_R = 0;
    PWM_RP_L = PWM_WP_L = 0;
    PWM_Cycle_Tmp = PWM_Int_Tmp = 0;
    PWM_FIFO_L_Tmp = PWM_FIFO_R_Tmp = 0;
    PWM_Set_Cycle(0);
    PWM_Set_Int(0);
}

void FASTCALL PWM_Set_Cycle(unsigned int cycle)
{
    PWM_Cycle_Tmp = cycle--;
    PWM_Cycle = (cycle & 0xFFF);
    PWM_Cycle_Cnt = cycle = PWM_Cycles;
}

void FASTCALL PWM_Set_Int(unsigned int int_time)
{
    int_time &= 0xF;
    if (int_time)
        PWM_Int = PWM_Int_Cnt = int_time;
    else
        PWM_Int = PWM_Int_Cnt = 16;
}

void FASTCALL PWM_Clear_Timer(void)
{
    PWM_Cycle_Cnt = 0;
}

void FASTCALL PWM_Shift_Data()
{
    switch (PWM_Mode & 0xF)
    {
    case 0x1:
    case 0xD:
        //Right -> Ignore and Left -> Left
        if (PWM_RP_L != PWM_WP_L) // Make sure Left channel FIFO isn't empty
        {
            PWM_Out_L = PWM_FIFO_L[PWM_RP_L]; // get Left channel output from Left channel FIFO
            PWM_RP_L = (PWM_RP_L + 1) & (PWM_BUF_SIZE - 1); //Increment Left channel read pointer, resetting to 0 if it overflows
        }
        break;

    case 0x2:
    case 0xE:
        //Right -> Ignore and Left -> Right
        if (PWM_RP_L != PWM_WP_L) // Make sure Left channel FIFO isn't empty
        {
            PWM_Out_R = PWM_FIFO_L[PWM_RP_L]; // get Right channel output from Left channel FIFO
            PWM_RP_L = (PWM_RP_L + 1) & (PWM_BUF_SIZE - 1); //Increment Left channel read pointer, resetting to 0 if it overflows
        }
        break;

    case 0x4:
    case 0x7:
        //Right -> Left and Left -> Ignore
        if (PWM_RP_R != PWM_WP_R) // Make sure Right channel FIFO isn't empty
        {
            PWM_Out_L = PWM_FIFO_R[PWM_RP_L]; // get Left channel output from Right channel FIFO
            PWM_RP_R = (PWM_RP_R + 1) & (PWM_BUF_SIZE - 1); //Increment Right channel read pointer, resetting to 0 if it overflows
        }
        break;

    case 0x5:
    case 0x9:
        //Right -> Right and Left -> Left
        if (PWM_RP_L != PWM_WP_L) // Make sure Left channel FIFO isn't empty
        {
            PWM_Out_L = PWM_FIFO_L[PWM_RP_L]; // get Left channel output from Left channel FIFO
            PWM_RP_L = (PWM_RP_L + 1) & (PWM_BUF_SIZE - 1); //Increment Left channel read pointer, resetting to 0 if it overflows
        }
        if (PWM_RP_R != PWM_WP_R) // Make sure Right channel FIFO isn't empty
        {
            PWM_Out_R = PWM_FIFO_R[PWM_RP_R]; // get Right channel output from Right channel FIFO
            PWM_RP_R = (PWM_RP_R + 1) & (PWM_BUF_SIZE - 1); //Increment Right channel read pointer, resetting to 0 if it overflows
        }
        break;

    case 0x6:
    case 0xA:
        //Right -> Left and Left -> Right
        if (PWM_RP_L != PWM_WP_L) // Make sure Left channel FIFO isn't empty
        {
            PWM_Out_R = PWM_FIFO_L[PWM_RP_L]; // get Right channel output from Left channel FIFO
            PWM_RP_L = (PWM_RP_L + 1) & (PWM_BUF_SIZE - 1); //Increment Left channel read pointer, resetting to 0 if it overflows
        }
        if (PWM_RP_R != PWM_WP_R) // Make sure Right channel FIFO isn't empty
        {
            PWM_Out_L = PWM_FIFO_R[PWM_RP_R]; // get Left channel output from Right channel FIFO
            PWM_RP_R = (PWM_RP_R + 1) & (PWM_BUF_SIZE - 1); //Increment Right channel read pointer, resetting to 0 if it overflows
        }
        break;

    case 0x8:
    case 0xB:
        //Right -> Right and Left -> Ignore
        if (PWM_RP_R != PWM_WP_R) // Make sure Right channel FIFO isn't empty
        {
            PWM_Out_R = PWM_FIFO_R[PWM_RP_L]; // get Right channel output from Right channel FIFO
            PWM_RP_R = (PWM_RP_R + 1) & (PWM_BUF_SIZE - 1); //Increment Right channel read pointer, resetting to 0 if it overflows
        }
        break;

    default:
        //Right -> Ignore and Left -> Ignore
        break;
    }
}

void FASTCALL PWM_Update_Timer(unsigned int cycle)
{
    if ((PWM_Enable || (PWM_Mode & 0xF)) && PWM_Cycle && (PWM_Cycle_Cnt <= cycle))
    {
        PWM_Shift_Data();

        PWM_Cycle_Cnt += PWM_Cycle;

        PWM_Int_Cnt--;
        if (!PWM_Int_Cnt)
        {
            PWM_Int_Cnt = PWM_Int;
            if (_32X_MINT & 1) SH2_Interrupt(&M_SH2, 6);
            if (_32X_SINT & 1) SH2_Interrupt(&S_SH2, 6);
        }
    }
}

/*void FASTCALL PWM_Update(int **buf, int length)
{
if (PWM_Enable && length)
{
int TempOutL = ((PWM_Out_L << 5) & 0xFFFF) - 0x4000;
int TempOutR = ((PWM_Out_R << 5) & 0xFFFF) - 0x4000;
TempOutL *= 3;
TempOutR *= 3;
TempOutL *= PWMVol;
TempOutR *= PWMVol;
TempOutL >>= 8;
TempOutR >>= 8;
while (length)
{
if (PWM_Out_L) buf[0][length] += TempOutL;
if (PWM_Out_R) buf[1][length] += TempOutR;
length--;
}
}
}*/

void ApplyPWMVol()
{
    // this gets the volume levels right relative to other channels like YM2612
    // it's kind of a weird way to do this, but it works well somehow

    if (PWM_Out_L_Tmp >= 0)
        PWM_Out_L_Tmp += PWM_Out_L_Tmp >> 1;
    else
        PWM_Out_L_Tmp += PWM_Out_L_Tmp << 1;

    if (PWM_Out_R_Tmp >= 0)
        PWM_Out_R_Tmp += PWM_Out_R_Tmp >> 1;
    else
        PWM_Out_R_Tmp += PWM_Out_R_Tmp << 1;

    // now apply the user volume setting also

    PWM_Out_L_Tmp = (PWM_Out_L_Tmp * PWMVol) >> 8;
    PWM_Out_R_Tmp = (PWM_Out_R_Tmp * PWMVol) >> 8;
}