#ifndef __CDDA_MP3_H__
#define __CDDA_MP3_H__

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

    int MP3_Init(void);
    void MP3_Reset(void);
    int MP3_Get_Bitrate(FILE *f);
    int MP3_Length_LBA(FILE *f);
    int MP3_Find_Frame(FILE *f, int pos_wanted);
    int MP3_Play(int track, int lba_pos, int async);
    int MP3_Update(char *buf, int *rate, int *channel, unsigned int length_dest); // returns number of bytes written to buf
    void MP3_Test(FILE* f);
    FILE* GetMP3TrackFile(int trackIndex, int* pDontLeaveFileOpen, int* pWhere_read);
    void MP3_CancelAllPreloading(void);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* __CDDA_MP3_H__ */
