#include <stdio.h>
#include "gens.h"
#include "g_main.h"
#include "joypads.h"
#include "g_ddraw.h"
#include "g_input.h"
#include "movie.h"
#include "mem_m68k.h"
#include "luascript.h"

long unsigned int FrameCount = 0;
long unsigned int LagCount = 0;
long unsigned int LagCountPersistent = 0; // same as LagCount but ignores manual resets
long unsigned int Track1_FrameCount = 0;
long unsigned int Track2_FrameCount = 0;
long unsigned int Track3_FrameCount = 0;
unsigned int Temp_Controller_Type[4]; // upthadd - for saving the config's controller modes
char Recent_Movie[MAX_RECENT_MOVIES][1024];
char Movie_Dir[1024] = "";
bool tempflag = false; //Upth-Add - This is for the new feature which pauses at the end of movie play if readonly isn't set
bool AutoCloseMovie = false; //Upth-Add - For the new AutoClose Movie toggle
bool Def_Read_Only = true; //Upth-Add - For the new Default Read Only toggle
char track = 1 | 2 | 4;
typeMovie MainMovie;
extern "C" char preloaded_tracks[100], played_tracks_linear[105]; // Modif N. -- added
extern "C" int Clear_Sound_Buffer(void);

void Update_Recent_Movie(const char *Path)
{
    int i;
    for (i = 0; i < MAX_RECENT_MOVIES; i++)
    {
        if (!(strcmp(Recent_Movie[i], Path)))
        {
            // move recent item to the top of the list
            if (i == 0)
                return;
            char temp[1024];
            strcpy(temp, Recent_Movie[i]);
            int j;
            for (j = i; j > 0; j--)
                strcpy(Recent_Movie[j], Recent_Movie[j - 1]);
            strcpy(Recent_Movie[0], temp);
            return;
        }
    }

    for (i = MAX_RECENT_MOVIES - 1; i > 0; i--)
        strcpy(Recent_Movie[i], Recent_Movie[i - 1]);

    strcpy(Recent_Movie[0], Path);
}

