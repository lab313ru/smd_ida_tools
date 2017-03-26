#include "resource.h"
#include "gens.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "mem_sh2.h"
#include "misc.h"
#include "mem_z80.h"
#include "vdp_io.h"
#include "save.h"
#include "ram_search.h"
#include "ramwatch.h"
#include "g_main.h"
#include <assert.h>
#include <windows.h>
#include <commctrl.h>
#include <string>

static HMENU ramwatchmenu;
static HMENU rwrecentmenu;
static HACCEL RamWatchAccels = NULL;
char rw_recent_files[MAX_RECENT_WATCHES][1024];
char Watch_Dir[1024] = "";
const unsigned int RW_MENU_FIRST_RECENT_FILE = 600;
bool RWfileChanged = false; //Keeps track of whether the current watch file has been changed, if so, ramwatch will prompt to save changes
bool AutoRWLoad = false;    //Keeps track of whether Auto-load is checked
bool RWSaveWindowPos = false; //Keeps track of whether Save Window position is checked
char currentWatch[1024];
int ramw_x, ramw_y;			//Used to store ramwatch dialog window positions
AddressWatcher rswatches[MAX_WATCH_COUNT];
int WatchCount = 0;

#define MESSAGEBOXPARENT (RamWatchHWnd ? RamWatchHWnd : HWnd)

bool QuickSaveWatches();
bool ResetWatches();
extern "C" int Clear_Sound_Buffer(void);

void RefreshWatchListSelectedCountControlStatus(HWND hDlg);

unsigned int GetCurrentValue(AddressWatcher& watch)
{
    return ReadValueAtHardwareAddress(watch.Address, watch.Size == 'd' ? 4 : watch.Size == 'w' ? 2 : 1);
}

bool IsSameWatch(const AddressWatcher& l, const AddressWatcher& r)
{
    if (r.Size == 'S') return false;
    return ((l.Address == r.Address) && (l.Size == r.Size) && (l.Type == r.Type)/* && (l.WrongEndian == r.WrongEndian)*/);
}

bool VerifyWatchNotAlreadyAdded(const AddressWatcher& watch)
{
    for (int j = 0; j < WatchCount; j++)
    {
        if (IsSameWatch(rswatches[j], watch))
        {
            if (RamWatchHWnd)
                SetForegroundWindow(RamWatchHWnd);
            return false;
        }
    }
    return true;
}

/*
int i: either the number of existing watches minus 1, or the index of the watch to edit.
*/
bool InsertWatch(const AddressWatcher& Watch, char *Comment, int i)
{
    if (i == WatchCount) // append new watch
    {
        if (!VerifyWatchNotAlreadyAdded(Watch))
            return false;
        if (WatchCount >= MAX_WATCH_COUNT)
            return false;

        WatchCount++;
    }
    else // replace existing watch
    {
        // TODO: minor bug: we have not checked that they haven't changed the address to an already-existing watch.
        //       We can't just use VerifyWatchNotAlreadyAdded because doing so doesn't let us keep the same address but change the comment.
        free(rswatches[i].comment);
    }
    AddressWatcher& NewWatch = rswatches[i];
    NewWatch = Watch;
    NewWatch.comment = (char *)malloc(strlen(Comment) + 2);
    NewWatch.CurValue = GetCurrentValue(NewWatch);
    strcpy(NewWatch.comment, Comment);
    ListView_SetItemCount(GetDlgItem(RamWatchHWnd, IDC_WATCHLIST), WatchCount);
    RWfileChanged = true;

    return true;
}

LRESULT CALLBACK PromptWatchNameProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) //Gets the description of a watched address
{
    RECT r;
    RECT r2;
    int dx1, dy1, dx2, dy2;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        Clear_Sound_Buffer();

        if (Full_Screen)
        {
            while (ShowCursor(false) >= 0);
            while (ShowCursor(true) < 0);
        }

        GetWindowRect(HWnd, &r);
        dx1 = (r.right - r.left) / 2;
        dy1 = (r.bottom - r.top) / 2;

        GetWindowRect(hDlg, &r2);
        dx2 = (r2.right - r2.left) / 2;
        dy2 = (r2.bottom - r2.top) / 2;

        //SetWindowPos(hDlg, NULL, max(0, r.left + (dx1 - dx2)), max(0, r.top + (dy1 - dy2)), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
        SetWindowPos(hDlg, NULL, r.left, r.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
        strcpy(Str_Tmp, "Enter a name for this RAM address.");
        SendDlgItemMessage(hDlg, IDC_PROMPT_TEXT, WM_SETTEXT, 0, (LPARAM)Str_Tmp);
        strcpy(Str_Tmp, "");
        SendDlgItemMessage(hDlg, IDC_PROMPT_TEXT2, WM_SETTEXT, 0, (LPARAM)Str_Tmp);
        return true;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            if (Full_Screen)
            {
                while (ShowCursor(true) < 0);
                while (ShowCursor(false) >= 0);
            }
            GetDlgItemText(hDlg, IDC_PROMPT_EDIT, Str_Tmp, 80);
            InsertWatch(rswatches[WatchCount], Str_Tmp, WatchCount);
            DialogsOpen--;
            EndDialog(hDlg, true);
            return true;
            break;
        }
        case ID_CANCEL:
        case IDCANCEL:
            if (Full_Screen)
            {
                while (ShowCursor(true) < 0);
                while (ShowCursor(false) >= 0);
            }

            DialogsOpen--;
            EndDialog(hDlg, false);
            return false;
            break;
        }
        break;

    case WM_CLOSE:
        if (Full_Screen)
        {
            while (ShowCursor(true) < 0);
            while (ShowCursor(false) >= 0);
        }
        DialogsOpen--;
        EndDialog(hDlg, false);
        return false;
        break;
    }

    return false;
}

bool InsertWatch(const AddressWatcher& Watch, HWND parent)
{
    if (!VerifyWatchNotAlreadyAdded(Watch))
        return false;

    if (!parent)
        parent = RamWatchHWnd;
    if (!parent)
        parent = HWnd;

    int prevWatchCount = WatchCount;

    rswatches[WatchCount] = Watch;
    rswatches[WatchCount].CurValue = GetCurrentValue(rswatches[WatchCount]);
    DialogBox(ghInstance, MAKEINTRESOURCE(IDD_PROMPT), parent, (DLGPROC)PromptWatchNameProc);

    return WatchCount > prevWatchCount;
}

