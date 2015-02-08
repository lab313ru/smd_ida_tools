#ifndef __GEN_H__
#define __GEN_H__

struct gen_hdr {
	unsigned char CopyRights[32];
	unsigned char DomesticName[48];
	unsigned char OverseasName[48];
	unsigned char ProductCode[14];
	unsigned short CheckSum;
	unsigned char Peripherials[16];
	unsigned int RomStart;
	unsigned int RomEnd;
	unsigned int RamStart;
	unsigned int RamEnd;
	unsigned char SramCode[12];
	unsigned char ModemCode[12];
	unsigned char Reserved[40];
	unsigned char CountryCode[16];
};

struct gen_vect {
	union {
		struct {
			unsigned int SSP;
			unsigned int Reset;
			unsigned int BusErr;
			unsigned int AdrErr;
			unsigned int InvOpCode;
			unsigned int DivBy0;
			unsigned int Check;
			unsigned int TrapV;
			unsigned int GPF;
			unsigned int Trace;
			unsigned int Reserv0;
			unsigned int Reserv1;
			unsigned int Reserv2;
			unsigned int Reserv3;
			unsigned int Reserv4;
			unsigned int BadInt;
			unsigned int Reserv10;
			unsigned int Reserv11;
			unsigned int Reserv12;
			unsigned int Reserv13;
			unsigned int Reserv14;
			unsigned int Reserv15;
			unsigned int Reserv16;
			unsigned int Reserv17;
			unsigned int BadIRQ;
			unsigned int IRQ1;
			unsigned int EXT;
			unsigned int IRQ3;
			unsigned int HBLANK;
			unsigned int IRQ5;
			unsigned int VBLANK;
			unsigned int IRQ7;
			unsigned int Trap0;
			unsigned int Trap1;
			unsigned int Trap2;
			unsigned int Trap3;
			unsigned int Trap4;
			unsigned int Trap5;
			unsigned int Trap6;
			unsigned int Trap7;
			unsigned int Trap8;
			unsigned int Trap9;
			unsigned int Trap10;
			unsigned int Trap11;
			unsigned int Trap12;
			unsigned int Trap13;
			unsigned int Trap14;
			unsigned int Trap15;
			unsigned int Reserv30;
			unsigned int Reserv31;
			unsigned int Reserv32;
			unsigned int Reserv33;
			unsigned int Reserv34;
			unsigned int Reserv35;
			unsigned int Reserv36;
			unsigned int Reserv37;
			unsigned int Reserv38;
			unsigned int Reserv39;
			unsigned int Reserv3A;
			unsigned int Reserv3B;
			unsigned int Reserv3C;
			unsigned int Reserv3D;
			unsigned int Reserv3E;
			unsigned int Reserv3F;
		};
		unsigned int vectors[64];
	};
};

idaman loader_t ida_module_data LDSC;

#endif
