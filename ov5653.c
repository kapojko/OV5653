// Based on https://github.com/hello/kasa/blob/master/ambarella/kernel/private/drivers/vin/sensors/omnivision_ov5653/ov5653.c

/*
 * Filename : ov5653.c
 *
 * History:
 *    2014/08/18 - [Hao Zeng] Create
 *
 *
 * Copyright (c) 2015 Ambarella, Inc.
 *
 * This file and its contents ("Software") are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella, Inc. and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any portion thereof except pursuant to a signed license agreement
 * or nondisclosure agreement with Ambarella, Inc. or its authorized affiliates.
 * In the absence of such an agreement, you agree to promptly notify and return
 * this Software to Ambarella, Inc.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AMBARELLA, INC. OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "ov5653.h"
#include "ov5653_reg.h"
#include "ov5653_table.h"

 // FIXME: DIV64_CLOSEST
#define DIV64_CLOSEST(x, y) ((x) / (y))

 // MODULE_PARM_DESC(bayer_pattern, "set bayer pattern: 0:RG, 1:BG, 2:GR, 3:GB, 255:default");

 // static int ov5653_set_vin_mode(struct vin_device *vdev, struct vin_video_format *format)
 // {
 // 	struct vin_device_config ov5653_config;

 // 	memset(&ov5653_config, 0, sizeof (ov5653_config));

 // 	ov5653_config.interface_type = SENSOR_PARALLEL_LVCMOS;
 // 	ov5653_config.sync_mode = SENSOR_SYNC_MODE_MASTER;
 // 	ov5653_config.input_mode = SENSOR_RGB_1PIX;

 // 	ov5653_config.plvcmos_cfg.vs_hs_polarity = SENSOR_VS_HIGH | SENSOR_HS_HIGH;
 // 	ov5653_config.plvcmos_cfg.data_edge = SENSOR_DATA_RISING_EDGE;
 // 	ov5653_config.plvcmos_cfg.paralle_sync_type = SENSOR_PARALLEL_SYNC_601;

 // 	ov5653_config.cap_win.x = format->def_start_x;
 // 	ov5653_config.cap_win.y = format->def_start_y;
 // 	ov5653_config.cap_win.width = format->def_width;
 // 	ov5653_config.cap_win.height = format->def_height;

 // 	ov5653_config.sensor_id = GENERIC_SENSOR;
 // 	ov5653_config.input_format = AMBA_VIN_INPUT_FORMAT_RGB_RAW;
 // 	ov5653_config.bayer_pattern = format->bayer_pattern;
 // 	ov5653_config.video_format = format->format;
 // 	ov5653_config.bit_resolution = format->bits;

 // 	return ambarella_set_vin_config(&ov5653_config);
 // }

void OV5653_StartStreaming(void)
{
    OV5653_WR_Reg(OV5653_SYSTEM_CTRL0, 0x02);
}

void OV5653_SwReset(void)
{
    u8 reset_value = 0xFF;
    OV5653_WR_Reg(OV5653_SYSTEM_RESET00, reset_value);
    OV5653_WR_Reg(OV5653_SYSTEM_RESET01, reset_value);
    OV5653_WR_Reg(OV5653_SYSTEM_RESET02, reset_value);
    OV5653_WR_Reg(OV5653_SYSTEM_RESET03, reset_value);
    OV5653_Delay_ms(10);
}

void OV5653_FillShareRegs(void)
{
    struct vin_reg_16_8 *regs;
    int i, regs_num;

    /* fill common registers */
    regs = ov5653_share_regs;
    regs_num = OV5653_SHARE_REGS;

    for (i = 0; i < regs_num; i++)
        OV5653_WR_Reg(regs[i].addr, regs[i].data);
}

int OV5653_ReadSensorID(uint16_t *pid) {
    // Initialize to 0xFF so a failed I2C read (data buffer untouched)
    // yields 0xFFFF instead of uninitialized stack garbage
    u8 pidh = 0xFF, pidl = 0xFF;
    if (OV5653_RD_Reg(OV5653_CHIP_ID_H, &pidh) != 0)
        return -1;
    if (OV5653_RD_Reg(OV5653_CHIP_ID_L, &pidl) != 0)
        return -1;
    *pid = ((u16)pidh << 8) | pidl;
    return 0;
}