void Update_RAM_Watch()
{
    // update cached values and detect changes to displayed listview items
    BOOL watchChanged[MAX_WATCH_COUNT] = { 0 };
    for (int i = 0; i < WatchCount; i++)
    {
        unsigned int prevCurValue = rswatches[i].CurValue;
        unsigned int newCurValue = GetCurrentValue(rswatches[i]);
        if (prevCurValue != newCurValue)
        {
            rswatches[i].CurValue = newCurValue;
            watchChanged[i] = TRUE;
        }
    }

    // refresh any visible parts of the listview box that changed
    HWND lv = GetDlgItem(RamWatchHWnd, IDC_WATCHLIST);
    int top = ListView_GetTopIndex(lv);
    int bottom = top + ListView_GetCountPerPage(lv) + 1; // +1 is so we will update a partially-displayed last item
    if (top < 0) top = 0;
    if (bottom > WatchCount) bottom = WatchCount;
    int start = -1;
    for (int i = top; i <= bottom; i++)
    {
        if (start == -1)
        {
            if (i != bottom && watchChanged[i])
            {
                start = i;
                //somethingChanged = true;
            }
        }
        else
        {
            if (i == bottom || !watchChanged[i])
            {
                ListView_RedrawItems(lv, start, i - 1);
                start = -1;
            }
        }
    }
}

bool AskSave()
{
    //This function asks to save changes if the watch file contents have changed
    //returns false only if a save was attempted but failed or was cancelled
    if (RWfileChanged)
    {
        int answer = MessageBox(MESSAGEBOXPARENT, "Save Changes?", "Ram Watch", MB_YESNOCANCEL);
        if (answer == IDYES)
            if (!QuickSaveWatches())
                return false;
        return (answer != IDCANCEL);
    }
    return true;
}

void UpdateRW_RMenu(HMENU menu, unsigned int mitem, unsigned int baseid)
{
    MENUITEMINFO moo;
    int x;

    moo.cbSize = sizeof(moo);
    moo.fMask = MIIM_SUBMENU | MIIM_STATE;

    GetMenuItemInfo(GetSubMenu(ramwatchmenu, 0), mitem, FALSE, &moo);
    moo.hSubMenu = menu;
    moo.fState = strlen(rw_recent_files[0]) ? MFS_ENABLED : MFS_GRAYED;

    SetMenuItemInfo(GetSubMenu(ramwatchmenu, 0), mitem, FALSE, &moo);

    // Remove all recent files submenus
    for (x = 0; x < MAX_RECENT_WATCHES; x++)
    {
        RemoveMenu(menu, baseid + x, MF_BYCOMMAND);
    }

    // Recreate the menus
    for (x = MAX_RECENT_WATCHES - 1; x >= 0; x--)
    {
        char tmp[128 + 5];

        // Skip empty strings
        if (!strlen(rw_recent_files[x]))
        {
            continue;
        }

        moo.cbSize = sizeof(moo);
        moo.fMask = MIIM_DATA | MIIM_ID | MIIM_TYPE;

        // Fill in the menu text.
        if (strlen(rw_recent_files[x]) < 128)
        {
            sprintf(tmp, "&%d. %s", (x + 1) % 10, rw_recent_files[x]);
        }
        else
        {
            sprintf(tmp, "&%d. %s", (x + 1) % 10, rw_recent_files[x] + strlen(rw_recent_files[x]) - 127);
        }

        // Insert the menu item
        moo.cch = strlen(tmp);
        moo.fType = 0;
        moo.wID = baseid + x;
        moo.dwTypeData = tmp;
        InsertMenuItem(menu, 0, 1, &moo);
    }
}

void UpdateRWRecentArray(const char* addString, unsigned int arrayLen, HMENU menu, unsigned int menuItem, unsigned int baseId)
{
    // Try to find out if the filename is already in the recent files list.
    for (unsigned int x = 0; x < arrayLen; x++)
    {
        if (strlen(rw_recent_files[x]))
        {
            if (!strcmp(rw_recent_files[x], addString))    // Item is already in list.
            {
                // If the filename is in the file list don't add it again.
                // Move it up in the list instead.

                int y;
                char tmp[1024];

                // Save pointer.
                strcpy(tmp, rw_recent_files[x]);

                for (y = x; y; y--)
                {
                    // Move items down.
                    strcpy(rw_recent_files[y], rw_recent_files[y - 1]);
                }

                // Put item on top.
                strcpy(rw_recent_files[0], tmp);

                // Update the recent files menu
                UpdateRW_RMenu(menu, menuItem, baseId);

                return;
            }
        }
    }

    // The filename wasn't found in the list. That means we need to add it.

    // Move the other items down.
    for (unsigned int x = arrayLen - 1; x; x--)
    {
        strcpy(rw_recent_files[x], rw_recent_files[x - 1]);
    }

    // Add the new item.
    strcpy(rw_recent_files[0], addString);

    // Update the recent files menu
    UpdateRW_RMenu(menu, menuItem, baseId);
}

void RWAddRecentFile(const char *filename)
{
    UpdateRWRecentArray(filename, MAX_RECENT_WATCHES, rwrecentmenu, RAMMENU_FILE_RECENT, RW_MENU_FIRST_RECENT_FILE);
}

