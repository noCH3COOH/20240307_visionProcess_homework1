#ifndef __MAIN_H__
#define __MAIN_H__

#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

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
struct VRAM_t VRAM1 = {{0}, true};
struct VRAM_t* VRAM_toProcess = nullptr;

enum PIXEL_FMT fmt;

ifstream src;
ofstream uni_log;

string msg_toLog;

bool fileEnd = false;
bool userEnd = false;
bool debug1 = false;
bool debug2 = false;

int interval;
int pixelFmt_size[4][2] = {
    {480, 360},
    {640, 480},
    {1280, 720},
    {1920, 1080}
};

float matrix_yuv2rgb[3][3] = {
    {0.9, 0, 2},
    {1, -0.02, -1},
    {1, 2, 0}
};

int tem_matrix[3][3] = {
    {
        50, 50, 50
    },
    {
        50, 50, 50
    },
    {
        50, 50, 50
    }
};

//===================== function =====================

void on_tacker_matrix(int, void*);

void thread1_inputData();
void thread2_transData(std::chrono::milliseconds interval);
void thread3_playVideo(std::chrono::milliseconds interval);

struct VRAM_t* VRAM_sw(void);
void make_log(string message);
void input_yuvData_1f(enum PIXEL_FMT FMT, struct VRAM_t* VRAM);
void trans_yuv2rgb888_1p(uint8_t* src_pixel);
void trans_yuv2rgb888_1f(enum PIXEL_FMT FMT, struct VRAM_t* VRAM);
void play_VRAM(enum PIXEL_FMT FMT, struct VRAM_t* VRAM);

#endif
