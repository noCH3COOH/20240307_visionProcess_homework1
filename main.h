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

//===================== struct and enum =====================

struct VRAM_t{
    uint8_t data[(VRAM_MAX_H * VRAM_MAX_W * VRAM_MAX_COLOR_CHANNEL)];
    bool is_empty;
    std::mutex mtx;
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
struct VRAM_t VRAM2 = {{0}, true};
struct VRAM_t* VRAM_needs_to_play = nullptr;

enum PIXEL_FMT fmt;

ifstream src;

std::condition_variable cv_condition_variable;
std::mutex mtx;
std::atomic<bool> terminateProgram(false);

bool threadOneDone = false;
bool threadTwoDone = false;
bool fileEnd = false;

int pixelFmt_size[4][2] = {
    {480, 360},
    {640, 480},
    {1280, 720},
    {1920, 1080}
};

//===================== function =====================

void thread1_inputData();
void thread2_transData();
void thread3_playVideo(std::chrono::milliseconds interval);

struct VRAM_t* VRAM_sw(void);
void input_yuvData_1f(ifstream* src, enum PIXEL_FMT FMT, struct VRAM_t* VRAM);
void trans_yuv2rgb888_1p(uint8_t* src_pixel);
void trans_yuv2rgb888_1f(enum PIXEL_FMT FMT, struct VRAM_t* VRAM);
void play_VRAM(enum PIXEL_FMT FMT, struct VRAM_t* VRAM);

#endif
