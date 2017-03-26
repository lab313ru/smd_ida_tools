#ifndef _H_MOVIE_
#define _H_MOVIE_

#include "stdio.h"

#define MAX_RECENT_MOVIES 15
#define MOVIE_PLAYING	1
#define MOVIE_RECORDING 2
#define MOVIE_FINISHED  3
#define TYPEGMV 0
#define TYPEGM2 1
#define TYPEGM2TEXT 2
#define TRACK1 1
#define TRACK2 2
#define TRACK3 4
#define TRACK1_2 (TRACK1 | TRACK2)
#define TRACK1_3 (TRACK1 | TRACK3)
#define TRACK2_3 (TRACK2 | TRACK3)
#define ALL_TRACKS (TRACK1 | TRACK2 | TRACK3)

extern long unsigned int FrameCount;
extern long unsigned int LagCount, LagCountPersistent;
extern long unsigned int Track1_FrameCount;
extern long unsigned int Track2_FrameCount;
extern long unsigned int Track3_FrameCount;
extern char Recent_Movie[MAX_RECENT_MOVIES][1024];
extern char Movie_Dir[1024];
extern char track;
void MovieRecordingStuff();
void MoviePlayingStuff();

struct typeMovie
{
    int Status;
    FILE *File;
    int ReadOnly;
    unsigned int LastFrame;
    char FileName[1024];
    int UseState;
    char StateName[1024];
    unsigned int StateFrame;
    char Header[17];
    unsigned int NbRerecords;
    char Note[41];
    int PlayerConfig[8];
    int Version;
    int Ok;
    int StateOk;
    int Vfreq;		//' 0 =60Hz  1=50Hz
    int StateRequired; // ' 0= No savestate required 1=Savestate required
    int TriplePlayerHack; // 0=No triple player hack 1=yes
    unsigned char Type;
    bool Recorded;
    bool ClearSRAM;
    char PhysicalFileName[1024];
};

extern bool AutoCloseMovie; //Upth-Add - So these flags
extern bool Def_Read_Only;  //Upth-Add - are externally accessible
extern bool UseMovieStates; //Upth-Add - save.h doesn't like bools
void InitMovie(typeMovie * aMovie);
void CopyMovie(typeMovie *MovieSrc, typeMovie *MovieDest);
int GetMovieInfo(char *FileName, typeMovie *aMovie);
void GetStateInfo(char * FileName, typeMovie *aMovie);
void WriteMovieHeader(typeMovie *aMovie);
int CloseMovieFile(typeMovie *aMovie);
int FlushMovieFile(typeMovie *aMovie); // same as CloseMovieFile but doesn't clear the info in the movie struct
int OpenMovieFile(typeMovie *aMovie);
int BackupMovieFile(typeMovie *aMovie);

extern typeMovie MainMovie;

#endif