//Modif
void MoviePlayingStuff()
{
    if (tempflag) //Upth-Add - If we just reached the end of the movie and paused
    {             //Upth-Add - we now switch to recording
        if (AutoBackupEnabled)
        {
            strncpy(Str_Tmp, MainMovie.FileName, 512);
            for (int i = strlen(Str_Tmp); i >= 0; i--) if (Str_Tmp[i] == '|') Str_Tmp[i] = '_';
            strcat(MainMovie.FileName, ".gmv");
            MainMovie.FileName[strlen(MainMovie.FileName) - 7] = 'b'; // ".bak"
            MainMovie.FileName[strlen(MainMovie.FileName) - 6] = 'a';
            MainMovie.FileName[strlen(MainMovie.FileName) - 5] = 'k';
            BackupMovieFile(&MainMovie);
            strncpy(MainMovie.FileName, Str_Tmp, 512);
        }
        MainMovie.Status = MOVIE_RECORDING;
        MustUpdateMenu = 1;
        tempflag = 0;
        MovieRecordingStuff();
        return;
    }

    if (!MainMovie.File) // can't continue if MainMovie.File is NULL (we would crash)
    {                   // this should never happen, but better safe than sorry
        DialogsOpen++;
        MessageBox(HWnd, "Movie is not playing.", "Warning", MB_ICONWARNING);
        DialogsOpen--;
        MainMovie.Status = 0;
        return;
    }

    char PadData[3]; //Modif

    Check_Misc_Key();
    fseek(MainMovie.File, 64 + FrameCount * 3, SEEK_SET);
    fread(PadData, 3, 1, MainMovie.File);
    Controller_1_Up = (PadData[0] & 1);
    Controller_1_Down = (PadData[0] & 2) >> 1;
    Controller_1_Left = (PadData[0] & 4) >> 2;
    Controller_1_Right = (PadData[0] & 8) >> 3;
    Controller_1_A = (PadData[0] & 16) >> 4;
    Controller_1_B = (PadData[0] & 32) >> 5;
    Controller_1_C = (PadData[0] & 64) >> 6;
    Controller_1_Start = (PadData[0] & 128) >> 7;
    if (MainMovie.TriplePlayerHack)
    {
        Controller_1B_Up = (PadData[1] & 1);
        Controller_1B_Down = (PadData[1] & 2) >> 1;
        Controller_1B_Left = (PadData[1] & 4) >> 2;
        Controller_1B_Right = (PadData[1] & 8) >> 3;
        Controller_1B_A = (PadData[1] & 16) >> 4;
        Controller_1B_B = (PadData[1] & 32) >> 5;
        Controller_1B_C = (PadData[1] & 64) >> 6;
        Controller_1B_Start = (PadData[1] & 128) >> 7;
        Controller_1C_Up = (PadData[2] & 1);
        Controller_1C_Down = (PadData[2] & 2) >> 1;
        Controller_1C_Left = (PadData[2] & 4) >> 2;
        Controller_1C_Right = (PadData[2] & 8) >> 3;
        Controller_1C_A = (PadData[2] & 16) >> 4;
        Controller_1C_B = (PadData[2] & 32) >> 5;
        Controller_1C_C = (PadData[2] & 64) >> 6;
        Controller_1C_Start = (PadData[2] & 128) >> 7;
    }
    else
    {
        Controller_2_Up = (PadData[1] & 1);
        Controller_2_Down = (PadData[1] & 2) >> 1;
        Controller_2_Left = (PadData[1] & 4) >> 2;
        Controller_2_Right = (PadData[1] & 8) >> 3;
        Controller_2_A = (PadData[1] & 16) >> 4;
        Controller_2_B = (PadData[1] & 32) >> 5;
        Controller_2_C = (PadData[1] & 64) >> 6;
        Controller_2_Start = (PadData[1] & 128) >> 7;
        Controller_1_X = (PadData[2] & 1);
        Controller_1_Y = (PadData[2] & 2) >> 1;
        Controller_1_Z = (PadData[2] & 4) >> 2;
        Controller_1_Mode = (PadData[2] & 8) >> 3;
        Controller_2_X = (PadData[2] & 16) >> 4;
        Controller_2_Y = (PadData[2] & 32) >> 5;
        Controller_2_Z = (PadData[2] & 64) >> 6;
        Controller_2_Mode = (PadData[2] & 128) >> 7;
    }

    if (FrameCount >= MainMovie.LastFrame - 1)
    {
        if (AutoCloseMovie) //Upth-Add - If we have the movie set to autoclose then we probably weren't intending to rerecord
        {
            MainMovie.Status = MOVIE_FINISHED;
            sprintf(Str_Tmp, "Movie finished");
            Put_Info(Str_Tmp);
            MustUpdateMenu = 1;
            CloseMovieFile(&MainMovie);//UpthAdd - So controller settings are restored
        }
        else if (MainMovie.Recorded || !MainMovie.ReadOnly) //Upth-Add - Otherwise, though
        {
            Clear_Sound_Buffer(); //eliminate stutter

            int result = MainMovie.ReadOnly ? IDNO : IDYES;
            if (MainMovie.ReadOnly == 1)
            {
                DialogsOpen++;
                result = MessageBox(HWnd, "Movie end reached. Resume recording?", "Notice", MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION);
                DialogsOpen--;
            }
            if (result == IDYES)
            {
                Paused = 1; //Upth-Add - We should pause
                Pause_Screen();
                sprintf(Str_Tmp, "Movie end reached; Paused; Recording will resume."); //Upth-Add - Announce that the movie has paused and will switch to recording
                MainMovie.Status = MOVIE_RECORDING;
                Put_Info(Str_Tmp);
                tempflag = true; //Upth-Add - And set the flag which brings us back into recording mode
            }
            else
            {
                MainMovie.Status = MOVIE_FINISHED;
                sprintf(Str_Tmp, "Movie finished");
                Put_Info(Str_Tmp);
                MustUpdateMenu = 1;
            }
        }
        else
        {
            MainMovie.Status = MOVIE_FINISHED;
            sprintf(Str_Tmp, "Movie finished");
            Put_Info(Str_Tmp);
            MustUpdateMenu = 1;
        }
    }

    // because this function gets called instead of Update_Controllers
    // and Update_Controllers is otherwise responsible for LUACALL_BEFOREEMULATION
    CallRegisteredLuaFunctions(LUACALL_BEFOREEMULATION);
}
//Modif N - p1copy (playback player 1 while recording others)
void MoviePlayPlayer1()
{
    char PadData[3]; //Modif
    //	if(!fseek(MainMovie.File,64+FrameCount*3,SEEK_SET))
    fseek(MainMovie.File, 64 + FrameCount * 3, SEEK_SET);
    {
        fread(PadData, 3, 1, MainMovie.File);
        {
            Controller_1_Up = (PadData[0] & 1);
            Controller_1_Down = (PadData[0] & 2) >> 1;
            Controller_1_Left = (PadData[0] & 4) >> 2;
            Controller_1_Right = (PadData[0] & 8) >> 3;
            Controller_1_A = (PadData[0] & 16) >> 4;
            Controller_1_B = (PadData[0] & 32) >> 5;
            Controller_1_C = (PadData[0] & 64) >> 6;
            Controller_1_Start = (PadData[0] & 128) >> 7;
            if (!(MainMovie.TriplePlayerHack))
            {
                Controller_1_X = (PadData[2] & 1);
                Controller_1_Y = (PadData[2] & 2) >> 1;
                Controller_1_Z = (PadData[2] & 4) >> 2;
                Controller_1_Mode = (PadData[2] & 8) >> 3;
            }
        }
    }
}
// XXX - probably most people won't need to use this?
//Modif N - p2copy (playback player 2 while recording others)
void MoviePlayPlayer2()
{
    char PadData[3]; //Modif
    //	if(!fseek(MainMovie.File,64+FrameCount*3,SEEK_SET))
    fseek(MainMovie.File, 64 + FrameCount * 3, SEEK_SET);
    {
        fread(PadData, 3, 1, MainMovie.File);
        {
            if (MainMovie.TriplePlayerHack)
            {
                Controller_1B_Up = (PadData[1] & 1);
                Controller_1B_Down = (PadData[1] & 2) >> 1;
                Controller_1B_Left = (PadData[1] & 4) >> 2;
                Controller_1B_Right = (PadData[1] & 8) >> 3;
                Controller_1B_A = (PadData[1] & 16) >> 4;
                Controller_1B_B = (PadData[1] & 32) >> 5;
                Controller_1B_C = (PadData[1] & 64) >> 6;
                Controller_1B_Start = (PadData[1] & 128) >> 7;
            }
            else
            {
                Controller_2_Up = (PadData[1] & 1);
                Controller_2_Down = (PadData[1] & 2) >> 1;
                Controller_2_Left = (PadData[1] & 4) >> 2;
                Controller_2_Right = (PadData[1] & 8) >> 3;
                Controller_2_A = (PadData[1] & 16) >> 4;
                Controller_2_B = (PadData[1] & 32) >> 5;
                Controller_2_C = (PadData[1] & 64) >> 6;
                Controller_2_Start = (PadData[1] & 128) >> 7;
                Controller_2_X = (PadData[2] & 16) >> 4;
                Controller_2_Y = (PadData[2] & 32) >> 5;
                Controller_2_Z = (PadData[2] & 64) >> 6;
                Controller_2_Mode = (PadData[2] & 128) >> 7;
            }
        }
    }
}
// XXX - probably most people won't need to use this?
//Modif N - p3copy (playback player 3 while recording others)
void MoviePlayPlayer3()
{
    if (!MainMovie.TriplePlayerHack)
        return;
    char PadData[3]; //Modif
    fseek(MainMovie.File, 64 + FrameCount * 3, SEEK_SET);
    {
        fread(PadData, 3, 1, MainMovie.File);
        {
            Controller_1C_Up = (PadData[2] & 1);
            Controller_1C_Down = (PadData[2] & 2) >> 1;
            Controller_1C_Left = (PadData[2] & 4) >> 2;
            Controller_1C_Right = (PadData[2] & 8) >> 3;
            Controller_1C_A = (PadData[2] & 16) >> 4;
            Controller_1C_B = (PadData[2] & 32) >> 5;
            Controller_1C_C = (PadData[2] & 64) >> 6;
            Controller_1C_Start = (PadData[2] & 128) >> 7;
        }
    }
}