void OpenRWRecentFile(int memwRFileNumber)
{
    if (!ResetWatches())
        return;

    int rnum = memwRFileNumber;
    if ((unsigned int)rnum >= MAX_RECENT_WATCHES)
        return; //just in case

    char* x;

    while (true)
    {
        x = rw_recent_files[rnum];
        if (!*x)
            return;		//If no recent files exist just return.  Useful for Load last file on startup (or if something goes screwy)

        if (rnum) //Change order of recent files if not most recent
        {
            RWAddRecentFile(x);
            rnum = 0;
        }
        else
        {
            break;
        }
    }

    strcpy(currentWatch, x);
    strcpy(Str_Tmp, currentWatch);

    //loadwatches here
    FILE *WatchFile = fopen(Str_Tmp, "rb");
    if (!WatchFile)
    {
        int answer = MessageBox(MESSAGEBOXPARENT, "Error opening file.", "ERROR", MB_OKCANCEL);
        if (answer == IDOK)
        {
            rw_recent_files[rnum][0] = '\0';	//Clear file from list
            if (rnum)							//Update the ramwatch list
                RWAddRecentFile(rw_recent_files[0]);
            else
                RWAddRecentFile(rw_recent_files[1]);
        }
        return;
    }
    const char DELIM = '\t';
    AddressWatcher Temp;
    char mode;
    fgets(Str_Tmp, 1024, WatchFile);
    sscanf(Str_Tmp, "%c%*s", &mode);
    if ((mode == '1' && !(SegaCD_Started)) || (mode == '2' && !(_32X_Started)))
    {
        char Device[8];
        strcpy(Device, (mode > '1') ? "32X" : "SegaCD");
        sprintf(Str_Tmp, "Warning: %s not started. \nWatches for %s addresses will be ignored.", Device, Device);
        MessageBox(MESSAGEBOXPARENT, Str_Tmp, "Possible Device Mismatch", MB_OK);
    }
    int WatchAdd;
    fgets(Str_Tmp, 1024, WatchFile);
    sscanf(Str_Tmp, "%d%*s", &WatchAdd);
    WatchAdd += WatchCount;
    for (int i = WatchCount; i < WatchAdd; i++)
    {
        while (i < 0)
            i++;
        do {
            fgets(Str_Tmp, 1024, WatchFile);
        } while (Str_Tmp[0] == '\n');
        sscanf(Str_Tmp, "%*05X%*c%08X%*c%c%*c%c%*c%d", &(Temp.Address), &(Temp.Size), &(Temp.Type), &(Temp.WrongEndian));
        Temp.WrongEndian = 0;
        char *Comment = strrchr(Str_Tmp, DELIM) + 1;
        *strrchr(Comment, '\n') = '\0';
        InsertWatch(Temp, Comment, WatchCount);
    }

    fclose(WatchFile);
    if (RamWatchHWnd) {
        ListView_SetItemCount(GetDlgItem(RamWatchHWnd, IDC_WATCHLIST), WatchCount);
        RefreshWatchListSelectedCountControlStatus(RamWatchHWnd);
    }
    RWfileChanged = false;
    return;
}

bool Save_Watches()
{
    strncpy(Str_Tmp, Rom_Name, 512);
    strcat(Str_Tmp, ".wch");
    if (Change_File_S(Str_Tmp, Gens_Path, "Save Watches", "GENs Watchlist\0*.wch\0All Files\0*.*\0\0", "wch", RamWatchHWnd))
    {
        FILE *WatchFile = fopen(Str_Tmp, "r+b");
        if (!WatchFile) WatchFile = fopen(Str_Tmp, "w+b");
        fputc(SegaCD_Started ? '1' : (_32X_Started ? '2' : '0'), WatchFile);
        fputc('\n', WatchFile);
        strcpy(currentWatch, Str_Tmp);
        RWAddRecentFile(currentWatch);
        sprintf(Str_Tmp, "%d\n", WatchCount);
        fputs(Str_Tmp, WatchFile);
        const char DELIM = '\t';
        for (int i = 0; i < WatchCount; i++)
        {
            sprintf(Str_Tmp, "%05X%c%08X%c%c%c%c%c%d%c%s\n", i, DELIM, rswatches[i].Address, DELIM, rswatches[i].Size, DELIM, rswatches[i].Type, DELIM, rswatches[i].WrongEndian, DELIM, rswatches[i].comment);
            fputs(Str_Tmp, WatchFile);
        }

        fclose(WatchFile);
        RWfileChanged = false;
        //TODO: Add to recent list function call here
        return true;
    }
    return false;
}

bool QuickSaveWatches()
{
    if (RWfileChanged == false) return true; //If file has not changed, no need to save changes
    if (currentWatch[0] == NULL) //If there is no currently loaded file, run to Save as and then return
    {
        return Save_Watches();
    }

    strcpy(Str_Tmp, currentWatch);
    FILE *WatchFile = fopen(Str_Tmp, "r+b");
    if (!WatchFile) WatchFile = fopen(Str_Tmp, "w+b");
    fputc(SegaCD_Started ? '1' : (_32X_Started ? '2' : '0'), WatchFile);
    fputc('\n', WatchFile);
    sprintf(Str_Tmp, "%d\n", WatchCount);
    fputs(Str_Tmp, WatchFile);
    const char DELIM = '\t';
    for (int i = 0; i < WatchCount; i++)
    {
        sprintf(Str_Tmp, "%05X%c%08X%c%c%c%c%c%d%c%s\n", i, DELIM, rswatches[i].Address, DELIM, rswatches[i].Size, DELIM, rswatches[i].Type, DELIM, rswatches[i].WrongEndian, DELIM, rswatches[i].comment);
        fputs(Str_Tmp, WatchFile);
    }
    fclose(WatchFile);
    RWfileChanged = false;
    return true;
}

