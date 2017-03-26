#ifdef __cplusplus
extern "C" {
#endif

    extern unsigned int Controller_1_Type;
    extern unsigned int Controller_1_State;
    extern unsigned int Controller_1_COM;
    extern unsigned int Controller_1_Counter;
    extern unsigned int Controller_1_Delay;
    extern unsigned int Controller_1_Up;
    extern unsigned int Controller_1_Down;
    extern unsigned int Controller_1_Left;
    extern unsigned int Controller_1_Right;
    extern unsigned int Controller_1_Start;
    extern unsigned int Controller_1_Mode;
    extern unsigned int Controller_1_A;
    extern unsigned int Controller_1_B;
    extern unsigned int Controller_1_C;
    extern unsigned int Controller_1_X;
    extern unsigned int Controller_1_Y;
    extern unsigned int Controller_1_Z;

    extern unsigned int Controller_1B_Type;
    extern unsigned int Controller_1B_State;
    extern unsigned int Controller_1B_COM;
    extern unsigned int Controller_1B_Counter;
    extern unsigned int Controller_1B_Delay;
    extern unsigned int Controller_1B_Up;
    extern unsigned int Controller_1B_Down;
    extern unsigned int Controller_1B_Left;
    extern unsigned int Controller_1B_Right;
    extern unsigned int Controller_1B_Start;
    extern unsigned int Controller_1B_Mode;
    extern unsigned int Controller_1B_A;
    extern unsigned int Controller_1B_B;
    extern unsigned int Controller_1B_C;
    extern unsigned int Controller_1B_X;
    extern unsigned int Controller_1B_Y;
    extern unsigned int Controller_1B_Z;

    extern unsigned int Controller_1C_Type;
    extern unsigned int Controller_1C_State;
    extern unsigned int Controller_1C_COM;
    extern unsigned int Controller_1C_Counter;
    extern unsigned int Controller_1C_Delay;
    extern unsigned int Controller_1C_Up;
    extern unsigned int Controller_1C_Down;
    extern unsigned int Controller_1C_Left;
    extern unsigned int Controller_1C_Right;
    extern unsigned int Controller_1C_Start;
    extern unsigned int Controller_1C_Mode;
    extern unsigned int Controller_1C_A;
    extern unsigned int Controller_1C_B;
    extern unsigned int Controller_1C_C;
    extern unsigned int Controller_1C_X;
    extern unsigned int Controller_1C_Y;
    extern unsigned int Controller_1C_Z;

    extern unsigned int Controller_1D_Type;
    extern unsigned int Controller_1D_State;
    extern unsigned int Controller_1D_COM;
    extern unsigned int Controller_1D_Counter;
    extern unsigned int Controller_1D_Delay;
    extern unsigned int Controller_1D_Up;
    extern unsigned int Controller_1D_Down;
    extern unsigned int Controller_1D_Left;
    extern unsigned int Controller_1D_Right;
    extern unsigned int Controller_1D_Start;
    extern unsigned int Controller_1D_Mode;
    extern unsigned int Controller_1D_A;
    extern unsigned int Controller_1D_B;
    extern unsigned int Controller_1D_C;
    extern unsigned int Controller_1D_X;
    extern unsigned int Controller_1D_Y;
    extern unsigned int Controller_1D_Z;

    extern unsigned int Controller_2_Type;
    extern unsigned int Controller_2_State;
    extern unsigned int Controller_2_COM;
    extern unsigned int Controller_2_Counter;
    extern unsigned int Controller_2_Delay;
    extern unsigned int Controller_2_Up;
    extern unsigned int Controller_2_Down;
    extern unsigned int Controller_2_Left;
    extern unsigned int Controller_2_Right;
    extern unsigned int Controller_2_Start;
    extern unsigned int Controller_2_Mode;
    extern unsigned int Controller_2_A;
    extern unsigned int Controller_2_B;
    extern unsigned int Controller_2_C;
    extern unsigned int Controller_2_X;
    extern unsigned int Controller_2_Y;
    extern unsigned int Controller_2_Z;

    extern unsigned int Controller_2B_Type;
    extern unsigned int Controller_2B_State;
    extern unsigned int Controller_2B_COM;
    extern unsigned int Controller_2B_Counter;
    extern unsigned int Controller_2B_Delay;
    extern unsigned int Controller_2B_Up;
    extern unsigned int Controller_2B_Down;
    extern unsigned int Controller_2B_Left;
    extern unsigned int Controller_2B_Right;
    extern unsigned int Controller_2B_Start;
    extern unsigned int Controller_2B_Mode;
    extern unsigned int Controller_2B_A;
    extern unsigned int Controller_2B_B;
    extern unsigned int Controller_2B_C;
    extern unsigned int Controller_2B_X;
    extern unsigned int Controller_2B_Y;
    extern unsigned int Controller_2B_Z;

    extern unsigned int Controller_2C_Type;
    extern unsigned int Controller_2C_State;
    extern unsigned int Controller_2C_COM;
    extern unsigned int Controller_2C_Counter;
    extern unsigned int Controller_2C_Delay;
    extern unsigned int Controller_2C_Up;
    extern unsigned int Controller_2C_Down;
    extern unsigned int Controller_2C_Left;
    extern unsigned int Controller_2C_Right;
    extern unsigned int Controller_2C_Start;
    extern unsigned int Controller_2C_Mode;
    extern unsigned int Controller_2C_A;
    extern unsigned int Controller_2C_B;
    extern unsigned int Controller_2C_C;
    extern unsigned int Controller_2C_X;
    extern unsigned int Controller_2C_Y;
    extern unsigned int Controller_2C_Z;

    extern unsigned int Controller_2D_Type;
    extern unsigned int Controller_2D_State;
    extern unsigned int Controller_2D_COM;
    extern unsigned int Controller_2D_Counter;
    extern unsigned int Controller_2D_Delay;
    extern unsigned int Controller_2D_Up;
    extern unsigned int Controller_2D_Down;
    extern unsigned int Controller_2D_Left;
    extern unsigned int Controller_2D_Right;
    extern unsigned int Controller_2D_Start;
    extern unsigned int Controller_2D_Mode;
    extern unsigned int Controller_2D_A;
    extern unsigned int Controller_2D_B;
    extern unsigned int Controller_2D_C;
    extern unsigned int Controller_2D_X;
    extern unsigned int Controller_2D_Y;
    extern unsigned int Controller_2D_Z;

    extern unsigned char Lag_Frame;

    unsigned char RD_Controller_1(void);
    unsigned char RD_Controller_2(void);
    unsigned char WR_Controller_1(unsigned char data);
    unsigned char WR_Controller_2(unsigned char data);
    void Fix_Controllers(void);
    void Make_IO_Table(void);

#ifdef __cplusplus
};
#endif
