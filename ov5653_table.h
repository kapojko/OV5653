#ifndef OV5653_TABLE_H
#define OV5653_TABLE_H

#include <stdint.h>

#define OV5653_MODES (6)
#define OV5653_MODE_REGS (46)
#define OV5653_SHARE_REGS (104)

/* OV5653 global gain table row size */
#define OV5653_GAIN_ROWS				(97)
#define OV5653_GAIN_COLS				(3)
#define OV5653_GAIN_0DB				(96)
#define OV5653_GAIN_DOUBLE			(16)

#define OV5653_GAIN_COL_AGC			(0)
#define OV5653_GAIN_COL_REG350A		(1)
#define OV5653_GAIN_COL_REG350B		(2)

struct vin_reg_16_8 {
    uint16_t addr;
    uint8_t data;
};

extern struct vin_reg_16_8 ov5653_mode_regs[OV5653_MODES][OV5653_MODE_REGS];
extern struct vin_reg_16_8 ov5653_share_regs[OV5653_SHARE_REGS];

extern uint16_t ov5653_gains[OV5653_GAIN_ROWS][OV5653_GAIN_COLS];

#endif // OV5653_TABLE_H
