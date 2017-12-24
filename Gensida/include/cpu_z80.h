#ifndef _CPU_Z80_H
#define _CPU_Z80_H

#ifdef __cplusplus
extern "C" {
#endif

    int Z80_Init(void);
    void Z80_Reset(void);
	void Write_To_68K_Space(int adr, int data);
	void Read_To_68K_Space(int adr);
	void Write_To_Bank(int val);

#ifdef __cplusplus
};
#endif

#endif
