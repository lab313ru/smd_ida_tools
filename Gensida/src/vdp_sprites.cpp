#include <windows.h>
#include <commctrl.h>

#include "vdp_ram.h"
#include "vdp_rend.h"
#include "vdp_io.h"
#include "resource.h"
#include "g_main.h"
#include "rom.h"
#include "g_ddraw.h"

/******************** SPRITES ***************/

void DrawTile_KMod(HDC hDCMain, unsigned short int numTile, WORD xpos, WORD ypos, UCHAR pal, UCHAR zoom)
{
    unsigned char y, x;
    unsigned char TileData;
    HDC hDC;
    HBITMAP hBitmap, hOldBitmap;

    BYTE* pdst;
    BITMAPINFOHEADER bmih = { sizeof(BITMAPINFOHEADER), 8, -8, 1, 32 };

    hDC = CreateCompatibleDC(hDCMain);
    hBitmap = CreateDIBSection(hDC, (BITMAPINFO *)&bmih, DIB_RGB_COLORS, (void **)&pdst, NULL, NULL);
    hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);

    for (y = 0; y < 8; y++)
    {
        for (x = 0; x < 4; x++)
        {
            int bytes = 4;
            int xMul = bytes;
            int _x = x * 2 + 0;
            int xOff = _x * xMul;
            int yMul = (bytes * 8);
            int yOff = y * yMul;

            TileData = VRam[numTile * 32 + y * 4 + (x ^ 1)];
            COLORREF c1 = GetPalColorNoSwap(16 * pal + (TileData >> 4));
            COLORREF c2 = GetPalColorNoSwap(16 * pal + (TileData & 0xF));

            *(COLORREF*)(&pdst[yOff + xOff]) = c1;
            *(COLORREF*)(&pdst[yOff + xOff + bytes]) = c2;
        }
    }

    StretchDIBits(
        hDCMain,
        xpos,
        ypos,
        8 * zoom,
        8 * zoom,
        0,
        0,
        8,
        8,
        pdst,
        (const BITMAPINFO *)&bmih,
        DIB_RGB_COLORS,
        SRCCOPY
    );

    SelectObject(hDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hDC);
}

unsigned char TrueSize_KMod(unsigned short int data)
{
    if (data < 4)
        return (data + 1) * 8;
    return 0;
}

char debug_string[2024];

void UpdateSprites_KMod()
{
    unsigned int i, topIdx, selIdx;
    unsigned short int *sprData, tmp;
    char tmp_string[32];

    if (!VDPSpritesHWnd)	return;

    SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);

    topIdx = SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, LB_GETTOPINDEX, (WPARAM)0, (LPARAM)0);
    selIdx = SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);

    sprData = (unsigned short *)(VRam + (VDP_Reg.Spr_Att_Adr << 9));

    SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    for (i = 0; i < 80; i++)
    {
        wsprintf(debug_string, "%02d", i);
        wsprintf(tmp_string, "  %4d", sprData[0] & 0x03FF);
        lstrcat(debug_string, tmp_string);
        wsprintf(tmp_string, "  %4d", sprData[3] & 0x03FF);
        lstrcat(debug_string, tmp_string);
        wsprintf(tmp_string, "  %.2dx", TrueSize_KMod(((sprData[1] & 0x0C00) >> 10)));
        lstrcat(debug_string, tmp_string);
        wsprintf(tmp_string, "%.2d", TrueSize_KMod(((sprData[1] & 0x0300) >> 8)));
        lstrcat(debug_string, tmp_string);
        wsprintf(tmp_string, "  %02d", sprData[1] & 0x007F);
        lstrcat(debug_string, tmp_string);
        wsprintf(tmp_string, " %2d", (sprData[2] & 0x6000) >> 13);
        lstrcat(debug_string, tmp_string);
        wsprintf(tmp_string, "  %03X", (sprData[2] & 0x07FF));
        lstrcat(debug_string, tmp_string);
        tmp = 0;
        if (sprData[2] & 0x8000)	tmp |= 4;
        if (sprData[2] & 0x1000)	tmp |= 2;
        if (sprData[2] & 0x0800)	tmp |= 1;
        wsprintf(tmp_string, "  %c%c%c", tmp & 4 ? 'P' : 'p', tmp & 2 ? 'V' : 'v', tmp & 1 ? 'H' : 'h');
        lstrcat(debug_string, tmp_string);
        SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, LB_INSERTSTRING, (WPARAM)i, (LPARAM)debug_string);

        sprData += 4;
    }

    SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, LB_SETCURSEL, (WPARAM)selIdx, (LPARAM)0);
    SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, LB_SETTOPINDEX, (WPARAM)topIdx, (LPARAM)0);

    SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0);

    //    RedrawWindow(VDPSpritesHWnd, NULL, NULL, RDW_INVALIDATE);
    RedrawWindow(GetDlgItem(VDPSpritesHWnd, IDC_SPRITES_PREVIEW), NULL, NULL, RDW_INVALIDATE);
    RedrawWindow(GetDlgItem(VDPSpritesHWnd, IDC_SPRITES_PREVIEW2), NULL, NULL, RDW_INVALIDATE);
}

