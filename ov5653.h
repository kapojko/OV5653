#ifndef OV5653_H
#define OV5653_H

// Based on https://github.com/hello/kasa/blob/master/ambarella/kernel/private/drivers/vin/sensors/omnivision_ov5653

#include <stdint.h>
#include <stdbool.h>

#define OV5653_SCCB_SLAVE_ID 0x6C

#ifndef OV5653_XCLK_MHZ
#define OV5653_XCLK_MHZ 24
#endif

enum OV5653_Format {
    OV5653_FORMAT_1280x720_60fps = 0, // 1280x720 60fps
    OV5653_FORMAT_1920x1080_30fps = 1, // 1920x1080 30fps
    OV5653_FORMAT_1600x1200_30fps = 2, // 1600x1200 30fps
    OV5653_FORMAT_2048x1536_20fps = 3, // 2048x1536 20fps
    OV5653_FORMAT_2560x1440_20fps = 4, // 2560x1440 20fps
    OV5653_FORMAT_2592x1944_15fps = 5, // 2592x1944 15fps
};

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct OV5653_Timing {
    u32 pixelclk;
    u16 line_length;
    u16 frame_length_lines;
    
    u32 line_time;
    u32 shutter_time;
    u64 vb_time;
};

// Redeclare in user code, return 0 on success, non-zero on error
extern u8 OV5653_WR_Reg(u16 reg, u8 data);
extern u8 OV5653_RD_Reg(u16 reg, u8 *data);
extern void OV5653_Delay_ms(u16 ms);

void OV5653_StartStreaming(void);
void OV5653_SwReset(void);
void OV5653_FillShareRegs(void);

void OV5653_ReadSensorID(uint16_t *pid);

int OV5653_Init(void);

void OV5653_InitTimingStruct(struct OV5653_Timing *timing, u32 pixelclk);
int OV5653_ReadHVInfo(struct OV5653_Timing *timing);
int OV5653_GetLineTime(struct OV5653_Timing *timing);

int OV5653_SetFormat(enum OV5653_Format format, struct OV5653_Timing *timing);
int OV5653_SetShutterRow(u32 row, struct OV5653_Timing *timing);
int OV5653_Shutter2Row(struct OV5653_Timing *timing);
int OV5653_SetFps(int fps, struct OV5653_Timing *timing, u16 height);

int OV5653_AGCParseVirtualIndex(u32 virtual_index, bool video_mode_720p, u32 *agc_index, u32 *summing_gain);
void OV5653_SetBinningSumming(u32 bin_sum_config);
int OV5653_SetAgcIndex(int agc_idx);
int OV5653_SetAgcAdj(int agc_index);


#endif // OV5653_H
