#include "gens.h"
#include "g_main.h"
#include "resource.h"
#include "plane_explorer_kmod.h"
#include "vdp_io.h"

#include <set>

void Update_Plane_Explorer()
{
    if (PlaneExplorerHWnd)
    {
        InvalidateRect(PlaneExplorerHWnd, NULL, FALSE);
    }
}

/*********** PLANE EXPLORER ******/

static unsigned char plane_explorer_data[128 * 8 * 128 * 8];
static COLORREF plane_explorer_palette[256];
static int plane_explorer_plane = 0;
static bool show_transparence = false;
static POINT tile_pt = { 0 };
static RECT sprite_rect = { 0 };

static void PlaneExplorerInit_KMod(HWND hDlg)
{
    HWND hexplorer;
    RECT rc, rc2;

    CheckDlgButton(hDlg, IDC_PLANEEXPLORER_TRANS, show_transparence ? BST_CHECKED : BST_UNCHECKED);
    CheckRadioButton(hDlg, IDC_PLANEEXPLORER_PLANE_A, IDC_PLANEEXPLORER_SPRITES, IDC_PLANEEXPLORER_PLANE_A + plane_explorer_plane);
    hexplorer = (HWND)GetDlgItem(hDlg, IDC_PLANEEXPLORER_MAIN);
    GetWindowRect(hexplorer, &rc);
    GetWindowRect(hDlg, &rc2);
    SetWindowPos(hexplorer, HWND_TOP, 0, 0, (rc2.right - rc.left) - 20, (rc2.bottom - rc.top) - 20, SWP_NOMOVE);
}

static void PlaneExplorer_UpdatePalette(void)
{
    COLORREF * cr = &plane_explorer_palette[0];
    COLORREF col;
    unsigned short * pal = (unsigned short *)(&CRam[0]);
    int i;
    static const COLORREF normal_pal[] =
    {
        0x00000000,
        0x00000011,
        0x00000022,
        0x00000033,
        0x00000044,
        0x00000055,
        0x00000066,
        0x00000077,
        0x00000088,
        0x00000099,
        0x000000AA,
        0x000000BB,
        0x000000CC,
        0x000000DD,
        0x000000EE,
        0x000000FF
    };

    for (i = 0; i < 64; i++)
    {
        unsigned short p = *pal++;
        col = normal_pal[(p >> 8) & 0xF] << 0;
        col |= normal_pal[(p >> 4) & 0xF] << 8;
        col |= normal_pal[(p >> 0) & 0xF] << 16;
        *cr++ = col;
    }

    plane_explorer_palette[253] = 0x00333333;
    plane_explorer_palette[254] = 0x00444444;
    plane_explorer_palette[255] = 0x00555555;
}

union PATTERN_NAME
{
    struct
    {
        unsigned short tile_index : 11;
        unsigned short h_flip : 1;
        unsigned short v_flip : 1;
        unsigned short pal_index : 2;
        unsigned short priority : 1;
    };
    unsigned short word;
};

static void PlaneExplorer_DrawTile(unsigned short name_word, unsigned int x, unsigned int y, int transcolor)
{
    union PATTERN_NAME name;
    int tile_height = ((VDP_Reg.Set4 & 0x6) == 6) ? 16 : 8;
    unsigned char * ptr = &plane_explorer_data[y * 1024 * tile_height + x * 8];
    unsigned int j, k;
    unsigned int * tile_data;
    int stride = 1024;
    unsigned char pal_index;

    name.word = name_word;
    tile_data = (unsigned int *)(&VRam[(name.tile_index * tile_height * 4) & 0xFFFF]);
    pal_index = (unsigned char)name.pal_index << 4;

    if (name.v_flip)
    {
        ptr += (tile_height - 1) * stride;
        stride = -stride;
    }

    if (name.h_flip)
    {
        for (j = 0; j < (unsigned int)tile_height; j++)
        {
            unsigned int tile_row = tile_data[j];
            ptr[4] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[5] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[6] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[7] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[0] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[1] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[2] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[3] = (tile_row & 0xF) | pal_index;
            if (transcolor != -1)
            {
                for (k = 0; k < 8; k++)
                {
                    if (ptr[k] == pal_index)
                    {
                        ptr[k] = transcolor;
                    }
                }
            }
            ptr += stride;
        }
    }
    else
    {
        for (j = 0; j < (unsigned int)tile_height; j++)
        {
            unsigned int tile_row = tile_data[j];
            ptr[3] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[2] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[1] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[0] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[7] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[6] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[5] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
            ptr[4] = (tile_row & 0xF) | pal_index;
            if (transcolor != -1)
            {
                for (k = 0; k < 8; k++)
                {
                    if (ptr[k] == pal_index)
                    {
                        ptr[k] = transcolor;
                    }
                }
            }
            ptr += stride;
        }
    }
}