// Modif N. -- added
void CompressBoolArray(char* output, int outputBytes, const char* input, int inputBytes)
{
    int outByte = 0, bit = 0, curByte = 0;
    for (int inByte = 0; inByte < inputBytes && outByte < outputBytes; bit++)
    {
        if (bit == 8)
        {
            bit = 0;
            output[outByte++] = curByte;
            curByte = 0;
        }
        curByte |= (input[inByte++] ? 1 : 0) << bit;
    }
    if (outByte < outputBytes)
        output[outByte] = curByte;
}
void DecompressBoolArray(char* output, int outputBytes, const char* input, int inputBytes, int trueValue = 1)
{
    int outByte = 0, bit = 0;
    for (int inByte = 0; inByte < inputBytes && outByte < outputBytes; bit++)
    {
        if (bit == 8)
        {
            bit = 0;
            inByte++;
            if (inByte >= inputBytes)
                break;
        }

        output[outByte++] = (input[inByte] & (1 << bit)) ? trueValue : 0;
    }
    while (outByte < outputBytes)
        output[outByte++] = 0;
}

// the BRAM size really has to be saved in movie files to avoid desyncs since different games need different sizes.
// GMV is not very extensible so the way I do this here is a terrible hack, but it's better than nothing for now.
void EmbedBRAMSizeInTracks()
{
    int size = (BRAM_Ex_State & 0x100) ? (BRAM_Ex_Size + 2) : 1;
    played_tracks_linear[100] = size & 0x1;
    played_tracks_linear[101] = size & 0x2;
    played_tracks_linear[102] = size & 0x4;
}
extern "C" int SegaCD_Started;
void ExtractBRAMSizeFromTracks()
{
    int size = (played_tracks_linear[100] ? 0x1 : 0) | (played_tracks_linear[101] ? 0x2 : 0) | (played_tracks_linear[102] ? 0x4 : 0);
    if (size == 0 || !SegaCD_Started) return;
    if (size == 1) { BRAM_Ex_State &= 1; return; }
    BRAM_Ex_State |= 0x100; BRAM_Ex_Size = size - 2;
}