int OV5653_Init(void)
{
    OV5653_SwReset();

    /* query sensor id */
    u16 sensorId;
    if (OV5653_ReadSensorID(&sensorId) != 0) {
        return -1;
    }

    // Check sensor id value
    if (sensorId != OV5653_CHIP_ID) {
        return -1;
    }

    // Fill share registers
    OV5653_FillShareRegs();

    return 0;
}

#if 0
int OV5653_SetPll(int pll_idx) {

    return 0;
}
#endif

void OV5653_InitTimingStruct(struct OV5653_Timing *timing, u32 pixelclk) {
    timing->pixelclk = pixelclk;
}

int OV5653_ReadHVInfo(struct OV5653_Timing *timing)
{
    u8 val_high, val_low;

    OV5653_RD_Reg(OV5653_TIMING_HTS_H, &val_high);
    OV5653_RD_Reg(OV5653_TIMING_HTS_L, &val_low);
    timing->line_length = ((u16)val_high << 8) + val_low;
    if (!timing->line_length) {
        return -1;
    }

    OV5653_RD_Reg(OV5653_TIMING_VTS_H, &val_high);
    OV5653_RD_Reg(OV5653_TIMING_VTS_L, &val_low);
    timing->frame_length_lines = ((u16)val_high << 8) + val_low;

    return 0;
}

int OV5653_GetLineTime(struct OV5653_Timing *timing)
{
    u64 h_clks;

    h_clks = (u64)timing->line_length * 512000000ull;
    h_clks = DIV64_CLOSEST(h_clks, timing->pixelclk);

    timing->line_time = (u32)h_clks;

    return 0;
}

int OV5653_SetFormat(enum OV5653_Format format, struct OV5653_Timing *timing)
{
    int rval;
    struct vin_reg_16_8 *regs;
    int i, regs_num;

    if ((unsigned)format > (unsigned)OV5653_FORMAT_2592x1944_15fps) {
        return -1;
    }

    regs = ov5653_mode_regs[(int)format];
    regs_num = OV5653_MODE_REGS;

    for (i = 0; i < regs_num; i++)
        OV5653_WR_Reg(regs[i].addr, regs[i].data);

    rval = OV5653_ReadHVInfo(timing);
    if (rval < 0)
        return rval;

    OV5653_GetLineTime(timing);

    /* Enable Streaming */
    OV5653_StartStreaming();

    /* communiate with IAV */
    // rval = ov5653_set_vin_mode(format);
    // if (rval < 0)
    // 	return rval;

    return 0;
}

int OV5653_SetShutterRow(u32 row, struct OV5653_Timing *timing)
{
    u64 exposure_lines;
    u32 num_line, min_line, max_line;

    num_line = row;

    /* FIXME: shutter width: 0 ~ (Frame format(V) - 3) */
    min_line = 0;
    max_line = timing->frame_length_lines - 3;

    // num_line = clamp(num_line, min_line, max_line);
    if (num_line < min_line)
        num_line = min_line;
    else if (num_line > max_line)
        num_line = max_line;

    num_line <<= 4;
    OV5653_WR_Reg(OV5653_LONG_EXPO_H, (num_line >> 16) & 0x0F);
    OV5653_WR_Reg(OV5653_LONG_EXPO_M, (num_line >> 8) & 0xFF);
    OV5653_WR_Reg(OV5653_LONG_EXPO_L, num_line & 0xFF);

    num_line >>= 4;
    exposure_lines = num_line;
    exposure_lines = exposure_lines * (u64)timing->line_length * 512000000llu;
    exposure_lines = DIV64_CLOSEST(exposure_lines, timing->pixelclk);

    timing->shutter_time = (u32)exposure_lines;
    // vin_debug("shutter_time:%d, row:%d\n", vdev->shutter_time, num_line);

    return 0;
}

