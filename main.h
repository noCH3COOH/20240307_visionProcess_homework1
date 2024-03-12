#ifndef __MAIN_H__
#define __MAIN_H__

enum PIXEL_FMT{
    FMT_360P = 1,    /* 480*360 */
    FMT_480P,     /* 640*480 */
    FMT_720P,     /* 1280*720 */
    FMT_1080P    /* 1920*1080 */
};

#define VRAM_MAX_W 1920
#define VRAM_MAX_H 1080
#define VRAM_MAX_COLOR_CHANNEL 3

uint8_t VRAM[(VRAM_MAX_W * VRAM_MAX_H * VRAM_MAX_COLOR_CHANNEL)] = {0};

#define p_VRAM_CHANNEL_R VRAM
#define p_VRAM_CHANNEL_G &(VRAM[(VRAM_MAX_W * VRAM_MAX_H)])
#define p_VRAM_CHANNEL_B &(VRAM[(VRAM_MAX_W * VRAM_MAX_H * 2)])

void yuv2rgb888_1p(uint8_t* src_pixel);
void yuv2rgb888_all(enum PIXEL_FMT FMT);
void play_VRAM(void);

#endif
