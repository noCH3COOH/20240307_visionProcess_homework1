#ifndef __MAIN_H__
#define __MAIN_H__

#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <time.h>

//===================== namespace =====================
using namespace std;

//===================== define =====================

#define VRAM_MAX_W 1920
#define VRAM_MAX_H 1080
#define VRAM_MAX_COLOR_CHANNEL 3

#define FLAG_DEBUG 2

#define WINDOW_NAME "RGB 视频"
#define COLOR_WINDOW_NAME "调色板"

#define RGB_Check(value) ((value >= 0) ? (value <= 255 ? value : 255) : 0) 

//===================== struct and enum =====================

struct VRAM_t{
    uint8_t raw[(int)(VRAM_MAX_H * VRAM_MAX_W * 1.5)];
    uint8_t data[(VRAM_MAX_H * VRAM_MAX_W * VRAM_MAX_COLOR_CHANNEL)];
    bool is_empty;
    std::mutex vram_mtx;
};

enum PIXEL_FMT{
    FMT_360P = 0,    /* 480*360 */
    FMT_480P,     /* 640*480 */
    FMT_720P,     /* 1280*720 */
    FMT_1080P    /* 1920*1080 */
};

//===================== global variable =====================

// H * W * Channel
// 双缓冲
struct VRAM_t VRAM1 = {{0}, {0}, true};
struct VRAM_t* VRAM_toProcess = nullptr;

enum PIXEL_FMT fmt;

ifstream src;
ofstream uni_log;

time_t main_start_time;

string msg_toLog;
string msg_frame;

bool fileEnd = false;
bool userEnd = false;
bool debug1 = false;
bool debug2 = false;

int interval;
int pixelFmt_size[4][4] = {
    {480, 360, 172800, 216000},    // col, row, col*row, col*row*1.25
    {640, 480, 307200, 384000},
    {1280, 720, 921600, 1152000},
    {1920, 1080, 2073600, 2592000}
};

extern const float matrix_yuv2rgb[3][3];

//===================== function =====================

struct VRAM_t* VRAM_sw(void);
void make_log(string message);
void input_yuvData_1f(enum PIXEL_FMT FMT, struct VRAM_t* VRAM);
void trans_yuv2rgb888_1p(uint8_t* src_pixel);
void play_VRAM(enum PIXEL_FMT FMT, struct VRAM_t* VRAM);

#endif
