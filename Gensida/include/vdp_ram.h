#ifndef VDP_RAM_H
#define VDP_RAM_H

void Redraw_VDP_View();

#define SwapPalColor(x) ((((x) >> 16) & 0xFF) | (((x) & 0xFF) << 16) | ((x) & 0xFF00))
#define GetPalColor(x) (COLORREF)SwapPalColor(Palette32[CRam[x]|0x4000])
#define GetPalColorNoSwap(x) (COLORREF)Palette32[CRam[x]|0x4000]

#endif
