#ifndef _DRAWUTIL_H
#define _DRAWUTIL_H

#include "math.h"

typedef unsigned short pix16;
typedef unsigned int pix32;
typedef signed short pix15;

class DrawUtil
{
public:
    static inline pix32 Blend(pix32 A, pix32 B);
    static inline pix16 Blend(pix16 A, pix16 B);
    static inline pix32 Blend(pix32 A, pix32 B, int AWeight, int BWeight, int Shift);
    static inline pix16 Blend(pix16 A, pix16 B, int AWeight, int BWeight, int Shift);
    static inline pix32 Blend(pix32 To, pix32 From, int Opac); // 256 Opac means To is returned
    static inline pix16 Blend(pix16 To, pix16 From, int Opac); // 0 Opac means From is returned

    static inline pix32 Blend(pix32 A, pix32 B, pix32 C, pix32 D);
    static inline pix16 Blend(pix16 A, pix16 B, pix16 C, pix16 D);
    static inline pix32 Blend_3_1(pix32 A, pix32 B);
    static inline pix16 Blend_3_1(pix16 A, pix16 B);

    static inline pix32 Add(pix32 A, pix32 B);
    static inline pix16 Add(pix16 A, pix16 B);

    static inline pix32 Pix16To32(pix16 Src);
    static inline pix32 Pix15To32(pix15 Src);
    static inline pix16 Pix32To16(pix32 Src);
    static inline pix15 Pix32To15(pix32 Src);

    static inline pix16 Make16(int R, int G, int B);
    static inline pix32 Make32(int R, int G, int B);
    static inline int GetR(pix16 Src);
    static inline int GetR(pix32 Src);
    static inline int GetG(pix16 Src);
    static inline int GetG(pix32 Src);
    static inline int GetB(pix16 Src);
    static inline int GetB(pix32 Src);

    enum
    {
        RMASK_16 = 0xF800,
        GMASK_16 = 0x07E0,
        BMASK_16 = 0x001F,
        RBMASK_16 = (RMASK_16 | BMASK_16),
    };
    enum
    {
        RMASK_32 = 0x00FF0000,
        GMASK_32 = 0x0000FF00,
        BMASK_32 = 0x000000FF,
        RBMASK_32 = (RMASK_32 | BMASK_32),
    };
    enum
    {
        RMASK_15 = 0x7C00,
        GMASK_15 = 0x03E0,
        BMASK_15 = 0x001F,
        RBMASK_15 = (RMASK_15 | BMASK_15),
    };
};