struct sprite_info
{
    unsigned short ypos;
    unsigned char width;
    unsigned char height;
    unsigned char link;
    bool priority;
    unsigned char paletteLine;
    bool vflip;
    bool hflip;
    unsigned short blockNumber;
    unsigned short xpos;
};

inline static unsigned short PlaneExplorer_GetSpriteOffset(unsigned char currentSpriteNo)
{
    return (currentSpriteNo * 8);
}

static void PlaneExplorer_GetSpriteInfo(unsigned short *plane, unsigned char currentSpriteNo, sprite_info &sprite)
{
    // Read the mapping data for this sprite
    unsigned short offset = PlaneExplorer_GetSpriteOffset(currentSpriteNo) / 2;
    sprite.ypos = plane[offset + 0];
    sprite.width = (plane[offset + 1] >> 10) & 3;
    sprite.height = (plane[offset + 1] >> 8) & 3;
    sprite.link = plane[offset + 1] & 0x7F;
    sprite.priority = ((plane[offset + 2] >> 15) & 1) ? true : false;
    sprite.paletteLine = (plane[offset + 2] >> 13) & 3;
    sprite.vflip = ((plane[offset + 2] >> 12) & 1) ? true : false;
    sprite.hflip = ((plane[offset + 2] >> 11) & 1) ? true : false;
    sprite.blockNumber = plane[offset + 2] & 0x7FF;
    sprite.xpos = plane[offset + 3];
}

//----------------------------------------------------------------------------------------
static void CalculateEffectiveCellScrollSize(unsigned char& effectiveScrollWidth, unsigned char& effectiveScrollHeight,
    unsigned char &tileWidthInPixels, unsigned char &tileHeightInPixels, bool& h40ModeActive, bool& interlaceMode2Active)
{
    unsigned int hszState = VDP_Reg.Scr_Size & 3;
    unsigned int vszState = (VDP_Reg.Scr_Size >> 4) & 3;

    unsigned int screenSizeModeH = hszState;
    unsigned int screenSizeModeV = ((vszState & 0x1) & ((~hszState & 0x02) >> 1)) | ((vszState & 0x02) & ((~hszState & 0x01) << 1));
    effectiveScrollWidth = (screenSizeModeH + 1) * 32;
    effectiveScrollHeight = (screenSizeModeV + 1) * 32;
    if (screenSizeModeH == 2)
    {
        effectiveScrollWidth = 32;
        effectiveScrollHeight = 1;
    }
    else if (screenSizeModeV == 2)
    {
        effectiveScrollWidth = 32;
        effectiveScrollHeight = 32;
    }

    tileWidthInPixels = 8;
    tileHeightInPixels = ((VDP_Reg.Set4 & 0x6) == 6) ? 16 : 8;
    h40ModeActive = (VDP_Reg.Set4 & 1) ? true : false;
    interlaceMode2Active = ((VDP_Reg.Set4 & 6) == 6) ? true : false;

    switch (plane_explorer_plane)
    {
    case 2: // W
        effectiveScrollWidth = h40ModeActive ? 64 : 32;
        effectiveScrollHeight = 32;
        break;
    case 3: // S
        effectiveScrollWidth = 64;
        effectiveScrollHeight = interlaceMode2Active ? 128 : 64;
        break;
    }
}

