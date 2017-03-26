#ifndef MISC_H
#define MISC_H

#ifdef __cplusplus
extern "C" {
#endif

#define EMU_MODE 0x01
#define WHITE    0x00
#define BLUE     0x02
#define GREEN    0x04
#define RED      0x06
#define TRANS    0x08
#define SIZE_X2  0x10

    extern int Have_MMX;
    extern int MMX_Enable;

    void Identify_CPU(void);
    int Half_Blur(void);
    void Byte_Swap(void *Ptr, int NumByte);
    void Word_Swap(void *Ptr, int NumByte);
    void Print_Text(char *str, int Size, int Pos_X, int Pos_Y, int Style);
    //void Print_Text32(char *str, int Size, int Pos_X, int Pos_Y, int Style);

    void Cell_8x8_Dump(unsigned char *Adr, int Palette);
    void Cell_16x16_Dump(unsigned char *Adr, int Palette);
    void Cell_32x32_Dump(unsigned char *Adr, int Palette);

    void CDD_Export_Status(void);

    void Write_Sound_Mono_MMX(int *Left, int *Right, short *Dest, int length);
    void Write_Sound_Stereo_MMX(int *Left, int *Right, short *Dest, int length);

#ifdef __cplusplus
};
#endif

#endif