void DrawSprite_KMod(LPDRAWITEMSTRUCT hlDIS)
{
    unsigned int selIdx;
    unsigned short int *sprData, pal;
    WORD posX, posY;
    unsigned char sizeH, sizeV;
    unsigned short int numTile = 0;
    HDC hDC;
    HBITMAP hBitmap, hOldBitmap;

    selIdx = SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    if (selIdx == LB_ERR)
        return;

    sprData = (unsigned short *)(VRam + (VDP_Reg.Spr_Att_Adr << 9));
    sprData += selIdx * 4; /* each sprite is 4 short int data */

    numTile = sprData[2] & 0x07FF;
    sizeH = TrueSize_KMod(((sprData[1] & 0x0C00) >> 10));
    sizeV = TrueSize_KMod(((sprData[1] & 0x0300) >> 8));

    hDC = CreateCompatibleDC(hlDIS->hDC);
    hBitmap = CreateCompatibleBitmap(hlDIS->hDC, 32, 32);
    hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);
    FillRect(hDC, &(hlDIS->rcItem), (HBRUSH)(COLOR_3DFACE + 1));

    pal = (sprData[2] & 0x6000) >> 13;
    for (posX = 0; posX < sizeH; posX += 8)
    {
        for (posY = 0; posY < sizeV; posY += 8)
        {
            DrawTile_KMod(hDC, numTile++, posX, posY, (UCHAR)pal, 1);
        }
    }

    BitBlt(
        hlDIS->hDC, // handle to destination device context
        0,  // x-coordinate of destination rectangle's upper-left
        // corner
        0,  // y-coordinate of destination rectangle's upper-left
        // corner
        32,  // width of destination rectangle
        32, // height of destination rectangle
        hDC,  // handle to source device context
        0,   // x-coordinate of source rectangle's upper-left
        // corner
        0,   // y-coordinate of source rectangle's upper-left
        // corner
        SRCCOPY  // raster operation code
    );

    SelectObject(hDC, hOldBitmap);
    DeleteObject(hBitmap);

    DeleteDC(hDC);
}

