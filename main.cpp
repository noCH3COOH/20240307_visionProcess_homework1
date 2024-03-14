#include "main.h"

using namespace std;

int main()
{
    ifstream src;
    src.open("./video/test.yuv", ios_base::in);
    if(!src.is_open())
    {
        cout << "[ERROR] 文件未打开" << endl;
        return 1;
    }

    enum PIXEL_FMT fmt;
    int tem;
    
    cout << "选择分辨率：360p (0)、480p (1)、720p (2) 、1080p (3)" << endl;
    cin >> tem;
    if(0 == tem)
        fmt = FMT_360P;
    else if(1 == tem)
        fmt = FMT_480P;
    else if(2 == tem)
        fmt = FMT_720P;
    else if(3 == tem)
        fmt = FMT_1080P;
    else
    {
        cout << "预置无该分辨率" << endl;
        return 1;
    }
    
    input_yuvData(&src, fmt);
    trans_yuv2rgb888_all(fmt);
    play_VRAM(fmt);

    src.close();
    return 0;
}

void input_yuvData(ifstream* src, enum PIXEL_FMT FMT)
{
    int pixel_max_len = pixelFmt_size[FMT][0] * pixelFmt_size[FMT][1];

    /**
     * 假设数据按 I444 格式存储
     * 
     * I444:
     * YYYYYYYY
     * YYYYYYYY
     * YYYYYYYY
     * UUUUUUUU
     * UUUUUUUU
     * UUUUUUUU
     * VVVVVVVV
     * VVVVVVVV
     * VVVVVVVV
    */
    for(int i=0; i<pixel_max_len; i++)
        VRAM[3*i] = (*src).get();    // 读取 Y 分量
    
    for(int i=0; i<pixel_max_len; i++)
        VRAM[3*i + 1] = (*src).get();    // 读取 U 分量

    for(int i=0; i<pixel_max_len; i++)
        VRAM[3*i + 2] = (*src).get();    // 读取 V 分量
}

void trans_yuv2rgb888_1p(uint8_t* src_pixel)
{
    uint8_t src_yuv_pixel[3];
    src_yuv_pixel[0] = src_pixel[0];
    src_yuv_pixel[1] = src_pixel[1] - 128;
    src_yuv_pixel[2] = src_pixel[2] - 128;

    src_pixel[0] = src_yuv_pixel[0] + (-0.00093) * src_yuv_pixel[1] + 1.401687 * src_yuv_pixel[2];
    src_pixel[1] = src_yuv_pixel[0] + (-0.3437) * src_yuv_pixel[1] + (-0.71417) * src_yuv_pixel[2];
    src_pixel[2] = src_yuv_pixel[0] + 1.77216 * src_yuv_pixel[1] + 0.00099 * src_yuv_pixel[2];
}

void trans_yuv2rgb888_all(enum PIXEL_FMT FMT)
{
    uint8_t tem[3] = {0};
    int pixel_max_len = pixelFmt_size[FMT][0] * pixelFmt_size[FMT][1];

    for(int i=0; i<pixel_max_len; i++)
    {
        // 将暂存至 RGB VRAM 的 YUV 数据读取出
        tem[0] = VRAM[3*i];
        tem[1] = VRAM[3*i + 1];
        tem[2] = VRAM[3*i + 2];

        trans_yuv2rgb888_1p(tem);

        VRAM[3*i] = tem[0];
        VRAM[3*i + 1] = tem[1];
        VRAM[3*i + 2] = tem[2];
    }
}

void play_VRAM(enum PIXEL_FMT FMT)
{
    cv::Mat play_frame = cv::Mat(cv::Size(pixelFmt_size[FMT][0], pixelFmt_size[FMT][1])
                                , CV_8UC3, VRAM, 0UL);

    cv::imshow("播放 RGB 视频", play_frame);
}