int OV5653_Shutter2Row(struct OV5653_Timing *timing)
{
    u64 exposure_lines;
    int rval = 0;

    /* for fast boot, it may call set shutter time directly, so we must read line length/frame line */
    if (!timing->line_length) {
        rval = OV5653_ReadHVInfo(timing);
        if (rval < 0)
            return rval;
    }

    exposure_lines = ((u64)timing->shutter_time) * (u64)timing->pixelclk;
    exposure_lines = DIV64_CLOSEST(exposure_lines, timing->line_length);
    exposure_lines = DIV64_CLOSEST(exposure_lines, 512000000llu);

    timing->shutter_time = exposure_lines;

    return rval;
}

int OV5653_SetFps(int fps, struct OV5653_Timing *timing, u16 height)
{
    u64 v_lines, vb_time;

    v_lines = fps * (u64)timing->pixelclk;
    v_lines = DIV64_CLOSEST(v_lines, timing->line_length);
    v_lines = DIV64_CLOSEST(v_lines, 512000000llu);

    OV5653_WR_Reg(OV5653_TIMING_VTS_H, (v_lines >> 8) & 0xFF);
    OV5653_WR_Reg(OV5653_TIMING_VTS_L, (v_lines >> 0) & 0xFF);

    timing->frame_length_lines = (u32)v_lines;

    vb_time = timing->line_length * (u64)(v_lines - height) * 1000000000llu;
    vb_time = DIV64_CLOSEST(vb_time, timing->pixelclk);
    timing->vb_time = vb_time;

    return 0;
}

int OV5653_AGCParseVirtualIndex(u32 virtual_index, bool video_mode_720p, u32 *agc_index, u32 *summing_gain)
{
    if (video_mode_720p) {
        u32 index_0dB = OV5653_GAIN_0DB;
        u32 index_6dB = OV5653_GAIN_0DB - OV5653_GAIN_DOUBLE * 1;
        u32 index_12dB = OV5653_GAIN_0DB - OV5653_GAIN_DOUBLE * 2;

        if ((index_0dB >= virtual_index) && (virtual_index > index_6dB)) { /* 0dB <= gain < 6dB */
            *agc_index = virtual_index;
            *summing_gain = 1;
        } else if ((index_6dB >= virtual_index) && (virtual_index > index_12dB)) { /* 6dB <= gain < 12dB */
            *agc_index = virtual_index + OV5653_GAIN_DOUBLE * 1;	/* AGC(dB) = Virtual(dB) - 6dB */
            *summing_gain = 2;
        } else { /* 12 dB <= gain */
            *agc_index = virtual_index + OV5653_GAIN_DOUBLE * 2;	/* AGC(dB) = Virtual(dB) - 12dB */
            *summing_gain = 4;
        }
    } else {
        *agc_index = virtual_index;
        *summing_gain = 0;
    }

    // vin_debug("ov5653_agc_index: %d, virtual_index: %d, summing_gain: %d\n", *agc_index, *summing_gain);

    return 0;
}

void OV5653_SetBinningSumming(u32 bin_sum_config) /* toggle between 1x, 2x, 4x sum */
{
    u8 reg_0x3613;
    u8 reg_0x3621;

    OV5653_RD_Reg(0x3613, &reg_0x3613);
    OV5653_RD_Reg(0x3621, &reg_0x3621);

    switch (bin_sum_config) {
    case 0:	/* do nothing */
        break;

    case 1:
        reg_0x3621 |= 0x40;  /* Reg0x3621[6]=1 for H-binning off: 1x <H-skip> */
        reg_0x3613 = 0x44;   /* 1x-gain */
        break;

    case 2:
        reg_0x3621 &= ~0x40; /* Reg0x3621[6]=0 for H-binning sum on: 2x <H-bin sum> */
        reg_0x3613 = 0x44;   /* 1x-gain */
        break;

    case 4:
        reg_0x3621 &= ~0x40; /* Reg0x3621[6]=0 for H-binning sum on: 2x <H-bin sum> */
        reg_0x3613 = 0xC4;   /* 2x-gain */
        break;

    default:
        break;
    }

    OV5653_WR_Reg(0x3613, reg_0x3613);
    OV5653_WR_Reg(0x3621, reg_0x3621);
}

