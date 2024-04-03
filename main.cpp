//===================== usage =====================
/**
 * 请在 main.cpp 同级目录下创建 video 文件夹，
 * 并在里边创建 videoinfo.txt 文件，内容如下：
 * 
 *  (文件名) (宽) (高) (帧率)
 * 
 * 例如：
 * 
 * test 1920 1080 30
 * 
 * 请将 YUV 视频文件放入 video 文件夹内
 * 随后便可以运行程序
*/

//===================== include =====================

#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <time.h>
#include <thread>
#include <chrono>

//===================== namespace =====================
using namespace std;

//===================== define =====================

#define VRAM_MAX_W 3840
#define VRAM_MAX_H 2160    // 4K
#define VRAM_MAX_COLOR_CHANNEL 3

//#define FLAG_DEBUG 1

#define WINDOW_NAME "RGB 视频"
#define COLOR_WINDOW_NAME "调色板"

#define Value_Check(value, min, max) ((value >= min) ? (value <= max ? value : max) : min) 

#define get_now() std::chrono::steady_clock::now()
#define await_time(interval)  (get_now() + std::chrono::duration<double, milli>(interval))

#define compare_x1_approachto_x2(x1, x2) ( (x1 < (x2*0.9) ? false : ( x1 > (x2*1.1) ? false : true )))

//===================== struct and enum =====================

struct VRAM_t{
    uint8_t raw[(int)(VRAM_MAX_H * VRAM_MAX_W * 1.5)];
    uint8_t data[(VRAM_MAX_H * VRAM_MAX_W * VRAM_MAX_COLOR_CHANNEL)];
    bool is_empty;
    std::mutex vram_mtx;
};

struct PID_t{
    const float Kp;
    const float Ki;
    const float Kd;

    double e[3];
};

//===================== global variable =====================

// H * W * Channel
// 双缓冲
struct VRAM_t VRAM1 = {{0}, {0}, true};

struct PID_t pid_para = {0.25, 2, 1, {0,0,0}};

std::ifstream src;
std::ofstream uni_log;

string msg_toLog;
string msg_frame;

bool fileEnd = false;
bool userEnd = false;

double interval;

int fps = 0;
int video_size[4] = {0};    // col, row, col*row, col*row*1.25

//===================== function =====================

void make_log(string message);
void input_yuvData_1f(int* FMT, struct VRAM_t* VRAM);
void play_VRAM(int* FMT, struct VRAM_t* VRAM);

double pid_control(double last_value, double target_value);

//===================== 主函数 =====================

int main()
{
    int tem = 1;
    int frame_rate;
    int average_fps = 0;
    int fps_count = 0;

    char tem_char;

    string src_file_path = "";
    string src_filename = "";
    string src_info = "";

    std::ifstream info;

    auto main_start_time = get_now();

    uni_log.open("log.txt", ios_base::out);

#ifdef FLAG_DEBUG
    msg_toLog = "[INFO] 第 " + FLAG_DEBUG;
    msg_toLog += " 次调试\n";
    make_log(msg_toLog);
#endif

    msg_toLog = "[INFO] 是否使用默认文件夹内视频(./video/*.yuv) [y/n]：";
    make_log(msg_toLog);
    cin >> tem_char;
    if('n' == tem_char)
    {
        msg_toLog = "[INFO] 请输入路径";
        make_log(msg_toLog);

        cin >> src_file_path;
    }
    else if('y' == tem_char)
    {
        src_file_path = "./video";
    }
    else
    {
        msg_toLog = "[ERROR] 本程序不提供第三种选项:)";
        make_log(msg_toLog);

        return -1;
    }

    msg_toLog = "[INFO] 使用文件路径：" + string(src_file_path);
    make_log(msg_toLog);

    src_info = src_file_path + string("/videoinfo.txt");
    info.open(src_info.c_str(), ios_base::in);
    if (!info.is_open())
    {
        msg_toLog = "[ERROR] 未找到视频信息文件";
        make_log(msg_toLog);

        msg_toLog = "[INFO] 输入分辨率 width ：";
        make_log(msg_toLog);
        cin >> video_size[0];

        msg_toLog = "[INFO] 输入分辨率 height ：";
        make_log(msg_toLog);
        cin >> video_size[1];

        msg_toLog = "[INFO] 输入帧率：";
        make_log(msg_toLog);
        cin >> frame_rate;
    }
    else
    {
        info >> src_filename;

        info >> video_size[0];
        info >> video_size[1];

        info >> frame_rate;

        msg_toLog = "[INFO] 已找到视频信息文件";
        msg_toLog += " 分辨率:" + std::to_string(video_size[0]) + "x" + std::to_string(video_size[1]);
        msg_toLog += " 帧率" + frame_rate;
        make_log(msg_toLog);
    }

    make_log("");

    video_size[2] = video_size[0] * video_size[1];
    video_size[3] = (int)(video_size[0] * video_size[1] * 1.25);

    src_filename = src_file_path + '/' + src_filename + ".yuv";
    src.open(src_filename.c_str(), ios_base::in);
    if (!src.is_open())
    {
        msg_toLog = "[ERROR] 文件未打开" + src_filename;
        make_log(msg_toLog);
        return 1;
    }
    msg_toLog = "[INFO] 文件已打开：" + src_filename;
    make_log(msg_toLog);

    cv::namedWindow(WINDOW_NAME, 1);

    interval = 1000.0 / frame_rate;
    std::chrono::duration<double> cost_time;

    while((!userEnd) || (!fileEnd))
    {
        main_start_time = get_now();
    
        input_yuvData_1f(video_size, &VRAM1);
        play_VRAM(video_size, &VRAM1);
        if(userEnd || fileEnd)
            break;

        std::this_thread::sleep_until(await_time(interval));
        
        cost_time = get_now() - main_start_time;
        fps = (1 / cost_time.count());
        interval = cost_time.count();

        interval = pid_control(interval, (1000.0/frame_rate));

#ifdef FLAG_DEBUG
        msg_toLog = "[INFO] 更换等待时间：" + std::to_string(interval);
        make_log(msg_toLog);
#endif

        average_fps += fps;
        fps_count += 1;
    }

    average_fps /= fps_count;
    
    cv::destroyAllWindows();

    src.close();
    info.close();
    uni_log.close();

    msg_toLog = "[INFO] 平均帧数：" + std::to_string(average_fps);
    make_log(msg_toLog);

    return 0;
}