bool Load_Watches(bool clear, const char* filename)
{
    const char DELIM = '\t';
    FILE* WatchFile = fopen(filename, "rb");
    if (!WatchFile)
    {
        MessageBox(MESSAGEBOXPARENT, "Error opening file.", "ERROR", MB_OK);
        return false;
    }
    if (clear)
    {
        if (!ResetWatches())
        {
            fclose(WatchFile);
            return false;
        }
    }
    strcpy(currentWatch, filename);
    RWAddRecentFile(currentWatch);
    AddressWatcher Temp;
    char mode;
    fgets(Str_Tmp, 1024, WatchFile);
    sscanf(Str_Tmp, "%c%*s", &mode);
    if ((mode == '1' && !(SegaCD_Started)) || (mode == '2' && !(_32X_Started)))
    {
        char Device[8];
        strcpy(Device, (mode > '1') ? "32X" : "SegaCD");
        sprintf(Str_Tmp, "Warning: %s not started. \nWatches for %s addresses will be ignored.", Device, Device);
        MessageBox(MESSAGEBOXPARENT, Str_Tmp, "Possible Device Mismatch", MB_OK);
    }
    int WatchAdd;
    fgets(Str_Tmp, 1024, WatchFile);
    sscanf(Str_Tmp, "%d%*s", &WatchAdd);
    WatchAdd += WatchCount;
    for (int i = WatchCount; i < WatchAdd; i++)
    {
        while (i < 0)
            i++;
        do {
            fgets(Str_Tmp, 1024, WatchFile);
        } while (Str_Tmp[0] == '\n');
        sscanf(Str_Tmp, "%*05X%*c%08X%*c%c%*c%c%*c%d", &(Temp.Address), &(Temp.Size), &(Temp.Type), &(Temp.WrongEndian));
        Temp.WrongEndian = 0;
        char *Comment = strrchr(Str_Tmp, DELIM) + 1;
        *strrchr(Comment, '\n') = '\0';
        InsertWatch(Temp, Comment, WatchCount);
    }

    fclose(WatchFile);
    if (RamWatchHWnd)
        ListView_SetItemCount(GetDlgItem(RamWatchHWnd, IDC_WATCHLIST), WatchCount);
    RWfileChanged = false;
    return true;
}

bool Load_Watches(bool clear)
{
    strncpy(Str_Tmp, Rom_Name, 512);
    strcat(Str_Tmp, ".wch");
    if (Change_File_L(Str_Tmp, Watch_Dir, "Load Watches", "GENs Watchlist\0*.wch\0All Files\0*.*\0\0", "wch", RamWatchHWnd))
    {
        return Load_Watches(clear, Str_Tmp);
    }
    return false;
}

bool ResetWatches()
{
    if (!AskSave())
        return false;
    for (; WatchCount >= 0; WatchCount--)
    {
        free(rswatches[WatchCount].comment);
        rswatches[WatchCount].comment = NULL;
    }
    WatchCount++;
    if (RamWatchHWnd) {
        ListView_SetItemCount(GetDlgItem(RamWatchHWnd, IDC_WATCHLIST), WatchCount);
        RefreshWatchListSelectedCountControlStatus(RamWatchHWnd);
    }
    RWfileChanged = false;
    currentWatch[0] = NULL;
    return true;
}

void RemoveWatch(int watchIndex)
{
    free(rswatches[watchIndex].comment);
    rswatches[watchIndex].comment = NULL;
    for (int i = watchIndex; i <= WatchCount; i++)
        rswatches[i] = rswatches[i + 1];
    WatchCount--;
}

/*
Utility function for reading an int from a dialog text box.
*/
int GetDlgItemTextAsInt(HWND hDlg, unsigned int IDC, unsigned int maxLen, char hexDec)
{
    int ret = 0;
    int result = 1;
    GetDlgItemText(hDlg, IDC, Str_Tmp, 1024);
    char *addrstr = Str_Tmp;
    if (strlen(Str_Tmp) > maxLen) addrstr = &(Str_Tmp[strlen(Str_Tmp) - maxLen]); //truncate and take the last maxLen characters
    for (int i = 0; addrstr[i]; i++) { if (toupper(addrstr[i]) == 'O') addrstr[i] = '0'; } // convert letter 'O' to number '0'
    if (*addrstr == 0) //empty text box results in 0
        ret = 0;
    else if (hexDec == 'h') //interpret text as a hex value
        result = sscanf(addrstr, "%08X", &(ret));
    else if (hexDec == 'd') //interpret text as a decimal value
        result = sscanf(addrstr, "%08d", &(ret));
    if (result != 1) // if sscanf failed to read in a number
    {
        std::string msg = std::string("Could not recognize input as a number: \"") + std::string(addrstr) + "\"";
        MessageBox(hDlg, msg.c_str(), "ERROR", MB_OK);
    }
    return ret;
}

bool ValidateAndAddWatch(HWND hDlg, unsigned int watchAddress, AddressWatcher &Temp, char *StrTmp, int index)
{
    Temp.Address = watchAddress;

    if ((Temp.Address & ~0xFFFFFF) == ~0xFFFFFF) // ?
        Temp.Address &= 0xFFFFFF;

    if (IsHardwareRAMAddressValid(Temp.Address))
    {
        InsertWatch(Temp, Str_Tmp, index);
        if (RamWatchHWnd)
        {
            ListView_SetItemCount(GetDlgItem(RamWatchHWnd, IDC_WATCHLIST), WatchCount);
        }
        return true;
    }
    else
    {
        return false;
    }
}

