#ifndef __MAIN_H__
#define __MAIN_H__

#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

enum PIXEL_FMT{
    FMT_360P = 0,    /* 480*360 */
    FMT_480P,     /* 640*480 */
    FMT_720P,     /* 1280*720 */
    FMT_1080P    /* 1920*1080 */
};
int pixelFmt_size[4][2] = {
    {480, 360},
    {640, 480},
    {1280, 720},
    {1920, 1080}
};

#define VRAM_MAX_W 1920
#define VRAM_MAX_H 1080
#define VRAM_MAX_COLOR_CHANNEL 3

uint8_t VRAM[(VRAM_MAX_H * VRAM_MAX_W * VRAM_MAX_COLOR_CHANNEL)] = {0};
// H * W * Channel

void input_yuvData(ifstream* src, enum PIXEL_FMT FMT);
void trans_yuv2rgb888_1p(uint8_t* src_pixel);
void trans_yuv2rgb888_all(enum PIXEL_FMT FMT);
void play_VRAM(enum PIXEL_FMT FMT);

#endif