// kind of a sneaky way of adding this information to the not-really-extendable GMV format
// it won't always work if the note is really long but that's ok because it's completely non-essential information anyway
void EmbedPreloadedTracksInNote(char* note)
{
    int i;
    for (i = 0; i < 40; i++)
        if (!note[i])
            break;
    i++; // after the null
    char compressed[13];
    EmbedBRAMSizeInTracks();
    CompressBoolArray(compressed, 13, played_tracks_linear, 104);
    for (int j = 0; i < 40 && j < 13; i++, j++)
        note[i] = compressed[j];
}
void ExtractPreloadedTracksFromNote(char* note)
{
    int i, j;
    for (i = 0; i < 40; i++)
        if (!note[i])
            break;
    i++; // after the null
    char compressed[13] = { 0 };
    for (j = 0; i < 40 && j < 13; i++, j++)
        compressed[j] = note[i];
    DecompressBoolArray(played_tracks_linear, min(104, j * 8), compressed, j);
    ExtractBRAMSizeFromTracks();
    for (i = 0; i < min(100, j * 8); i++)
        if (played_tracks_linear[i])
            preloaded_tracks[i] = 2; // 2 == "force preload if MP3"
    return;
}

//Modif
void MovieRecordingStuff()
{
    if (!MainMovie.File) // can't continue if MainMovie.File is NULL (we would crash)
    {                   // this should never happen... but sometimes it happens anyway
        DialogsOpen++;
        MessageBox(HWnd, "Movie is not recording.", "Warning", MB_ICONWARNING);
        DialogsOpen--;
        MainMovie.Status = 0;
        return;
    }

    if (!MainMovie.Recorded) MainMovie.Recorded = true;
    if (track & TRACK1)
        if (/*!(GetKeyState(VK_SCROLL) || GetKeyState(VK_NUMLOCK)) || */(FrameCount > Track1_FrameCount)) Track1_FrameCount = FrameCount;
    if (track & TRACK2)
        Track2_FrameCount = FrameCount;
    if ((track & TRACK3) && MainMovie.TriplePlayerHack)
        Track3_FrameCount = FrameCount;
    if (tempflag) //Upth-Add - This part is so we only increment the number of rerecords
    {             //Upth-Add - if the user does take the opportunity to resume recording
        MainMovie.NbRerecords++;
        if (MainMovie.TriplePlayerHack)
            MainMovie.LastFrame = max(max(Track1_FrameCount, Track2_FrameCount), Track3_FrameCount);
        else
            MainMovie.LastFrame = max(Track1_FrameCount, Track2_FrameCount);
        Put_Info("Recording from current frame"); //Upth-Add - Notify that we've resumed recording
        tempflag = false; //Upth-Add - And we unset the flag so it doesn't increment rerecords everyframe
        Build_Main_Menu();
    }
    char PadData[3]; //Modif
    /*	if (GetKeyState(VK_SCROLL))
        {
        int temp[4];
        temp[0] = Controller_1_A;
        temp[1] = Controller_1_B;
        temp[2] = Controller_1_C;
        temp[3] = Controller_1_Start;
        MoviePlayPlayer1();
        Controller_1_A = temp[0];
        Controller_1_B = temp[1];
        Controller_1_C = temp[2];
        Controller_1_Start = temp[3];
        }
        else if (GetKeyState(VK_NUMLOCK))
        {
        int temp[4];
        temp[0] = Controller_1_Up;
        temp[1] = Controller_1_Down;
        temp[2] = Controller_1_Left;
        temp[3] = Controller_1_Right;
        MoviePlayPlayer1();
        Controller_1_Up = temp[0];
        Controller_1_Down = temp[1];
        Controller_1_Left = temp[2];
        Controller_1_Right = temp[3];
        }
        else
        {*/
    if (!(track & 1))
    {
        Controller_1_Up = Controller_1_Down = Controller_1_Left = Controller_1_Right = 1;
        Controller_1_A = Controller_1_B = Controller_1_C = Controller_1_Start = 1;
        Controller_1_X = Controller_1_Y = Controller_1_Z = Controller_1_Mode = 1;
        if (Track1_FrameCount >= FrameCount) MoviePlayPlayer1();
    }
    if (!(track & 2))
    {
        if (MainMovie.TriplePlayerHack)
        {
            Controller_1B_Up = Controller_1B_Down = Controller_1B_Left = Controller_1B_Right = 1;
            Controller_1B_A = Controller_1B_B = Controller_1B_B = Controller_1B_Start = 1;
            Controller_1B_X = Controller_1B_Y = Controller_1B_Z = Controller_1B_Mode = 1;
        }
        else
        {
            Controller_2_Up = Controller_2_Down = Controller_2_Left = Controller_2_Right = 1;
            Controller_2_A = Controller_2_B = Controller_2_C = Controller_2_Start = 1;
            Controller_2_X = Controller_2_Y = Controller_2_Z = Controller_2_Mode = 1;
        }
        if (Track2_FrameCount >= FrameCount) MoviePlayPlayer2();
    }
    if (MainMovie.TriplePlayerHack && !(track & 4))
    {
        Controller_1C_Up = Controller_1C_Down = Controller_1C_Left = Controller_1C_Right = 1;
        Controller_1C_A = Controller_1C_B = Controller_1C_C = Controller_1C_Start = 1;
        Controller_1C_X = Controller_1C_Y = Controller_1C_Z = Controller_1C_Mode = 1;
        if (Track3_FrameCount >= FrameCount) MoviePlayPlayer3();
    }
    //	}
    PadData[0] = Controller_1_Up | (Controller_1_Down << 1) | (Controller_1_Left << 2) | (Controller_1_Right << 3)
        | (Controller_1_A << 4) | (Controller_1_B << 5) | (Controller_1_C << 6) | (Controller_1_Start << 7);
    if (MainMovie.TriplePlayerHack)
    {
        PadData[1] = Controller_1B_Up | (Controller_1B_Down << 1) | (Controller_1B_Left << 2) | (Controller_1B_Right << 3)
            | (Controller_1B_A << 4) | (Controller_1B_B << 5) | (Controller_1B_C << 6) | (Controller_1B_Start << 7);
        PadData[2] = Controller_1C_Up | (Controller_1C_Down << 1) | (Controller_1C_Left << 2) | (Controller_1C_Right << 3)
            | (Controller_1C_A << 4) | (Controller_1C_B << 5) | (Controller_1C_C << 6) | (Controller_1C_Start << 7);
    }
    else
    {
        PadData[1] = Controller_2_Up | (Controller_2_Down << 1) | (Controller_2_Left << 2) | (Controller_2_Right << 3)
            | (Controller_2_A << 4) | (Controller_2_B << 5) | (Controller_2_C << 6) | (Controller_2_Start << 7);
        PadData[2] = Controller_1_X | (Controller_1_Y << 1) | (Controller_1_Z << 2) | (Controller_1_Mode << 3)
            | (Controller_2_X << 4) | (Controller_2_Y << 5) | (Controller_2_Z << 6) | (Controller_2_Mode << 7);
    }
    fseek(MainMovie.File, 64 + FrameCount * 3, SEEK_SET);
    fwrite(PadData, 3, 1, MainMovie.File);
    if ((track == ALL_TRACKS) || ((track == (TRACK1 | TRACK2)) && !MainMovie.TriplePlayerHack))
        MainMovie.LastFrame = FrameCount;
    else
    {
        MainMovie.LastFrame = max(max(Track1_FrameCount, Track2_FrameCount), MainMovie.LastFrame);
        if (MainMovie.TriplePlayerHack) MainMovie.LastFrame = max(MainMovie.LastFrame, Track3_FrameCount);
    }
}