//===================== 实用函数层 =====================

void make_log(string message)
{
    message += '\n';

    cout << message;
    uni_log.write((message).c_str(), strlen(message.c_str()));
}

double pid_control(double last_value, double target_value)
{
    pid_para.e[2] = pid_para.e[1];
    pid_para.e[1] = pid_para.e[0];

    pid_para.e[0] = last_value - target_value;

    last_value += (pid_para.Kp * (pid_para.e[0] - pid_para.e[1]));
    last_value += (pid_para.Ki * pid_para.e[0]);
    last_value += (pid_para.Kd * (pid_para.e[0] - 2*pid_para.e[1] + pid_para.e[2]));

    return Value_Check(last_value, 0, 100);
}

/**
 * @brief 读取一帧 YUV 数据
 * @param src YUV 文件流
 * @param FMT 视频分辨率
 * @param VRAM VRAM 数据
 */
void input_yuvData_1f(int* FMT, struct VRAM_t *VRAM)
{
#ifdef FLAG_DEBUG
    time_t start_time = clock();
    time_t end_time;
    time_t cost_time;
#endif

    if((*VRAM).is_empty)
        (*VRAM).is_empty = false; // VRAM 不为空

    /**
     * 假设数据按 I420 格式存储
     *
     * I420:
     * YYYYYYYY
     * YYYYYYYY
     * YYYYYYYY
     * YYYYYYYY
     * UUUU
     * UUUU
     * VVVV
     * VVVV
     */

    if (!src.eof())
    {
        src.read((char*)(*VRAM).raw, (FMT[0] * FMT[1] * 1.5));

        float src_yuv_pixel[3];
        int half_col = FMT[0] / 2;

        for (int i = 0, now_row = 0, now_col = 0; i < FMT[2]; i++)
        {
            now_row = int(i / FMT[0]);
            now_col = int(i % FMT[0]);

            src_yuv_pixel[0] = (*VRAM).raw[i];
            src_yuv_pixel[1] = (*VRAM).raw[FMT[2] + int(now_row / 2) * half_col + int(now_col / 2)] - 128;
            src_yuv_pixel[2] = (*VRAM).raw[FMT[3] + int(now_row / 2) * half_col + int(now_col / 2)] - 128;

            // yuv to rgb
            (*VRAM).data[3 * i + 2] = Value_Check((0.875 * src_yuv_pixel[0]) + src_yuv_pixel[2] * 2, 0, 255);
            (*VRAM).data[3 * i + 1] = Value_Check(src_yuv_pixel[0] - src_yuv_pixel[2], 0, 255);
            (*VRAM).data[3 * i] = Value_Check(src_yuv_pixel[0] + src_yuv_pixel[1] * 2, 0, 255);
        }
    }
    else if (src.fail())
    {
        msg_toLog = "[SUCCESS] 文件已读取完毕";
        make_log(msg_toLog);
        (*VRAM).is_empty = true; // 释放 VRAM
        fileEnd = true;
    }
    else
    {
        msg_toLog = "[ERROR] 文件读取异常";
        make_log(msg_toLog);
        (*VRAM).is_empty = true; // 释放 VRAM
        fileEnd = true;
    }

#ifdef FLAG_DEBUG
    end_time = clock();
    cost_time = end_time - start_time;
    msg_toLog = "[INFO] input_yuvData_1f() 用时" + std::to_string((double)cost_time/CLOCKS_PER_SEC) + "s\n";
    make_log(msg_toLog);
#endif
}

/**
 * @brief 播放 VRAM 数据
 * @param FMT 视频分辨率
 * @param VRAM VRAM 数据
 */
void play_VRAM(int* FMT, struct VRAM_t *VRAM)
{
#ifdef FLAG_DEBUG
    time_t start_time = clock();
#endif

    cv::Mat play_frame = cv::Mat(cv::Size(FMT[0], FMT[1]), CV_8UC3, (*VRAM).data, 0UL);
    cv::blur(play_frame, play_frame, cv::Size(3, 3));

    cv::putText(play_frame, msg_frame, cv::Point(20,15), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, cv::Scalar(0, 0, 255));
    cv::imshow(WINDOW_NAME, play_frame);

    int key = cv::waitKey(1);
    if ('q' == key || 'Q' == key)
        userEnd = true;

    (*VRAM).is_empty = true; // 释放 VRAM

#ifdef FLAG_DEBUG
    msg_toLog = "[INFO] play_VRAM() 用时" + std::to_string((double)(clock() - start_time)/CLOCKS_PER_SEC) + "s\n";
    make_log(msg_toLog);
#endif
    
    msg_frame = "";
    msg_frame += "[INFO] fps: " + std::to_string(fps);
    make_log(msg_frame);
    msg_frame += "  resolution: " + std::to_string(FMT[0]) + 'x' + std::to_string(FMT[1]);
}