int OV5653_SetAgcIndex(int agc_idx)
{
    u32 virtual_gain_index, agc_index = 0, summing_gain = 0;

    virtual_gain_index = agc_idx;

    if (virtual_gain_index > OV5653_GAIN_0DB) {
        // vin_warn("agc index %d exceeds maximum %d\n", agc_idx, OV5653_GAIN_0DB);
        virtual_gain_index = OV5653_GAIN_0DB;
    }

    virtual_gain_index = OV5653_GAIN_0DB - virtual_gain_index;

    OV5653_WR_Reg(OV5653_SRM_GRUP_ACCESS, 0x00);
    OV5653_SetBinningSumming(summing_gain);
    OV5653_AGCParseVirtualIndex(virtual_gain_index, true, &agc_index, &summing_gain); // FIXME: video_mode_720p
    OV5653_WR_Reg(OV5653_AGC_ADJ_H, ov5653_gains[agc_index][OV5653_GAIN_COL_REG350A]);
    OV5653_WR_Reg(OV5653_AGC_ADJ_L, ov5653_gains[agc_index][OV5653_GAIN_COL_REG350B]);
    OV5653_WR_Reg(OV5653_SRM_GRUP_ACCESS, 0x10);
    OV5653_WR_Reg(OV5653_SRM_GRUP_ACCESS, 0xA0);

    return 0;
}

int OV5653_SetAgcAdj(int agc_index) {
    OV5653_WR_Reg(OV5653_AGC_ADJ_H, ov5653_gains[agc_index][OV5653_GAIN_COL_REG350A]);
    OV5653_WR_Reg(OV5653_AGC_ADJ_L, ov5653_gains[agc_index][OV5653_GAIN_COL_REG350B]);

    return 0;
}

int OV5653_SetMirrorMode(bool mirror_hor, bool mirror_ver, bool video_mode_720p)
{
    u32 reg3621, reg505a, reg505b, reg3827, reg3818;

    if (mirror_hor && mirror_ver) {
        if (video_mode_720p) {
            reg3621 = 0xbf;
            reg3818 = 0xa1;
        } else {
            reg3621 = 0x3f;
            reg3818 = 0xa0;
        }
        reg505a = 0x00;
        reg505b = 0x12;
        reg3827 = 0x0b;
        // bayer_pattern = VINDEV_BAYER_PATTERN_BG;
    } else if (mirror_hor) {
        if (video_mode_720p) {
            reg3621 = 0xbf;
            reg3818 = 0x81;
        } else {
            reg3621 = 0x3f;
            reg3818 = 0x80;
        }
        reg505a = 0x00;
        reg505b = 0x12;
        reg3827 = 0x0c;
        // bayer_pattern = VINDEV_BAYER_PATTERN_BG;
    } else if (mirror_ver) {
        if (video_mode_720p) {
            reg3621 = 0xaf;
            reg3818 = 0xe1;
        } else {
            reg3621 = 0x2f;
            reg3818 = 0xe0;
        }
        reg505a = 0x0a;
        reg505b = 0x2e;
        reg3827 = 0x0b;
        // bayer_pattern = VINDEV_BAYER_PATTERN_BG;
    } else {
        if (video_mode_720p) {
            reg3621 = 0xaf;
            reg3818 = 0xc1;
        } else {
            reg3621 = 0x2f;
            reg3818 = 0xc0;
        }
        reg505a = 0x0a;
        reg505b = 0x2e;
        reg3827 = 0x0c;
        // bayer_pattern = VINDEV_BAYER_PATTERN_BG;
    }

    OV5653_WR_Reg(OV5653_ARRAY_CONTROL, reg3621);
    OV5653_WR_Reg(0x505a, reg505a);
    OV5653_WR_Reg(0x505b, reg505b);
    OV5653_WR_Reg(0x3827, reg3827);
    OV5653_WR_Reg(OV5653_TIMING_TC_REG_18, reg3818);

    // if (mirror_mode->bayer_pattern == VINDEV_BAYER_PATTERN_AUTO)
    //     mirror_mode->bayer_pattern = bayer_pattern;

    return 0;
}