void InitMovie(typeMovie * aMovie)
{
    aMovie->File = NULL;
    strcpy(aMovie->FileName, "");
    strcpy(aMovie->PhysicalFileName, "");
    aMovie->LastFrame = 0;
    aMovie->NbRerecords = 0;
    strcpy(aMovie->Note, "");
    aMovie->PlayerConfig[0] = 6;
    aMovie->PlayerConfig[1] = 3;
    aMovie->ReadOnly = 0;
    strcpy(aMovie->StateName, "");
    aMovie->Status = 0;
    aMovie->UseState = 0;
    aMovie->Version = 'A' - '0';
    aMovie->Ok = 0;
    aMovie->StateFrame = 0;
    aMovie->StateOk = 0;
    aMovie->Vfreq = (CPU_Mode) ? 1 : 0;
    aMovie->StateRequired = 0;
    aMovie->TriplePlayerHack = 0;
    aMovie->Type = TYPEGMV;
    aMovie->Recorded = false;
    aMovie->ClearSRAM = true;
}

void CopyMovie(typeMovie * MovieSrc, typeMovie * MovieDest)
{
    MovieDest->File = MovieSrc->File;
    strncpy(MovieDest->FileName, MovieSrc->FileName, 1024);
    strncpy(MovieDest->PhysicalFileName, MovieSrc->PhysicalFileName, 1024);
    MovieDest->LastFrame = MovieSrc->LastFrame;
    MovieDest->NbRerecords = MovieSrc->NbRerecords;
    strncpy(MovieDest->Note, MovieSrc->Note, 41);
    memcpy(MovieDest->PlayerConfig, MovieSrc->PlayerConfig, sizeof(MovieSrc->PlayerConfig));
    MovieDest->ReadOnly = MovieSrc->ReadOnly;
    strncpy(MovieDest->StateName, MovieSrc->StateName, 1024);
    MovieDest->Status = MovieSrc->Status;
    MovieDest->UseState = MovieSrc->UseState;
    MovieDest->Version = MovieSrc->Version;
    MovieDest->Ok = MovieSrc->Ok;
    MovieDest->StateOk = MovieSrc->StateOk;
    strncpy(MovieDest->Header, MovieSrc->Header, 17);
    MovieDest->StateFrame = MovieSrc->StateFrame;
    MovieDest->Vfreq = MovieSrc->Vfreq;
    MovieDest->TriplePlayerHack = MovieSrc->TriplePlayerHack;
    MovieDest->StateRequired = MovieSrc->StateRequired;
    MovieDest->ClearSRAM = MovieSrc->ClearSRAM;

    if (MovieDest == &MainMovie) // Modif N.
        ExtractPreloadedTracksFromNote(MovieSrc->Note);
}

