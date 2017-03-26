#include "vdp_rend.h"

#ifndef _GUIDRAW_H
#define _GUIDRAW_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */
    void Pixel(short x, short y, unsigned int color32, unsigned short color16, char wrap = 1, unsigned char Opac = 0xFF);
    void PutText(char *string, short x, short y, short xl, short yl, short xh, short yh, int outstyle, int style);
    void PutText2(const char *string, short x, short y, int color = 0xFFFFFFFF, int outlineColor = 0x000000FF);
    void DrawLine(short x1, short y1, short x2, short y2, unsigned int color32, unsigned short color16, char wrap = 1, unsigned char Opac = 0xFF, char skipFirst = 0);
    void DrawBoxPP(short x1, short y1, short x2, short y2, unsigned int color32, unsigned short color16, char wrap = 1, unsigned char Opac = 0xFF, char fill = 0, unsigned char fillOpac = 0x7F);
    void DrawBoxPP2(short x1, short y1, short x2, short y2, unsigned int fillcolor32, unsigned int outlinecolor32);
    void DrawBoxMWH(short x, short y, short w, short h, unsigned int color32, unsigned short color16, char wrap = 1, unsigned char Opac = 0xFF, char fill = 0, unsigned char fillOpac = 0x7F);
    void DrawBoxCWH(short x, short y, short w, short h, unsigned int color32, unsigned short color16, char wrap = 1, unsigned char Opac = 0xFF, char fill = 0, unsigned char fillOpac = 0x7F);
    void DrawEccoOct(short x, short y, short r, unsigned int color32, unsigned short color16, char wrap = 1, unsigned char Opac = 0xFF, char fill = 0, unsigned char fillOpac = 0x7F);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */
#endif