//Gets info for a RAM Watch (or multiple watches using a range or steps), and then inserts it into the Watch List
LRESULT CALLBACK EditWatchProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT r;
    RECT r2;
    int dx1, dy1, dx2, dy2;
    static int index;
    static char s, t = s = 0; //s: data size (byte/word/doubleword).  t=display type (signed/unsigned/hex)

    switch (uMsg)
    {
    case WM_INITDIALOG:
        Clear_Sound_Buffer();

        if (Full_Screen)
        {
            while (ShowCursor(false) >= 0);
            while (ShowCursor(true) < 0);
        }

        GetWindowRect(HWnd, &r);
        dx1 = (r.right - r.left) / 2;
        dy1 = (r.bottom - r.top) / 2;

        GetWindowRect(hDlg, &r2);
        dx2 = (r2.right - r2.left) / 2;
        dy2 = (r2.bottom - r2.top) / 2;

        //SetWindowPos(hDlg, NULL, max(0, r.left + (dx1 - dx2)), max(0, r.top + (dy1 - dy2)), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
        SetWindowPos(hDlg, NULL, r.left, r.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
        index = (int)lParam;
        if (index < WatchCount) // disable range and step input when editing an existing watch
        {
            EnableWindow(GetDlgItem(hDlg, (int)IDC_EDIT_COMPAREADDRESSRANGE), FALSE);
            EnableWindow(GetDlgItem(hDlg, (int)IDC_EDIT_COMPAREADDRESSSTEP), FALSE);
            EnableWindow(GetDlgItem(hDlg, (int)IDC_EDIT_COMPAREADDRESSSTEPCOUNT), FALSE);
        }
        sprintf(Str_Tmp, "%08X", rswatches[index].Address);
        SetDlgItemText(hDlg, IDC_EDIT_COMPAREADDRESS, Str_Tmp);
        if (rswatches[index].comment != NULL)
            SetDlgItemText(hDlg, IDC_PROMPT_EDIT, rswatches[index].comment);
        s = rswatches[index].Size;
        t = rswatches[index].Type;
        switch (s) // data size
        {
        case 'b':
            SendDlgItemMessage(hDlg, IDC_1_BYTE, BM_SETCHECK, BST_CHECKED, 0);
            break;
        case 'w':
            SendDlgItemMessage(hDlg, IDC_2_BYTES, BM_SETCHECK, BST_CHECKED, 0);
            break;
        case 'd':
            SendDlgItemMessage(hDlg, IDC_4_BYTES, BM_SETCHECK, BST_CHECKED, 0);
            break;
        default:
            s = 0;
            break;
        }
        switch (t) // display type
        {
        case 's':
            SendDlgItemMessage(hDlg, IDC_SIGNED, BM_SETCHECK, BST_CHECKED, 0);
            break;
        case 'u':
            SendDlgItemMessage(hDlg, IDC_UNSIGNED, BM_SETCHECK, BST_CHECKED, 0);
            break;
        case 'h':
            SendDlgItemMessage(hDlg, IDC_HEX, BM_SETCHECK, BST_CHECKED, 0);
            break;
        default:
            t = 0;
            break;
        }

        return true;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_SIGNED:
            t = 's';
            return true;
        case IDC_UNSIGNED:
            t = 'u';
            return true;
        case IDC_HEX:
            t = 'h';
            return true;
        case IDC_1_BYTE:
            s = 'b';
            return true;
        case IDC_2_BYTES:
            s = 'w';
            return true;
        case IDC_4_BYTES:
            s = 'd';
            return true;
        case IDC_BTN_HELP:
            MessageBox(hDlg,
                "Edit Watch Help\n"
                "-----------------\n"
                "Basic Usage:\n"
                "Enter an Address to watch. Example: FF1234. "
                "Select the size of the data: 1, 2, or 4 bytes.  Select the display type of the data: signed, unsigned, or hexadecimal.  Optionally, add a note.\n"
                "\n"
                "Advanced Usage:\n"
                "Note that all the advanced options are specified in decimal values, not hexadecimal.\n"
                "There are 3 other ways to add watches.\n"
                "1) Specify an Offset, and a watch will be created for the address at Address+Offset.\n"
                "2) Specify a Range Length, and multiple watches will be created, covering all memory from Address to Address+Length-1.\n"
                "3) Specify a Step, Step Count, and (optionally) an Offset, and Step Count watches will be created, the first at Address+Offset+(1*Step), the second at Address+Offset+(2*Step), and so on.  This is useful for watching arrays of structures."
                , "Help", MB_OK | MB_ICONINFORMATION);
            return true;
        case IDOK:
        {
            if (Full_Screen)
            {
                while (ShowCursor(true) < 0);
                while (ShowCursor(false) >= 0);
            }
            if (s && t)
            {
                AddressWatcher Temp;
                Temp.Size = s;
                Temp.Type = t;
                Temp.WrongEndian = 0; //replace this when I get little endian working properly
                unsigned int watchAddress = 0;
                int rangeSize = 0; // use int (signed) so we can protect against negatives
                int offset = 0;
                int step = 0;
                int stepCount = 0;
                unsigned int sval; // byte = 1, word = 2, doubleword = 4
                int tmp;

                watchAddress = GetDlgItemTextAsInt(hDlg, IDC_EDIT_COMPAREADDRESS, 8, 'h');
                rangeSize = GetDlgItemTextAsInt(hDlg, IDC_EDIT_COMPAREADDRESSRANGE, 4, 'd');
                offset = GetDlgItemTextAsInt(hDlg, IDC_EDIT_COMPAREADDRESSOFFSET, 4, 'd');
                step = GetDlgItemTextAsInt(hDlg, IDC_EDIT_COMPAREADDRESSSTEP, 4, 'd');
                stepCount = GetDlgItemTextAsInt(hDlg, IDC_EDIT_COMPAREADDRESSSTEPCOUNT, 4, 'd');
                GetDlgItemText(hDlg, IDC_PROMPT_EDIT, Str_Tmp, 80);

                switch (Temp.Size)
                {
                case 'b':
                    sval = 1;
                    break;
                case 'w':
                    sval = 2;
                    break;
                case 'd':
                    sval = 4;
                    break;
                }

                /*
                 * Three different methods:
                 * 1.  single address:  base address + offset(optional)
                 * 2.  range: base address to base address + range size - 1
                 * 3.  step: base address + offset + step * (for 0 to stepcount-1)
                 * which method to use is decided based on what parameters were supplied with non-zero values.
                 */
                if (!rangeSize && !step)
                {
                    watchAddress += offset;
                    if (!ValidateAndAddWatch(hDlg, watchAddress, Temp, Str_Tmp, index))
                    {
                        sprintf(Str_Tmp, "Invalid Address: %X", watchAddress);
                        MessageBox(hDlg, Str_Tmp, "ERROR", MB_OK | MB_ICONSTOP);
                    }
                }
                else if (rangeSize)
                {
                    for (int rangeval = 0; rangeval < rangeSize; rangeval += sval)
                    {
                        tmp = strlen(Str_Tmp);
                        sprintf(Str_Tmp, "%s [%d]", Str_Tmp, rangeval);
                        if (!ValidateAndAddWatch(hDlg, watchAddress + rangeval, Temp, Str_Tmp, index))
                        {
                            sprintf(Str_Tmp, "Invalid Address: %X (%#X in range)", watchAddress + rangeval, rangeval);
                            MessageBox(hDlg, Str_Tmp, "ERROR", MB_OK | MB_ICONSTOP);
                            break;
                        }
                        index++;
                        Str_Tmp[tmp] = '\0';
                    }
                }
                else // step method
                {
                    watchAddress += offset;
                    for (int stepval = 0; stepval < stepCount; stepval++)
                    {
                        tmp = strlen(Str_Tmp);
                        sprintf(Str_Tmp, "%s [step %d]", Str_Tmp, stepval);
                        if (!ValidateAndAddWatch(hDlg, watchAddress + step*stepval, Temp, Str_Tmp, index))
                        {
                            sprintf(Str_Tmp, "Invalid Address: %X (step number %d)", watchAddress + step*stepval, stepval);
                            MessageBox(hDlg, Str_Tmp, "ERROR", MB_OK | MB_ICONSTOP);
                            break;
                        }
                        index++;
                        Str_Tmp[tmp] = '\0';
                    }
                }

                /* Even if something goes wrong (Invalid Address error), we will close the dialog.
                 * This way, if the error happens after part of a range or stepping is completed, the user can see how far things got before the error by seeing the watches that got added.
                 * Better ways this could be handled:	If the first watch added fails, we don't close the dialog (need a way to track this).
                 *										Or maybe remove all the newly added watches, show user which address was bad, and leave dialog open..
                 */
                DialogsOpen--;
                EndDialog(hDlg, true);
            }
            else
            {
                strcpy(Str_Tmp, "Error:");
                if (!s)
                    strcat(Str_Tmp, " Size must be specified.");
                if (!t)
                    strcat(Str_Tmp, " Type must be specified.");
                MessageBox(hDlg, Str_Tmp, "ERROR", MB_OK);
            }
            RWfileChanged = true;
            return true;
            break;
        }
        case ID_CANCEL:
        case IDCANCEL:
            if (Full_Screen)
            {
                while (ShowCursor(true) < 0);
                while (ShowCursor(false) >= 0);
            }
            DialogsOpen--;
            EndDialog(hDlg, false);
            return false;
            break;
        }
        break;

    case WM_CLOSE:
        if (Full_Screen)
        {
            while (ShowCursor(true) < 0);
            while (ShowCursor(false) >= 0);
        }
        DialogsOpen--;
        EndDialog(hDlg, false);
        return false;
        break;
    }

    return false;
}