// some extensions that might commonly be near movie files that almost certainly aren't movie files.
static const char* s_nonMovieExtensions[] = { "txt", "nfo", "htm", "html", "jpg", "jpeg", "png", "bmp", "gif", "mp3", "wav", "lnk", "exe", "bat", "lua", "luasav", "sav", "srm", "brm", "cfg", "wch", "gs*", "bin", "smd", "gen", "32x", "cue", "iso", "raw" };
// question: why use exclusion instead of inclusion?
// answer: because filename extensions aren't that reliable.
// if it's one of these extensions then it's probably safe to assume it's not a movie (and doing so makes things simpler and more convenient for the user),
// but if it isn't one of these then it's best to ask the user or check the file contents,
// in case it's a valid movie or a valid movie-containing archive with an unknown extension.

int GetMovieInfo(char *FileName, typeMovie *aMovie)
{
    FILE *test;
    unsigned char t;

    //UpthAdd - Save Controller Settings
    Temp_Controller_Type[0] = Controller_1_Type;
    Temp_Controller_Type[2] = Controller_1B_Type;
    Temp_Controller_Type[3] = Controller_1C_Type;
    Temp_Controller_Type[1] = Controller_2_Type;

    // use ObtainFile to support loading movies from archives
    char LogicalName[1024], PhysicalName[1024];
    strcpy(LogicalName, FileName);
    strcpy(PhysicalName, FileName);

    test = fopen(PhysicalName, "rb");
    if (test == NULL)
    {
        aMovie->Ok = 0;
        return -1;
    }

    fseek(test, 0, SEEK_END);
    if (ftell(test) < 64)
    {
        aMovie->Ok = 0;
        fclose(test);
        return -2;
    }

    aMovie->LastFrame = (ftell(test) - 64) / 3;

    fseek(test, 0, SEEK_SET);
    fread(aMovie->Header, 1, 10, test);
    aMovie->Header[10] = 0;
    if (strcmp(aMovie->Header, "Gens Movie"))
    {
        aMovie->Ok = 0;
        fclose(test);
        return -3;
    }

    fseek(test, 0, SEEK_SET);
    fread(aMovie->Header, 1, 16, test);
    aMovie->Header[17] = 0;

    fseek(test, 15, SEEK_SET);
    aMovie->Version = fgetc(test) - '0';

    fseek(test, 0, SEEK_END);
    aMovie->LastFrame = (ftell(test) - 64) / 3;

    fseek(test, 16, SEEK_SET);
    fread((char*)&aMovie->NbRerecords, sizeof(aMovie->NbRerecords), 1, test);

    aMovie->Ok = 1;

    strncpy(aMovie->FileName, LogicalName, 1024);
    strncpy(aMovie->PhysicalFileName, PhysicalName, 1024);

    if (aMovie->Version >= 9)
    {
        fseek(test, 24, SEEK_SET);
        fread(aMovie->Note, 40, 1, test);
        aMovie->Note[40] = 0;
        fseek(test, 20, SEEK_SET);
        aMovie->PlayerConfig[0] = fgetc(test) - '0';
        aMovie->PlayerConfig[1] = fgetc(test) - '0';
        if (aMovie->Version >= ('A' - '0'))
        {
            fseek(test, 22, SEEK_SET);
            t = fgetc(test);
            aMovie->Vfreq = ((t & 128) >> 7);
            aMovie->StateRequired = ((t & 64) >> 6);
            aMovie->TriplePlayerHack = ((t & 32) >> 5);
        }
        else
        {
            aMovie->Vfreq = CPU_Mode ? 1 : 0;
            aMovie->StateRequired = 0;
            aMovie->TriplePlayerHack = 0;
        }
    }
    else
    {
        strncpy(aMovie->Note, "None.  Movie version earlier than TEST9.", 40);
        aMovie->PlayerConfig[0] = 0;
        aMovie->PlayerConfig[1] = 0;
        aMovie->Vfreq = (CPU_Mode) ? 1 : 0;
    }
    fclose(test);
    return 0;
}