// macro to apply some code to the R, G, and B components of a 16 or 32 bit pixel and return the resulting pixel value
// every reference to pixel x in <code> must be of the form (x & MASK)
// assumes the range (per component) of outputs has no bits that are more than 1 full component away from the closest bit in the largest valid input (e.g. does not shift the input more than 8 bits left or right if 32-bit)
#define DO_RGB(depth, code) \
	pix##depth rv; \
	{ enum{MASK = RBMASK_##depth}; rv = (code)&MASK; } \
	{ enum{MASK =  GMASK_##depth}; rv |= (code)&MASK; } \
	return rv;

#define DO_RGBA(depth, code, codea) \
	pix##depth rv; \
	{ enum{MASK = RBMASK_##depth}; rv = (code)&MASK; } \
	{ enum{MASK =  GMASK_##depth}; rv |= (code)&MASK; } \
	{ enum{MASK =     0xFF000000}; rv |= (codea)&MASK; } \
	return rv;

inline pix16 DrawUtil::Blend(pix16 A, pix16 B) {
    DO_RGB(16, (((A & MASK) + (B & MASK)) >> 1));
}
inline pix32 DrawUtil::Blend(pix32 A, pix32 B) {
    DO_RGBA(32, (((A & MASK) + (B & MASK)) >> 1), (((A & MASK) >> 1) + ((B & MASK) >> 1)));
}

inline pix16 DrawUtil::Blend(pix16 A, pix16 B, int AWeight, int BWeight, int Shift) {
    DO_RGB(16, (((A & MASK)*AWeight + (B & MASK)*BWeight) >> Shift));
}
inline pix32 DrawUtil::Blend(pix32 A, pix32 B, int AWeight, int BWeight, int Shift) {
    DO_RGBA(32, (((A & MASK)*AWeight + (B & MASK)*BWeight) >> Shift), (((A & MASK) >> Shift)*AWeight + ((B & MASK) >> Shift)*BWeight));
}

inline pix16 DrawUtil::Blend(pix16 To, pix16 From, int Opac) {
    DO_RGB(16, (((To & MASK) * Opac + (From & MASK) * (256 - Opac)) >> 8));
}
inline pix32 DrawUtil::Blend(pix32 To, pix32 From, int Opac) {
    DO_RGBA(32, (((To & MASK) * Opac + (From & MASK) * (256 - Opac)) >> 8), (((To & MASK) >> 8) * Opac + ((From & MASK) >> 8) * (256 - Opac)));
}

inline pix16 DrawUtil::Blend(pix16 A, pix16 B, pix16 C, pix16 D) {
    return Blend(Blend(A, B), Blend(C, D));
}
inline pix32 DrawUtil::Blend(pix32 A, pix32 B, pix32 C, pix32 D) {
    return Blend(Blend(A, B), Blend(C, D));
}

inline pix16 DrawUtil::Blend_3_1(pix16 A, pix16 B) {
    return Blend(A, B, 3, 1, 2);
}
inline pix32 DrawUtil::Blend_3_1(pix32 A, pix32 B) {
    return Blend(A, B, 3, 1, 2);
}

inline pix16 DrawUtil::Add(pix16 A, pix16 B) {
    DO_RGB(16, ((A & MASK) + (B & MASK)));
}
inline pix32 DrawUtil::Add(pix32 A, pix32 B) {
    DO_RGB(32, ((A & MASK) + (B & MASK)));
}

#undef DO_RGB

// from: xxxxxxxxRRRRRxxxGGGGGGxxBBBBBxxx
//   to:                 RRRRRGGGGGGBBBBB
// todo: round instead of truncate?
inline pix16 DrawUtil::Pix32To16(pix32 Src)
{
    int rm = Src & 0xF80000;
    int gm = Src & 0x00FC00;
    int bm = Src & 0x0000F8;
    return (rm >> 8) | (gm >> 5) | (bm >> 3);
}

// from:                 RRRRRGGGGGGBBBBB
//   to: 00000000RRRRR000GGGGGG00BBBBB000
inline pix32 DrawUtil::Pix16To32(pix16 Src)
{
    int rm = Src & 0xF800;
    int gm = Src & 0x07E0;
    int bm = Src & 0x001F;
    return (rm << 8) | (gm << 5) | (bm << 3);
}

// from:                 xRRRRRGGGGGBBBBB
//   to: 00000000RRRRR000GGGGG000BBBBB000
inline pix32 DrawUtil::Pix15To32(pix15 Src)
{
    int rm = Src & 0x7C00;
    int gm = Src & 0x03E0;
    int bm = Src & 0x001F;
    return (rm << 9) | (gm << 6) | (bm << 3);
}

// from: xxxxxxxxRRRRRxxxGGGGGxxxBBBBBxxx
//   to:                 xRRRRRGGGGGBBBBB
inline pix15 DrawUtil::Pix32To15(pix32 Src)
{
    int rm = Src & 0xF80000;
    int gm = Src & 0x00F800;
    int bm = Src & 0x0000F8;
    return (rm >> 9) | (gm >> 6) | (bm >> 3);
}

inline pix16 DrawUtil::Make16(int R, int G, int B)
{
    return ((R & 0x1F) << 11) | ((G & 0x3F) << 5) | ((B & 0x1F) << 0);
}

inline pix32 DrawUtil::Make32(int R, int G, int B)
{
    return ((R & 0xFF) << 16) | ((G & 0xFF) << 8) | ((B & 0xFF) << 0);
}

inline int DrawUtil::GetR(pix16 Src)
{
    return (Src >> 8) & 0xF8;
}
inline int DrawUtil::GetG(pix16 Src)
{
    return (Src >> 3) & 0xFC;
}
inline int DrawUtil::GetB(pix16 Src)
{
    return (Src << 3) & 0xF8;
}

inline int DrawUtil::GetR(pix32 Src)
{
    return (Src >> 16) & 0xFF;
}
inline int DrawUtil::GetG(pix32 Src)
{
    return (Src >> 8) & 0xFF;
}
inline int DrawUtil::GetB(pix32 Src)
{
    return (Src >> 0) & 0xFF;
}

#endif