void RamWatchEnableCommand(HWND hDlg, HMENU hMenu, UINT uIDEnableItem, bool enable)
{
    EnableWindow(GetDlgItem(hDlg, uIDEnableItem), (enable ? TRUE : FALSE));
    if (hMenu != NULL) {
        if (uIDEnableItem == ID_WATCHES_UPDOWN) {
            EnableMenuItem(hMenu, IDC_C_WATCH_UP, MF_BYCOMMAND | (enable ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(hMenu, IDC_C_WATCH_DOWN, MF_BYCOMMAND | (enable ? MF_ENABLED : MF_GRAYED));
        }
        else
            EnableMenuItem(hMenu, uIDEnableItem, MF_BYCOMMAND | (enable ? MF_ENABLED : MF_GRAYED));
    }
}

void RefreshWatchListSelectedCountControlStatus(HWND hDlg)
{
    static int prevSelCount = -1;
    int selCount = ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_WATCHLIST));
    if (selCount != prevSelCount)
    {
        if (selCount < 2 || prevSelCount < 2)
        {
            RamWatchEnableCommand(hDlg, ramwatchmenu, IDC_C_WATCH_EDIT, selCount == 1);
            RamWatchEnableCommand(hDlg, ramwatchmenu, IDC_C_WATCH_REMOVE, selCount >= 1);
            RamWatchEnableCommand(hDlg, ramwatchmenu, IDC_C_WATCH_DUPLICATE, selCount == 1);
            RamWatchEnableCommand(hDlg, ramwatchmenu, IDC_C_ADDCHEAT, selCount == 1);
            RamWatchEnableCommand(hDlg, ramwatchmenu, ID_WATCHES_UPDOWN, selCount == 1);
        }
        prevSelCount = selCount;
    }
}