void DrawSpriteZoom_KMod(LPDRAWITEMSTRUCT hlDIS)
{
    unsigned int selIdx;
    unsigned short int *sprData, pal;
    WORD posX, posY;
    unsigned char sizeH, sizeV, zoom;
    unsigned short int numTile = 0;
    HDC hDC;
    HBITMAP hBitmap, hOldBitmap;

    selIdx = SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    if (selIdx == LB_ERR)
        return;

    sprData = (unsigned short*)(VRam + (VDP_Reg.Spr_Att_Adr << 9));
    sprData += selIdx * 4; /* each sprite is 4 short int data */

    numTile = sprData[2] & 0x07FF;
    sizeH = TrueSize_KMod(((sprData[1] & 0x0C00) >> 10));
    sizeV = TrueSize_KMod(((sprData[1] & 0x0300) >> 8));

    zoom = (unsigned char)((hlDIS->rcItem.right - hlDIS->rcItem.left) / 32);
    zoom = (unsigned char)min(zoom, (hlDIS->rcItem.bottom - hlDIS->rcItem.top) / 32);

    hDC = CreateCompatibleDC(hlDIS->hDC);
    hBitmap = CreateCompatibleBitmap(hlDIS->hDC, 32 * zoom, 32 * zoom);
    hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);
    FillRect(hDC, &(hlDIS->rcItem), (HBRUSH)(COLOR_3DFACE + 1));

    pal = (sprData[2] & 0x6000) >> 13;
    for (posX = 0; posX < sizeH; posX += 8)
    {
        for (posY = 0; posY < sizeV; posY += 8)
        {
            DrawTile_KMod(hDC, numTile++, posX*zoom, posY*zoom, (UCHAR)pal, zoom);
        }
    }

    BitBlt(
        hlDIS->hDC, // handle to destination device context
        0,  // x-coordinate of destination rectangle's upper-left
        // corner
        0,  // y-coordinate of destination rectangle's upper-left
        // corner
        32 * zoom,  // width of destination rectangle
        32 * zoom, // height of destination rectangle
        hDC,  // handle to source device context
        0,   // x-coordinate of source rectangle's upper-left
        // corner
        0,   // y-coordinate of source rectangle's upper-left
        // corner
        SRCCOPY  // raster operation code
    );

    SelectObject(hDC, hOldBitmap);
    DeleteObject(hBitmap);

    DeleteDC(hDC);
}

void DumpSprite_KMod(HWND hwnd)
{
    BITMAPFILEHEADER	bmfh;
    BITMAPINFOHEADER	bmiHeader;
    RGBQUAD				bmiColors[16];
    LPBYTE				pBits;

    OPENFILENAME		szFile;
    char				szFileName[MAX_PATH];
    HANDLE				hFr;
    DWORD				dwBytesToWrite, dwBytesWritten;

    COLORREF			tmpColor;
    unsigned short int	numTile, selIdx;
    unsigned char		TileData, sizeH, sizeV, posX, posY, j, pal;
    int tmp;
    unsigned short int	*sprData;

    selIdx = (unsigned short)SendDlgItemMessage(VDPSpritesHWnd, IDC_SPRITES_LIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    if (selIdx == LB_ERR)
        return;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

    strcpy(szFileName, Rom_Name);
    strcat(szFileName, "_Spr");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "16Colors Bitmap file (*.bmp)\0*.bmp\0\0";
    szFile.lpstrFile = szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump sprite";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
        OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "bmp";

    if (GetSaveFileName(&szFile) != TRUE)   return;

    sprData = (unsigned short*)(VRam + (VDP_Reg.Spr_Att_Adr << 9));
    sprData += selIdx * 4; /* each sprite is 4 short int data */

    numTile = sprData[2] & 0x07FF;
    sizeH = TrueSize_KMod(((sprData[1] & 0x0C00) >> 10));
    sizeV = TrueSize_KMod(((sprData[1] & 0x0300) >> 8));
    pal = (sprData[2] & 0x6000) >> 13;

    bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmiHeader.biWidth = sizeH;
    bmiHeader.biHeight = sizeV;
    bmiHeader.biPlanes = 0;
    bmiHeader.biBitCount = 4;
    bmiHeader.biClrUsed = 16;
    bmiHeader.biCompression = BI_RGB;
    bmiHeader.biSizeImage = sizeH*sizeV / 2;
    bmiHeader.biClrImportant = 0;

    bmfh.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
    bmfh.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) + bmiHeader.biSizeImage);
    bmfh.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD));

    for (j = 0; j < 16; j++)
    {
        tmpColor = GetPalColor(pal * 16 + j);
        bmiColors[j].rgbRed = (BYTE)tmpColor & 0xFF;
        tmpColor >>= 8;
        bmiColors[j].rgbGreen = (BYTE)tmpColor & 0xFF;
        tmpColor >>= 8;
        bmiColors[j].rgbBlue = (BYTE)tmpColor & 0xFF;
    }

    pBits = (LPBYTE)LocalAlloc(LPTR, bmiHeader.biSizeImage);
    if (pBits == NULL)
    {
        return;
    }

    for (posX = 0; posX < sizeH; posX += 8)
    {
        for (posY = 0; posY < sizeV; posY += 8)
        {
            for (j = 0; j < 8; j++)
            {
                TileData = VRam[numTile * 32 + j * 4 + 1];
                tmp = posX / 2 + int((sizeV - 1) - (posY + j))*int(sizeH) / 2;
                pBits[tmp] = TileData;

                TileData = VRam[numTile * 32 + j * 4];
                tmp = posX / 2 + int((sizeV - 1) - (posY + j))*int(sizeH) / 2 + 1;
                pBits[tmp] = TileData;

                TileData = VRam[numTile * 32 + j * 4 + 3];
                tmp = posX / 2 + int((sizeV - 1) - (posY + j))*int(sizeH) / 2 + 2;
                pBits[tmp] = TileData;

                TileData = VRam[numTile * 32 + j * 4 + 2];
                tmp = posX / 2 + int((sizeV - 1) - (posY + j))*int(sizeH) / 2 + 3;
                pBits[tmp] = TileData;
            }
            numTile++;
        }
    }

    hFr = CreateFile(szFileName, GENERIC_WRITE, (DWORD)0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
    if (hFr == INVALID_HANDLE_VALUE)
    {
        LocalFree((HLOCAL)pBits);
        return;
    }

    dwBytesToWrite = sizeof(BITMAPFILEHEADER);
    WriteFile(hFr, &bmfh, dwBytesToWrite, &dwBytesWritten, NULL);

    dwBytesToWrite = sizeof(BITMAPINFOHEADER);
    WriteFile(hFr, &bmiHeader, dwBytesToWrite, &dwBytesWritten, NULL);

    dwBytesToWrite = 16 * sizeof(RGBQUAD);
    WriteFile(hFr, bmiColors, dwBytesToWrite, &dwBytesWritten, NULL);

    dwBytesToWrite = bmiHeader.biSizeImage;
    WriteFile(hFr, pBits, dwBytesToWrite, &dwBytesWritten, NULL);

    CloseHandle(hFr);

    LocalFree((HLOCAL)pBits);

    Put_Info("Sprite dumped", 1500);
}

