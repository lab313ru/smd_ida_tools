/////////////////////////////////////////////////////////////////////////////////////////////
// SOUND.H
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef SOUND_H
#define SOUND_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

#define DIRECTSOUND_VERSION         0x0800

#include <dsound.h>

    extern int Sound_Rate;
    extern int Sound_Segs;
    extern int Sound_Enable;
    extern int Sound_Stereo;
    extern int Sound_Soften; // Modif N.
    extern int Sound_Is_Playing;
    extern int Sound_Initialised;
    extern int Seg_L[882], Seg_R[882];
    extern int Seg_Length;
    extern int WP, RP;

    extern unsigned int Sound_Interpol[882];
    extern unsigned int Sound_Extrapol[312][2];

    extern char Dump_Dir[1024];

    extern unsigned short MastVol;

    int Init_Sound(HWND hWnd);
    void End_Sound(void);
    int Get_Current_Seg(void);
    int Check_Sound_Timing(void);
    int Write_Sound_Buffer(void *Dump_Buf);
    int Clear_Sound_Buffer(void);
    int Fade_Sound_Buffer(void); // Modif N.
    int Play_Sound(void);
    int Stop_Sound(void);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif
