#include "main.h"
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    ifstream src;
    src.open("./video/test.yuv");
    if(!src.is_open())
    {
        cout << "[ERROR] 文件未打开" << endl;
        return 1;
    }
    
    

    return 0;
}


void yuv2rgb888_1p(uint8_t* src_pixel)
{
    uint8_t src_yuv_pixel[3];
    src_yuv_pixel[0] = src_pixel[0];
    src_yuv_pixel[1] = src_pixel[1] - 128;
    src_yuv_pixel[2] = src_pixel[2] - 128;

    src_pixel[0] = src_yuv_pixel[0] + (-0.00093) * src_yuv_pixel[1] + 1.401687 * src_yuv_pixel[2];
    src_pixel[1] = src_yuv_pixel[0] + (-0.3437) * src_yuv_pixel[1] + (-0.71417) * src_yuv_pixel[2];
    src_pixel[2] = src_yuv_pixel[0] + 1.77216 * src_yuv_pixel[1] + 0.00099 * src_yuv_pixel[2];
}

void yuv2rgb888_all(enum PIXEL_FMT FMT)
{

}

void play_VRAM(void)
{

}