static void PlaneExplorer_DrawSprite(sprite_info sprite, unsigned char tileWidthInPixels, unsigned char tileHeightInPixels, bool h40ModeActive, bool interlaceMode2Active)
{
    unsigned short spritePosScreenStartX = 0x80;
    unsigned short spritePosScreenStartY = (interlaceMode2Active) ? 0x100 : 0x80;
    unsigned short spritePosScreenEndX = (h40ModeActive) ? 0x1BF : 0x17F;
    unsigned short spritePosScreenEndY = (interlaceMode2Active) ? 0x2DF : 0x2BF;

    // Render this sprite to the buffer
    unsigned char spriteHeightInCells = sprite.height + 1;
    unsigned char spriteWidthInCells = sprite.width + 1;

    for (unsigned char j = 0; j < spriteHeightInCells * tileHeightInPixels; j++)
    {
        for (unsigned char i = 0; i < spriteWidthInCells * tileWidthInPixels; i++)
        {
            // If this sprite pixel lies outside the visible buffer region, skip it.
            if (
                ((sprite.xpos + i) < spritePosScreenStartX) || ((sprite.xpos + i) >= spritePosScreenEndX) ||
                ((sprite.ypos + j) < spritePosScreenStartY) || ((sprite.ypos + j) >= spritePosScreenEndY)
                )
            {
                continue;
            }

            // Calculate the target pixel row and column number within the sprite
            unsigned int pixelRowNo = (sprite.vflip) ? ((spriteHeightInCells * tileHeightInPixels) - 1) - j : j;
            unsigned int pixelColumnNo = (sprite.hflip) ? ((spriteWidthInCells * tileWidthInPixels) - 1) - i : i;

            // Calculate the row and column numbers for the target block within the
            // sprite, and the target pattern data within that block.
            unsigned int blockRowNo = pixelRowNo / tileHeightInPixels;
            unsigned int blockColumnNo = pixelColumnNo / tileWidthInPixels;
            unsigned int blockOffset = (blockColumnNo * spriteHeightInCells) + blockRowNo;
            unsigned int patternRowNo = pixelRowNo % tileHeightInPixels;
            unsigned int patternColumnNo = pixelColumnNo % tileWidthInPixels;

            // Calculate the VRAM address of the target pattern row data
            const unsigned int patternDataRowByteSize = 4;
            const unsigned int blockPatternByteSize = patternDataRowByteSize * tileHeightInPixels;
            unsigned int patternRowDataAddress = (((sprite.blockNumber + blockOffset) * blockPatternByteSize) + (patternRowNo * patternDataRowByteSize));

            // Read the pattern data byte for the target pixel in the target block
            const unsigned int pixelsPerPatternByte = 2;
            unsigned int patternByteNo = patternColumnNo / pixelsPerPatternByte;
            bool patternDataUpperHalf = (patternColumnNo % pixelsPerPatternByte) == 0;
            UINT8 patternData = VRam[((patternRowDataAddress + patternByteNo) % 0x10000) ^ 1];

            // Return the target palette row and index numbers
            unsigned char paletteIndex = (patternData >> (patternDataUpperHalf ? 4 : 0)) & 0xF;

            if (paletteIndex == 0)
            {
                continue;
            }

            int x = (sprite.xpos + i) - spritePosScreenStartX;
            int y = (sprite.ypos + j) - spritePosScreenStartY;
            plane_explorer_data[y * 1024 + x] = paletteIndex | (sprite.paletteLine << 4);
        }
    }
}