LRESULT CALLBACK RamWatchProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT r;
    RECT r2;
    int dx1, dy1, dx2, dy2;
    static int watchIndex = 0;

    switch (uMsg)
    {
    case WM_MOVE: {
        RECT wrect;
        GetWindowRect(hDlg, &wrect);
        ramw_x = wrect.left;
        ramw_y = wrect.top;
        break;
    };

    case WM_INITDIALOG: {
        if (Full_Screen)
        {
            while (ShowCursor(false) >= 0);
            while (ShowCursor(true) < 0);
        }

        GetWindowRect(HWnd, &r);  //Ramwatch window
        dx1 = (r.right - r.left) / 2;
        dy1 = (r.bottom - r.top) / 2;

        GetWindowRect(hDlg, &r2); // Gens window
        dx2 = (r2.right - r2.left) / 2;
        dy2 = (r2.bottom - r2.top) / 2;

        // push it away from the main window if we can
        const int width = (r.right - r.left);
        const int height = (r.bottom - r.top);
        const int width2 = (r2.right - r2.left);
        if (r.left + width2 + width < GetSystemMetrics(SM_CXSCREEN))
        {
            r.right += width;
            r.left += width;
        }
        else if ((int)r.left - (int)width2 > 0)
        {
            r.right -= width2;
            r.left -= width2;
        }

        //-----------------------------------------------------------------------------------
        //If user has Save Window Pos selected, override default positioning
        if (RWSaveWindowPos)
        {
            //If ramwindow is for some reason completely off screen, use default instead
            if (ramw_x > (-width * 2) || ramw_x < (width * 2 + GetSystemMetrics(SM_CYSCREEN)))
                r.left = ramw_x;	  //This also ignores cases of windows -32000 error codes
            //If ramwindow is for some reason completely off screen, use default instead
            if (ramw_y > (0 - height * 2) || ramw_y < (height * 2 + GetSystemMetrics(SM_CYSCREEN)))
                r.top = ramw_y;		  //This also ignores cases of windows -32000 error codes
        }
        //-------------------------------------------------------------------------------------
        SetWindowPos(hDlg, NULL, r.left, r.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

        ramwatchmenu = GetMenu(hDlg);
        rwrecentmenu = CreateMenu();
        UpdateRW_RMenu(rwrecentmenu, RAMMENU_FILE_RECENT, RW_MENU_FIRST_RECENT_FILE);

        const char* names[3] = { "Address", "Value", "Notes" };
        int widths[3] = { 62, 64, 64 + 51 + 53 };
        init_list_box(GetDlgItem(hDlg, IDC_WATCHLIST), names, 3, widths);
        if (!ResultCount)
            reset_address_info();
        else
            signal_new_frame();
        ListView_SetItemCount(GetDlgItem(hDlg, IDC_WATCHLIST), WatchCount);
        if (!noMisalign) SendDlgItemMessage(hDlg, IDC_MISALIGN, BM_SETCHECK, BST_CHECKED, 0);
        if (littleEndian) SendDlgItemMessage(hDlg, IDC_ENDIAN, BM_SETCHECK, BST_CHECKED, 0);

        RamWatchAccels = LoadAccelerators(ghInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

        // due to some bug in windows, the arrow button width from the resource gets ignored, so we have to set it here
        SetWindowPos(GetDlgItem(hDlg, ID_WATCHES_UPDOWN), 0, 0, 0, 30, 60, SWP_NOMOVE);

        Update_RAM_Watch();

        DragAcceptFiles(hDlg, TRUE);

        RefreshWatchListSelectedCountControlStatus(hDlg);
        return true;
        break;
    }

    case WM_INITMENU:
        CheckMenuItem(ramwatchmenu, RAMMENU_FILE_AUTOLOAD, AutoRWLoad ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(ramwatchmenu, RAMMENU_FILE_SAVEWINDOW, RWSaveWindowPos ? MF_CHECKED : MF_UNCHECKED);
        break;

    case WM_MENUSELECT:
    case WM_ENTERSIZEMOVE:
        Clear_Sound_Buffer();
        break;

    case WM_NOTIFY:
    {
        LPNMHDR lP = (LPNMHDR)lParam;
        switch (lP->code)
        {
        case LVN_ITEMCHANGED: // selection changed event
        {
            NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lP;
            if (pNMListView->uNewState & LVIS_FOCUSED ||
                (pNMListView->uNewState ^ pNMListView->uOldState) & LVIS_SELECTED)
            {
                // disable buttons that we don't have the right number of selected items for
                RefreshWatchListSelectedCountControlStatus(hDlg);
            }
        }	break;

        case LVN_GETDISPINFO:
        {
            LV_DISPINFO *Item = (LV_DISPINFO *)lParam;
            Item->item.mask = LVIF_TEXT;
            Item->item.state = 0;
            Item->item.iImage = 0;
            const unsigned int iNum = Item->item.iItem;
            static char num[11];
            switch (Item->item.iSubItem)
            {
            case 0:
                sprintf(num, "%08X", rswatches[iNum].Address);
                Item->item.pszText = num;
                return true;
            case 1: {
                int i = rswatches[iNum].CurValue;
                int t = rswatches[iNum].Type;
                int size = rswatches[iNum].Size;
                const char* formatString = ((t == 's') ? "%d" : (t == 'u') ? "%u" : (size == 'd' ? "%08X" : size == 'w' ? "%04X" : "%02X"));
                switch (size)
                {
                case 'b':
                default: sprintf(num, formatString, t == 's' ? (char)(i & 0xff) : (unsigned char)(i & 0xff)); break;
                case 'w': sprintf(num, formatString, t == 's' ? (short)(i & 0xffff) : (unsigned short)(i & 0xffff)); break;
                case 'd': sprintf(num, formatString, t == 's' ? (long)(i & 0xffffffff) : (unsigned long)(i & 0xffffffff)); break;
                }

                Item->item.pszText = num;
            }	return true;
            case 2:
                Item->item.pszText = rswatches[iNum].comment ? rswatches[iNum].comment : "";
                return true;

            default:
                return false;
            }
        }
        case LVN_ODFINDITEM:
        {
            // disable search by keyboard typing,
            // because it interferes with some of the accelerators
            // and it isn't very useful here anyway
            SetWindowLong(hDlg, DWL_MSGRESULT, ListView_GetSelectionMark(GetDlgItem(hDlg, IDC_WATCHLIST)));
            return 1;
        }
        }
    } break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case RAMMENU_FILE_SAVE:
            QuickSaveWatches();
            break;

        case RAMMENU_FILE_SAVEAS:
            //case IDC_C_SAVE:
            return Save_Watches();
        case RAMMENU_FILE_OPEN:
            return Load_Watches(true);
        case RAMMENU_FILE_APPEND:
            //case IDC_C_LOAD:
            return Load_Watches(false);
        case RAMMENU_FILE_NEW:
            //case IDC_C_RESET:
            ResetWatches();
            return true;
        case IDC_C_WATCH_REMOVE:
        {
            HWND watchListControl = GetDlgItem(hDlg, IDC_WATCHLIST);
            watchIndex = ListView_GetNextItem(watchListControl, -1, LVNI_ALL | LVNI_SELECTED);
            while (watchIndex >= 0)
            {
                RemoveWatch(watchIndex);
                ListView_DeleteItem(watchListControl, watchIndex);
                watchIndex = ListView_GetNextItem(watchListControl, -1, LVNI_ALL | LVNI_SELECTED);
            }
            RWfileChanged = true;
            SetFocus(GetDlgItem(hDlg, IDC_WATCHLIST));
            return true;
        }
        case IDC_C_WATCH_EDIT:
            watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg, IDC_WATCHLIST));
            DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_EDITWATCH), hDlg, (DLGPROC)EditWatchProc, (LPARAM)watchIndex);
            SetFocus(GetDlgItem(hDlg, IDC_WATCHLIST));
            return true;
        case IDC_C_WATCH:
            rswatches[WatchCount].Address = rswatches[WatchCount].WrongEndian = 0;
            rswatches[WatchCount].Size = 'b';
            rswatches[WatchCount].Type = 's';
            DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_EDITWATCH), hDlg, (DLGPROC)EditWatchProc, (LPARAM)WatchCount);
            SetFocus(GetDlgItem(hDlg, IDC_WATCHLIST));
            return true;
        case IDC_C_WATCH_DUPLICATE:
            watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg, IDC_WATCHLIST));
            rswatches[WatchCount].Address = rswatches[watchIndex].Address;
            rswatches[WatchCount].WrongEndian = rswatches[watchIndex].WrongEndian;
            rswatches[WatchCount].Size = rswatches[watchIndex].Size;
            rswatches[WatchCount].Type = rswatches[watchIndex].Type;
            DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_EDITWATCH), hDlg, (DLGPROC)EditWatchProc, (LPARAM)WatchCount);
            SetFocus(GetDlgItem(hDlg, IDC_WATCHLIST));
            return true;

        case IDC_C_WATCH_SEPARATE:
            AddressWatcher separator;
            separator.Address = 0;
            separator.WrongEndian = 0;
            separator.Size = 'S';
            separator.Type = 'S';
            InsertWatch(separator, "----------------------------", WatchCount);
            SetFocus(GetDlgItem(hDlg, IDC_WATCHLIST));
            return true;

        case IDC_C_WATCH_UP:
        {
            watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg, IDC_WATCHLIST));
            if (watchIndex == 0 || watchIndex == -1)
                return true;
            void *tmp = malloc(sizeof(AddressWatcher));
            memcpy(tmp, &(rswatches[watchIndex]), sizeof(AddressWatcher));
            memcpy(&(rswatches[watchIndex]), &(rswatches[watchIndex - 1]), sizeof(AddressWatcher));
            memcpy(&(rswatches[watchIndex - 1]), tmp, sizeof(AddressWatcher));
            free(tmp);
            ListView_SetItemState(GetDlgItem(hDlg, IDC_WATCHLIST), watchIndex, 0, LVIS_FOCUSED | LVIS_SELECTED);
            ListView_SetSelectionMark(GetDlgItem(hDlg, IDC_WATCHLIST), watchIndex - 1);
            ListView_SetItemState(GetDlgItem(hDlg, IDC_WATCHLIST), watchIndex - 1, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            ListView_SetItemCount(GetDlgItem(hDlg, IDC_WATCHLIST), WatchCount);
            RWfileChanged = true;
            return true;
        }
        case IDC_C_WATCH_DOWN:
        {
            watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg, IDC_WATCHLIST));
            if (watchIndex >= WatchCount - 1 || watchIndex == -1)
                return true;
            void *tmp = malloc(sizeof(AddressWatcher));
            memcpy(tmp, &(rswatches[watchIndex]), sizeof(AddressWatcher));
            memcpy(&(rswatches[watchIndex]), &(rswatches[watchIndex + 1]), sizeof(AddressWatcher));
            memcpy(&(rswatches[watchIndex + 1]), tmp, sizeof(AddressWatcher));
            free(tmp);
            ListView_SetItemState(GetDlgItem(hDlg, IDC_WATCHLIST), watchIndex, 0, LVIS_FOCUSED | LVIS_SELECTED);
            ListView_SetSelectionMark(GetDlgItem(hDlg, IDC_WATCHLIST), watchIndex + 1);
            ListView_SetItemState(GetDlgItem(hDlg, IDC_WATCHLIST), watchIndex + 1, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            ListView_SetItemCount(GetDlgItem(hDlg, IDC_WATCHLIST), WatchCount);
            RWfileChanged = true;
            return true;
        }
        case ID_WATCHES_UPDOWN:
        {
            int delta = ((LPNMUPDOWN)lParam)->iDelta;
            SendMessage(hDlg, WM_COMMAND, delta < 0 ? IDC_C_WATCH_UP : IDC_C_WATCH_DOWN, 0);
            break;
        }
        case RAMMENU_FILE_AUTOLOAD:
        {
            AutoRWLoad ^= 1;
            CheckMenuItem(ramwatchmenu, RAMMENU_FILE_AUTOLOAD, AutoRWLoad ? MF_CHECKED : MF_UNCHECKED);
            break;
        }
        case RAMMENU_FILE_SAVEWINDOW:
        {
            RWSaveWindowPos ^= 1;
            CheckMenuItem(ramwatchmenu, RAMMENU_FILE_SAVEWINDOW, RWSaveWindowPos ? MF_CHECKED : MF_UNCHECKED);
            break;
        }
        case IDC_C_ADDCHEAT:
        {
            //					watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg,IDC_WATCHLIST)) | (1 << 24);
            //					DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_EDITCHEAT), hDlg, (DLGPROC) EditCheatProc,(LPARAM) searchIndex);
        }
        case IDOK:
        case IDCANCEL:
            if (Full_Screen)
            {
                while (ShowCursor(true) < 0);
                while (ShowCursor(false) >= 0);
            }
            DialogsOpen--;
            RamWatchHWnd = NULL;
            DragAcceptFiles(hDlg, FALSE);
            EndDialog(hDlg, true);
            return true;
        default:
            if (LOWORD(wParam) >= RW_MENU_FIRST_RECENT_FILE && LOWORD(wParam) < RW_MENU_FIRST_RECENT_FILE + MAX_RECENT_WATCHES)
                OpenRWRecentFile(LOWORD(wParam) - RW_MENU_FIRST_RECENT_FILE);
        }
        break;

    case WM_KEYDOWN: // handle accelerator keys
    {
        SetFocus(GetDlgItem(hDlg, IDC_WATCHLIST));
        MSG msg;
        msg.hwnd = hDlg;
        msg.message = uMsg;
        msg.wParam = wParam;
        msg.lParam = lParam;
        if (RamWatchAccels && TranslateAccelerator(hDlg, RamWatchAccels, &msg))
            return true;
    }	break;

    case WM_CLOSE:
        if (Full_Screen)
        {
            while (ShowCursor(true) < 0);
            while (ShowCursor(false) >= 0);
        }
        DialogsOpen--;
        RamWatchHWnd = NULL;
        DragAcceptFiles(hDlg, FALSE);
        EndDialog(hDlg, true);
        return true;

    case WM_DROPFILES:
    {
        HDROP hDrop = (HDROP)wParam;
        DragQueryFile(hDrop, 0, Str_Tmp, 1024);
        DragFinish(hDrop);
        return Load_Watches(true, Str_Tmp);
    }	break;
    }

    return false;
}