LRESULT CALLBACK VDPSpritesProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    HFONT hFont = NULL;

    switch (Message)
    {
    case WM_INITDIALOG:
        hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
        SendDlgItemMessage(hwnd, IDC_SPRITES_LIST, WM_SETFONT, (WPARAM)hFont, TRUE);
        VDPSpritesHWnd = hwnd;
        UpdateSprites_KMod();

        break;
    case WM_DRAWITEM:
        if ((UINT)wParam == IDC_SPRITES_PREVIEW)
            DrawSprite_KMod((LPDRAWITEMSTRUCT)lParam);
        else if ((UINT)wParam == IDC_SPRITES_PREVIEW2)
            DrawSpriteZoom_KMod((LPDRAWITEMSTRUCT)lParam);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_SPRITES_LIST:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
                RedrawWindow(GetDlgItem(VDPSpritesHWnd, IDC_SPRITES_PREVIEW), NULL, NULL, RDW_INVALIDATE);
                RedrawWindow(GetDlgItem(VDPSpritesHWnd, IDC_SPRITES_PREVIEW2), NULL, NULL, RDW_INVALIDATE);
                break;
            }
            break;
        case IDC_SPRITES_DUMP:
            DumpSprite_KMod(VDPSpritesHWnd);
            break;
        }
        break;
    case WM_CLOSE:
        EndDialog(hwnd, 0);
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        DeleteObject((HGDIOBJ)hFont);
        //DestroyWindow( VDPSpritesHWnd );
        VDPSpritesHWnd = NULL;
        --DialogsOpen;
        //PostQuitMessage(0);
        break;

    default:
        return FALSE;
    }
    return TRUE;
}