static void PlaneExplorer_UpdateBitmap()
{
    bool h40ModeActive, interlaceMode2Active;
    unsigned char plane_width, plane_height, tile_width, tile_height;
    CalculateEffectiveCellScrollSize(plane_width, plane_height, tile_width, tile_height, h40ModeActive, interlaceMode2Active);

    unsigned int base = 0;
    switch (plane_explorer_plane)
    {
    case 0: // A
        base = (VDP_Reg.Pat_ScrA_Adr & 0x38) << 10;
        break;
    case 1: // B
        base = (VDP_Reg.Pat_ScrB_Adr & 0x7) << 13;
        break;
    case 2: // W
        base = (VDP_Reg.Pat_Win_Adr & (h40ModeActive ? 0x3C : 0x3E)) << 10;
        break;
    case 3: // S
        base = (VDP_Reg.Spr_Att_Adr & (h40ModeActive ? 0x7E : 0x7F)) << 9;
        break;
    }

    for (int j = 0; j < 1024; j++)
    {
        for (int i = 0; i < 1024; i++)
        {
            plane_explorer_data[j * 1024 + i] = (unsigned char)(((j ^ i) >> 2) & 1) + 253;
        }
    }

    unsigned short *plane = (unsigned short *)(&VRam[base % 0x10000]);

    switch (plane_explorer_plane)
    {
    case 0: // A
    case 1: // B
    case 2: // W
    {
        for (unsigned int j = 0; j < plane_height; j++)
        {
            for (unsigned int i = 0; i < plane_width; i++)
            {
                int trans_color = show_transparence ? (unsigned char)(((j ^ i) >> 1) & 1) + 254 : -1;
                PlaneExplorer_DrawTile(plane[j * plane_width + i], i, j, trans_color);
            }
        }
    } break;
    case 3: // S (sprite rendering code from Exodus)
    {
        //Render each sprite to the sprite plane
        unsigned char maxSpriteCount = (h40ModeActive) ? 80 : 64;

        std::set<unsigned char> processedSprites;
        unsigned char currentSpriteNo = 0;
        do
        {
            sprite_info sprite;
            PlaneExplorer_GetSpriteInfo(plane, currentSpriteNo, sprite);

            //Advance to the next sprite in the list
            processedSprites.insert(currentSpriteNo);
            currentSpriteNo = sprite.link;
        } while ((currentSpriteNo > 0) && (currentSpriteNo < maxSpriteCount) && (processedSprites.find(currentSpriteNo) == processedSprites.end()));

        for (std::set<unsigned char>::const_reverse_iterator i = processedSprites.rbegin(); i != processedSprites.rend(); i++)
        {
            sprite_info sprite;
            PlaneExplorer_GetSpriteInfo(plane, *i, sprite);
            PlaneExplorer_DrawSprite(sprite, tile_width, tile_height, h40ModeActive, interlaceMode2Active);
        }
    } break;
    }
}

static void PlaneExplorerPaint_KMod(HWND hwnd, LPDRAWITEMSTRUCT lpdi)
{
    struct BMI_LOCAL
    {
        BITMAPINFOHEADER hdr;
        COLORREF         palette[256];
    };

    struct BMI_LOCAL bmi =
    {
        {
            sizeof(BITMAPINFOHEADER),
            128 * 8,
        -128 * 8,
        1,
        8,
        BI_RGB,
        0,
        250,
        250,
        256,
        256,
        },
        {
            0x00FF0055,
        }
    };

    PlaneExplorer_UpdatePalette();
    PlaneExplorer_UpdateBitmap();

    memcpy(bmi.palette, plane_explorer_palette, sizeof(bmi.palette));

    SetDIBitsToDevice(
        lpdi->hDC,
        lpdi->rcItem.left, lpdi->rcItem.top,
        lpdi->rcItem.right - lpdi->rcItem.left,
        lpdi->rcItem.bottom - lpdi->rcItem.top,
        lpdi->rcItem.left, lpdi->rcItem.top,
        lpdi->rcItem.top, lpdi->rcItem.bottom - lpdi->rcItem.top,
        plane_explorer_data,
        (const BITMAPINFO *)&bmi,
        DIB_RGB_COLORS);

    bool h40ModeActive, interlaceMode2Active;
    unsigned char plane_width, plane_height, tile_width, tile_height;
    CalculateEffectiveCellScrollSize(plane_width, plane_height, tile_width, tile_height, h40ModeActive, interlaceMode2Active);

    RECT r;

    if (plane_explorer_plane != 3)
    {
        r.left = (tile_pt.x & ~(tile_width - 1));
        r.top = (tile_pt.y & ~(tile_height - 1));
        r.right = r.left + tile_width;
        r.bottom = r.top + tile_height;
    }
    else
    {
        r = sprite_rect;
    }

    DrawFocusRect(lpdi->hDC, &r);
}

