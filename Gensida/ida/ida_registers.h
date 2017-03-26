#pragma once

#define RC_GENERAL 1
#define RC_VDP 2
#define RC_BREAK 4

enum register_t
{
    R_D0,
    R_D1,
    R_D2,
    R_D3,
    R_D4,
    R_D5,
    R_D6,
    R_D7,

    R_A0,
    R_A1,
    R_A2,
    R_A3,
    R_A4,
    R_A5,
    R_A6,
    R_A7,

    R_PC,

    R_SR,

    R_VDP_DMA_LEN,
    R_VDP_DMA_SRC,
    R_VDP_WRITE_ADDR,

    // DX
    R_B00,
    R_B01,
    R_B02,
    R_B03,
    R_B04,
    R_B05,
    R_B06,
    R_B07,

    // AX
    R_B08,
    R_B09,
    R_B10,
    R_B11,
    R_B12,
    R_B13,
    R_B14,
    R_B15,

    // VX
    R_B16,
    R_B17,
    R_B18,
    R_B19,
    R_B20,
    R_B21,
    R_B22,
    R_B23,
    R_B24,
    R_B25,
    R_B26,
    R_B27,
    R_B28,
    R_B29,
    R_B30,
    R_B31,
    R_B32,
    R_B33,
    R_B34,
    R_B35,
    R_B36,
    R_B37,
    R_B38,
    R_B39,
    // Allow break regs
    R_B_ALLOW0,
    R_B_ALLOW1,

    R_DR00,
    R_DR01,
    R_DR02,
    R_DR03,
    R_DR04,
    R_DR05,
    R_DR06,
    R_DR07,
    R_DR08,
    R_DR09,
    R_DR10,
    R_DR11,
    R_DR12,
    R_DR13,
    R_DR14,
    R_DR15,
    R_DR16,
    R_DR17,
    R_DR18,
    R_DR19,
    R_DR20,
    R_DR21,
    R_DR22,
    R_DR23,
};