void GetStateInfo(char * FileName, typeMovie *aMovie)
{
    FILE *test;

    test = fopen(FileName, "rb");

    if (test == NULL)
    {
        aMovie->StateOk = 0;
        return;
    }

    fseek(test, 0, SEEK_END);

    if (ftell(test) < 0x2247C)
    {
        aMovie->StateOk = 0;
        fclose(test);
        return;
    }

    strncpy(aMovie->StateName, FileName, 1024);

    fseek(test, 0x22478, SEEK_SET);

    aMovie->StateFrame = fgetc(test);
    aMovie->StateFrame += fgetc(test) << 8;
    aMovie->StateFrame += fgetc(test) << 16;
    aMovie->StateFrame += fgetc(test) << 24;

    aMovie->StateOk = 1;
    fclose(test);
}

int OpenMovieFile(typeMovie *aMovie)
{
    if (!aMovie->Ok)
        return 0;

    // use ObtainFile to support loading movies from archives (read-only)
    char LogicalName[1024], PhysicalName[1024];
    strcpy(LogicalName, aMovie->FileName);
    strcpy(PhysicalName, aMovie->FileName);

    aMovie->File = fopen(PhysicalName, "r+b"); // so we can toggle readonly later without re-opening file
    if (!aMovie->File)
    {
        aMovie->File = fopen(PhysicalName, "rb");
        aMovie->ReadOnly = 2; // really read-only
    }
    strncpy(aMovie->PhysicalFileName, PhysicalName, 1024);
    Update_Recent_Movie(LogicalName);

    if (!aMovie->File)
        return 0;

    return 1;
}