void PlaneExplorer_GetTipText(unsigned short x, unsigned short y, char * buffer)
{
    bool h40ModeActive, interlaceMode2Active;
    unsigned char plane_width, plane_height, tile_width, tile_height;
    CalculateEffectiveCellScrollSize(plane_width, plane_height, tile_width, tile_height, h40ModeActive, interlaceMode2Active);

    unsigned int base = 0;
    char plane_char = 'A';
    switch (plane_explorer_plane)
    {
    case 0:
        base = (VDP_Reg.Pat_ScrA_Adr & 0x38) << 10;
        plane_char = 'A';
        break;
    case 1:
        base = (VDP_Reg.Pat_ScrB_Adr & 0x7) << 13;
        plane_char = 'B';
        break;
    case 2:
        base = (VDP_Reg.Pat_Win_Adr & (h40ModeActive ? 0x3C : 0x3E)) << 10;
        plane_char = 'W';
        break;
    case 3:
        base = (VDP_Reg.Spr_Att_Adr & (h40ModeActive ? 0x7E : 0x7F)) << 9;
        plane_char = 'S';
        break;
    }

    if (x >= (plane_width * tile_width) ||
        y >= (plane_height * tile_height))
    {
        buffer[0] = 0;
        return;
    }

    switch (plane_explorer_plane)
    {
    case 0:
    case 1:
    case 2:
    {
        union PATTERN_NAME name;
        unsigned int tile_addr;

        tile_addr = base + ((y / tile_height) * plane_width + (x / tile_width)) * 2;
        name.word = *(unsigned short *)(&VRam[tile_addr]);

        sprintf(buffer,
            "X/Y: %dx%d;\n\n"
            "PLANE: %c (0x%04X);\n\n"
            "ADDRESS: 0x%04X;\n\n"
            "VALUE: 0x%04X;\n\n"
            "INDEX: 0x%04X(%d);\n\n"
            "PALETTE: %d;\n\n"
            "HFLIP: %s; VFLIP: %s;\n\n"
            "PRIORITY: %s",

            x & ~(tile_width - 1), y & ~(tile_height - 1),
            plane_char, base,
            tile_addr,
            name.word,
            name.tile_index, name.tile_index,
            name.pal_index,
            name.h_flip ? "YES" : "NO",
            name.v_flip ? "YES" : "NO",
            name.priority ? "YES" : "NO"
        );
    } break;
    case 3:
    {
        unsigned short *plane = (unsigned short *)(&VRam[base % 0x10000]);

        //Render each sprite to the sprite plane
        unsigned char maxSpriteCount = (h40ModeActive) ? 80 : 64;
        unsigned short spritePosScreenStartX = 0x80;
        unsigned short spritePosScreenStartY = (interlaceMode2Active) ? 0x100 : 0x80;
        unsigned short spritePosScreenEndX = (h40ModeActive) ? 0x1BF : 0x17F;
        unsigned short spritePosScreenEndY = (interlaceMode2Active) ? 0x2DF : 0x2BF;

        std::set<unsigned char> processedSprites;
        unsigned char currentSpriteNo = 0;
        do
        {
            sprite_info sprite;
            PlaneExplorer_GetSpriteInfo(plane, currentSpriteNo, sprite);

            unsigned short sprite_offset = PlaneExplorer_GetSpriteOffset(currentSpriteNo);
            unsigned char spriteHeightInCells = sprite.height + 1;
            unsigned char spriteWidthInCells = sprite.width + 1;

            int min_x = sprite.xpos - spritePosScreenStartX;
            int min_y = sprite.ypos - spritePosScreenStartY;
            int max_x = (sprite.xpos + spriteWidthInCells * tile_width) - spritePosScreenStartX;
            int max_y = (sprite.ypos + spriteHeightInCells * tile_height) - spritePosScreenStartY;

            if (
                (min_x < 0) && (max_x >= plane_width * tile_width) &&
                (min_y < 0) && (max_y >= plane_height * tile_height)
                )
            {
                processedSprites.insert(currentSpriteNo);
                currentSpriteNo = sprite.link;
                continue;
            }

            if (
                (x >= min_x) && (x < max_x) &&
                (y >= min_y) && (y < max_y)
                )
            {
                sprintf(buffer,
                    "INDEX: %d;\n\n"
                    "SCREEN X/Y: %dx%d;\n\n"
                    "PLANE: %c (0x%04X);\n\n"
                    "ADDRESS: 0x%04X;\n\n"

                    "SPRITE X/Y: %dx%d;\n\n"
                    "SIZE W/H: %dx%d;\n\n"
                    "TILE ID: 0x%04X(%d)\n\n"
                    "LINK: %d;\n\n"
                    "PALETTE: %d;\n\n"
                    "HFLIP: %s; VFLIP: %s;\n\n"
                    "PRIORITY: %s",

                    currentSpriteNo,
                    x, y,
                    plane_char, base,
                    base + sprite_offset,

                    sprite.xpos, sprite.ypos,
                    (sprite.width + 1) * tile_width, (sprite.height + 1) * tile_height,
                    sprite.blockNumber, sprite.blockNumber,
                    sprite.link,
                    sprite.paletteLine,
                    sprite.hflip ? "YES" : "NO",
                    sprite.vflip ? "YES" : "NO",
                    sprite.priority ? "YES" : "NO"
                );

                sprite_rect.left = (sprite.xpos - spritePosScreenStartX);
                sprite_rect.top = (sprite.ypos - spritePosScreenStartY);
                sprite_rect.right = sprite_rect.left + (sprite.width + 1) * tile_width;
                sprite_rect.bottom = sprite_rect.top + (sprite.height + 1) * tile_height;
                break;
            }

            //Advance to the next sprite in the list
            processedSprites.insert(currentSpriteNo);
            currentSpriteNo = sprite.link;
        } while ((currentSpriteNo > 0) && (currentSpriteNo < maxSpriteCount) && (processedSprites.find(currentSpriteNo) == processedSprites.end()));
    } break;
    }
}

BOOL CALLBACK PlaneExplorerDialogProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
    case WM_INITDIALOG:
        PlaneExplorerInit_KMod(hwnd);
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_DRAWITEM:
        PlaneExplorerPaint_KMod(hwnd, (LPDRAWITEMSTRUCT)lParam);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_PLANEEXPLORER_PLANE_A:
        case IDC_PLANEEXPLORER_PLANE_B:
        case IDC_PLANEEXPLORER_WINDOW:
        case IDC_PLANEEXPLORER_SPRITES:
            plane_explorer_plane = LOWORD(wParam) - IDC_PLANEEXPLORER_PLANE_A;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case IDC_PLANEEXPLORER_TRANS:
            show_transparence = (IsDlgButtonChecked(hwnd, IDC_PLANEEXPLORER_TRANS) == BST_CHECKED);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        default:
            break;
        }
        break;

    case WM_SIZE:
    {
        RECT rc, rc2;
        HWND hexplorer = GetDlgItem(hwnd, IDC_PLANEEXPLORER_MAIN);
        GetWindowRect(hexplorer, &rc);
        GetWindowRect(hwnd, &rc2);
        SetWindowPos(hexplorer, HWND_TOP, 0, 0, (rc2.right - rc.left) - 20, (rc2.bottom - rc.top) - 20, SWP_NOMOVE);
        break;
    }
    case WM_CLOSE:
        DialogsOpen--;
        PlaneExplorerHWnd = NULL;
        EndDialog(hwnd, true);
        break;

    case WM_MOUSELEAVE:
        SetDlgItemText(hwnd, IDC_PLANEEXPLORER_TILEINFO, "");
        return FALSE;

    case WM_MOUSEMOVE:
    {
        HWND hexplorer = GetDlgItem(hwnd, IDC_PLANEEXPLORER_MAIN);
        int x = (short)(lParam);
        int y = (short)(lParam >> 16);
        char buffer[256] = "";
        RECT rc1;
        POINT _pt;
        TRACKMOUSEEVENT tme = { sizeof(tme) };
        tme.hwndTrack = hwnd;
        tme.dwFlags = TME_LEAVE;
        TrackMouseEvent(&tme);
        _pt.x = x;
        _pt.y = y;
        ClientToScreen(hwnd, &_pt);
        ScreenToClient(hexplorer, &_pt);
        GetClientRect(hexplorer, &rc1);
        if (PtInRect(&rc1, _pt))
        {
            PlaneExplorer_GetTipText((unsigned short)(_pt.x), (unsigned short)(_pt.y), &buffer[0]);
            tile_pt = _pt;

            bool h40ModeActive, interlaceMode2Active;
            unsigned char plane_width, plane_height, tile_width, tile_height;
            CalculateEffectiveCellScrollSize(plane_width, plane_height, tile_width, tile_height, h40ModeActive, interlaceMode2Active);

            RECT r;
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hexplorer, &ps);

            if (plane_explorer_plane != 3)
            {
                r.left = (_pt.x & ~(tile_width - 1));
                r.top = (_pt.y & ~(tile_height - 1));
                r.right = r.left + tile_width;
                r.bottom = r.top + tile_height;
            }
            else
            {
                r = sprite_rect;
            }

            DrawFocusRect(dc, &r);
            EndPaint(hexplorer, &ps);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        SetDlgItemText(hwnd, IDC_PLANEEXPLORER_TILEINFO, buffer);
        return FALSE;
    }

    default:
        return FALSE;
    }

    return TRUE;
}