void WriteMovieHeader(typeMovie *aMovie)
{
    unsigned char t;

    fseek(aMovie->File, 16, SEEK_SET);
    fwrite((char*)&aMovie->NbRerecords, sizeof(aMovie->NbRerecords), 1, aMovie->File);
    if (aMovie->Version >= 9)
    {
        fseek(aMovie->File, 20, SEEK_SET);
        if ((Controller_1_Type & 1) == 1)
            fputc('6', aMovie->File);
        else
            fputc('3', aMovie->File);
        if ((Controller_2_Type & 1) == 1)
            fputc('6', aMovie->File);
        else
            fputc('3', aMovie->File);
        if (aMovie->Version >= 'A' - '0')
        {
            fseek(aMovie->File, 22, SEEK_SET);
            t = fgetc(aMovie->File);
            t = (t & 127) + ((CPU_Mode) ? 128 : 0);
            t = (t & 191) + ((aMovie->StateRequired) ? 64 : 0);
            t = (t & 223) + ((aMovie->TriplePlayerHack) ? 32 : 0);
            fseek(aMovie->File, 22, SEEK_SET);
            fputc(t, aMovie->File);
        }
        fseek(aMovie->File, 24, SEEK_SET);
        EmbedPreloadedTracksInNote(aMovie->Note);
        fwrite(aMovie->Note, 40, 1, aMovie->File);
    }
}

int FlushMovieFile(typeMovie *aMovie)
{
    unsigned int MovieFileLastFrame = 0;
    char * movieData = NULL;

    if (aMovie->File == NULL)
        return 0;
    if (aMovie->ReadOnly == 0 || aMovie->Status == MOVIE_RECORDING)
    {
        WriteMovieHeader(aMovie);
        fseek(aMovie->File, 0, SEEK_END);
        MovieFileLastFrame = (ftell(aMovie->File) - 64) / 3;
        if (MovieFileLastFrame > aMovie->LastFrame)
        {
            fseek(aMovie->File, 0, SEEK_SET);
            movieData = new char[aMovie->LastFrame * 3 + 64];
            fread(movieData, aMovie->LastFrame * 3 + 64, 1, aMovie->File);
        }
    }

    fclose(aMovie->File);
    aMovie->File = NULL;

    // if necessary, truncate the frame data to match the length of the movie
    if ((MovieFileLastFrame > aMovie->LastFrame) && (aMovie->ReadOnly == 0 || aMovie->Status == MOVIE_RECORDING))
    {
        aMovie->File = fopen(aMovie->PhysicalFileName, "wb");
        if (aMovie->File)
        {
            fseek(aMovie->File, 0, SEEK_SET);
            fwrite(movieData, aMovie->LastFrame * 3 + 64, 1, aMovie->File);
            fclose(aMovie->File);
        }
    }

    delete[] movieData;
    return 1;
}

int CloseMovieFile(typeMovie *aMovie)
{
    if (!FlushMovieFile(aMovie))
        return 0;

    char status = aMovie->Status;
    InitMovie(aMovie);
    aMovie->StateFrame = status;
    //UpthAdd - Restore Controller settings
    Controller_1_Type = Temp_Controller_Type[0];
    Controller_1B_Type = Temp_Controller_Type[2];
    Controller_1C_Type = Temp_Controller_Type[3];
    Controller_2_Type = Temp_Controller_Type[1];

    return 1;
}

int BackupMovieFile(typeMovie *aMovie)
{
    char * movieData;
    FILE* Backup;

    if (aMovie->File == NULL)
        return 0;

    Put_Info(aMovie->FileName);

    strncpy(aMovie->PhysicalFileName, aMovie->FileName, 1024);
    Backup = fopen(aMovie->PhysicalFileName, "wb");

    if (Backup == NULL)
        return 0;

    fseek(aMovie->File, 0, SEEK_SET);
    movieData = new char[aMovie->LastFrame * 3 + 64];
    fread(movieData, aMovie->LastFrame * 3 + 64, 1, aMovie->File);
    fseek(Backup, 0, SEEK_SET);
    fwrite(movieData, aMovie->LastFrame * 3 + 64, 1, Backup);
    fclose(Backup);
    Backup = NULL;

    delete[] movieData;

    return 1;
}