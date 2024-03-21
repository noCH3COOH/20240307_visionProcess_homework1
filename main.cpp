#include "main.h"

//===================== 主函数 =====================

int main()
{
    int tem;
    int frame_rate;

    string src_file_path = "";
    string src_file = "";
    string src_info = "";

    ifstream info;

    uni_log.open("log.txt", ios_base::out);

    msg_toLog = "[INFO] 第 " + FLAG_DEBUG;
    msg_toLog += " 次调试\n";
    make_log(msg_toLog);

    msg_toLog = "[INFO] 输入 YUV 文件路径：\n";
    make_log(msg_toLog);
    cin >> src_file_path;
    if (src_file_path.empty())
    {
        msg_toLog = "[ERROR] 文件路径为空\n";
        make_log(msg_toLog);

        src_file_path = "./video";
    }

    msg_toLog = "[INFO] 使用文件路径：" + string(src_file_path) + '\n';
    make_log(msg_toLog);

    src_file = src_file_path + string("/test.yuv");
    src.open(src_file.c_str(), ios_base::in);
    if (!src.is_open())
    {
        msg_toLog = "[ERROR] 文件未打开\n";
        make_log(msg_toLog);
        return 1;
    }
    msg_toLog = "[INFO] 文件已打开：" + src_file + '\n';
    make_log(msg_toLog);

    src_info = src_file_path + string("/videoinfo.txt");
    info.open(src_info.c_str(), ios_base::in);
    if (!info.is_open())
    {
        msg_toLog = "[ERROR] 未找到视频信息文件\n";
        make_log(msg_toLog);

        msg_toLog = "[INFO] 选择分辨率：360p (0)、480p (1)、720p (2) 、1080p (3)\n";
        make_log(msg_toLog);
        cin >> tem;

        msg_toLog = "[INFO] 输入帧率：\n";
        make_log(msg_toLog);
        cin >> frame_rate;
    }
    else
    {
        tem = info.get();

        info.get();

        info >> frame_rate;

        msg_toLog = "[INFO] 已找到视频信息文件";
        msg_toLog += " 分辨率:" + (char)tem;
        msg_toLog += " 帧率" + frame_rate + '\n';
        make_log(msg_toLog);
    }

    if ('0' == tem)
        fmt = FMT_360P;
    else if ('1' == tem)
        fmt = FMT_480P;
    else if ('2' == tem)
        fmt = FMT_720P;
    else if ('3' == tem)
        fmt = FMT_1080P;
    else
    {
        msg_toLog = "[ERROR] 预置无该分辨率\n";
        make_log(msg_toLog);
        return 1;
    }

    cv::namedWindow(WINDOW_NAME, 1);
    cv::namedWindow(COLOR_WINDOW_NAME, 1);

    // 滑动条
    cv::createTrackbar("matrix[0][0]", COLOR_WINDOW_NAME, &tem_matrix[0][0]
                        , 100, on_tacker_matrix);
    cv::createTrackbar("matrix[0][1]", COLOR_WINDOW_NAME, &tem_matrix[0][1]
                        , 100, on_tacker_matrix);
    cv::createTrackbar("matrix[0][2]", COLOR_WINDOW_NAME, &tem_matrix[0][2]
                        , 100, on_tacker_matrix);
    cv::createTrackbar("matrix[1][0]", COLOR_WINDOW_NAME, &tem_matrix[1][0]
                        , 100, on_tacker_matrix);
    cv::createTrackbar("matrix[1][1]", COLOR_WINDOW_NAME, &tem_matrix[1][1]
                        , 100, on_tacker_matrix);
    cv::createTrackbar("matrix[1][2]", COLOR_WINDOW_NAME, &tem_matrix[1][2]
                        , 100, on_tacker_matrix);
    cv::createTrackbar("matrix[2][0]", COLOR_WINDOW_NAME, &tem_matrix[2][0]
                        , 100, on_tacker_matrix);
    cv::createTrackbar("matrix[2][1]", COLOR_WINDOW_NAME, &tem_matrix[2][1]
                        , 100, on_tacker_matrix);
    cv::createTrackbar("matrix[2][2]", COLOR_WINDOW_NAME, &tem_matrix[2][2]
                        , 100, on_tacker_matrix);

    on_tacker_matrix(tem_matrix[0][0], 0);
    on_tacker_matrix(tem_matrix[0][1], 0);
    on_tacker_matrix(tem_matrix[0][2], 0);
    on_tacker_matrix(tem_matrix[1][0], 0);
    on_tacker_matrix(tem_matrix[1][1], 0);
    on_tacker_matrix(tem_matrix[1][2], 0);
    on_tacker_matrix(tem_matrix[2][0], 0);
    on_tacker_matrix(tem_matrix[2][1], 0);
    on_tacker_matrix(tem_matrix[2][2], 0);

    interval = 1000 / frame_rate;

    while((!userEnd) || (!fileEnd))
    {

        do
        {
            VRAM_toProcess = VRAM_sw();
            if (nullptr == VRAM_toProcess)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                msg_toLog = "[ERROR] 显存申请失败\n";
                make_log(msg_toLog);
            }
            else
            {
                break;
            }
        } while (true);
    
        input_yuvData_1f(fmt, VRAM_toProcess);
        trans_yuv2rgb888_1f(fmt, VRAM_toProcess);
        play_VRAM(fmt, VRAM_toProcess);
        if(userEnd || fileEnd)
            break;

        //std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
    
    cv::destroyAllWindows();

#ifdef FLAG_DEBUG
    for(int i=0; i<3; i++)
        for(int j=0; j<3; j++)
        {
            msg_toLog = "";
            msg_toLog += "matrix[" + std::to_string(i) + "][" + std::to_string(j) + "]: ";
            msg_toLog += std::to_string(matrix_yuv2rgb[i][j]);
            msg_toLog += "( " + std::to_string(tem_matrix[i][j]) + " )\n";

            make_log(msg_toLog);
        }
#endif

    src.close();
    info.close();
    uni_log.close();
    return 0;
}

//===================== 回调函数层 =====================

void on_tacker_matrix(int, void*)
{
    matrix_yuv2rgb[0][0] = 0.9 + (tem_matrix[0][0] - 50) / 50.0;
    matrix_yuv2rgb[0][1] = 0 + (tem_matrix[0][1] - 50) / 50.0;
    matrix_yuv2rgb[0][2] = 2 + (tem_matrix[0][2] - 50) / 50.0;
    matrix_yuv2rgb[1][0] = 1 + (tem_matrix[1][0] - 50) / 50.0;
    matrix_yuv2rgb[1][1] = -0.02 + (tem_matrix[1][1] - 50) / 50.0;
    matrix_yuv2rgb[1][2] = -1 + (tem_matrix[1][2] - 50) / 50.0;
    matrix_yuv2rgb[2][0] = 1 + (tem_matrix[2][0] - 50) / 50.0;
    matrix_yuv2rgb[2][1] = 2 + (tem_matrix[2][1] - 50) / 50.0;
    matrix_yuv2rgb[2][2] = 0 + (tem_matrix[2][2] - 50) / 50.0;
}

//===================== 实用函数层 =====================

void make_log(string message)
{
    cout << message << endl;
    uni_log.write(message.c_str(), strlen(message.c_str()));
}

/**
 * @brief 双缓冲切换
 * @return 返回当前空闲的 VRAM
 */
struct VRAM_t *VRAM_sw(void)
{
    if (VRAM1.is_empty)
    {
        VRAM1.is_empty = false;
        msg_toLog = "[INFO] 申请到 VRAM1\n";
        make_log(msg_toLog);
        return &VRAM1;
    }
    else
    {
        msg_toLog = "[ERROR] VRAM 已满\n";
        make_log(msg_toLog);
        return nullptr;
    }
}

/**
 * @brief 读取一帧 YUV 数据
 * @param src YUV 文件流
 * @param FMT 视频分辨率
 * @param VRAM VRAM 数据
 */
void input_yuvData_1f(enum PIXEL_FMT FMT, struct VRAM_t *VRAM)
{
    time_t start_time = clock();

    int pixel_max_len = pixelFmt_size[FMT][0] * pixelFmt_size[FMT][1];

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
        for (int i = 0; i < pixel_max_len; i++)
        {
            (*VRAM).data[3 * i] = src.get(); // 读取 Y 分量
        }

        for (int i = 0; i < pixel_max_len; i += 2)
        {
            if (1 == ((int)(i / pixelFmt_size[FMT][0]) % 2))
            {
                i += pixelFmt_size[FMT][0] - 2;
                continue;
            }

            (*VRAM).data[3 * i + 1] = src.get();                                             // 读取 U 分量
            (*VRAM).data[3 * (i + pixelFmt_size[FMT][0]) + 1] = (*VRAM).data[3 * i + 1];       // 重复填充 U 分量
            (*VRAM).data[3 * (i + 1) + 1] = (*VRAM).data[3 * i + 1];                         // 重复填充 U 分量
            (*VRAM).data[3 * (i + 1 + pixelFmt_size[FMT][0]) + 1] = (*VRAM).data[3 * i + 1]; // 重复填充 U 分量
        }

        for (int i = 0; i < pixel_max_len; i += 2)
        {
            if (1 == ((i / pixelFmt_size[FMT][0]) % 2))
            {
                i += pixelFmt_size[FMT][0] - 2;
                continue;
            }

            (*VRAM).data[3 * i + 2] = src.get();                                             // 读取 V 分量
            (*VRAM).data[3 * (i + pixelFmt_size[FMT][0]) + 2] = (*VRAM).data[3 * i + 2];       // 重复填充 V 分量
            (*VRAM).data[3 * (i + 1) + 2] = (*VRAM).data[3 * i + 2];                         // 重复填充 V 分量
            (*VRAM).data[3 * (i + 1 + pixelFmt_size[FMT][0]) + 2] = (*VRAM).data[3 * i + 2]; // 重复填充 V 分量
        }
    }
    else if (src.fail())
    {
        msg_toLog = "[ERROR] 文件已读取完毕\n";
        make_log(msg_toLog);
        (*VRAM).is_empty = true; // 释放 VRAM
        fileEnd = true;
    }
    else
    {
        msg_toLog = "[ERROR] 文件读取异常\n";
        make_log(msg_toLog);
        (*VRAM).is_empty = true; // 释放 VRAM
        fileEnd = true;
    }

    time_t end_time = clock();
    time_t cost_time = end_time - start_time;
    msg_toLog = "[INFO] input_yuvData_1f() 用时" + std::to_string((double)cost_time/CLOCKS_PER_SEC) + "s\n";
    make_log(msg_toLog);
}

/**
 * @brief YUV 转 RGB888 1 个像素点
 * @param src_pixel YUV 数据
 */
void trans_yuv2rgb888_1p(uint8_t *src_pixel)
{
    uint8_t src_yuv_pixel[3];

    // src_yuv_pixel[0] = src_pixel[0];
    // src_yuv_pixel[1] = src_pixel[1] - 128;
    // src_yuv_pixel[2] = src_pixel[2] - 128;
    // src_pixel[0] = src_yuv_pixel[0] + ((360 * src_yuv_pixel[2]) >> 8);
    // src_pixel[1] = src_yuv_pixel[0] - (( 88* src_yuv_pixel[1] + 184 * src_yuv_pixel[2] ) >> 8);
    // src_pixel[2] = src_yuv_pixel[0] + ((455 * src_yuv_pixel[2]) >> 8);

    src_yuv_pixel[0] = src_pixel[0];
    src_yuv_pixel[1] = src_pixel[1] - 128;
    src_yuv_pixel[2] = src_pixel[2] - 128;

    // src_pixel[0] = src_yuv_pixel[0] + 0 * src_yuv_pixel[1] + 1.4746 * src_yuv_pixel[2];
    // src_pixel[1] = src_yuv_pixel[0] - 0.1645 * src_yuv_pixel[1] - 0.5713 * src_yuv_pixel[2];
    // src_pixel[2] = src_yuv_pixel[0] + 1.8814 * src_yuv_pixel[1] - 0.0001 * src_yuv_pixel[2];
    
    src_pixel[0] = matrix_yuv2rgb[0][0] * src_yuv_pixel[0] 
                 + matrix_yuv2rgb[0][1] * src_yuv_pixel[1]
                 + matrix_yuv2rgb[0][2] * src_yuv_pixel[2];
    src_pixel[1] = matrix_yuv2rgb[1][0] * src_yuv_pixel[0] 
                 + matrix_yuv2rgb[1][1] * src_yuv_pixel[1]
                 + matrix_yuv2rgb[1][2] * src_yuv_pixel[2];
    src_pixel[2] = matrix_yuv2rgb[2][0] * src_yuv_pixel[0] 
                 + matrix_yuv2rgb[2][1] * src_yuv_pixel[1]
                 + matrix_yuv2rgb[2][2] * src_yuv_pixel[2];

    src_pixel[0] = RGB_Check(src_pixel[0]);
    src_pixel[1] = RGB_Check(src_pixel[1]);
    src_pixel[2] = RGB_Check(src_pixel[2]);
}

/**
 * @brief YUV 转 RGB888
 * @param FMT 视频分辨率
 * @param VRAM VRAM 数据
 */
void trans_yuv2rgb888_1f(enum PIXEL_FMT FMT, struct VRAM_t *VRAM)
{
    time_t start_time = clock();

    uint8_t tem[3] = {0};
    int pixel_max_len = pixelFmt_size[FMT][0] * pixelFmt_size[FMT][1];

    for (int i = 0; i < pixel_max_len; i++)
    {
        // 将暂存至 RGB VRAM 的 YUV 数据读取出
        tem[0] = (*VRAM).data[3 * i];
        tem[1] = (*VRAM).data[3 * i + 1];
        tem[2] = (*VRAM).data[3 * i + 2];

        trans_yuv2rgb888_1p(tem);

        // 转换为 OpenCV 使用的 BGR 格式
        (*VRAM).data[3 * i] = tem[2];
        (*VRAM).data[3 * i + 1] = tem[1];
        (*VRAM).data[3 * i + 2] = tem[0];
    }

    time_t end_time = clock();
    time_t cost_time = end_time - start_time;
    msg_toLog = "[INFO] trans_yuv2rgb888_1f() 用时" + std::to_string((double)cost_time/CLOCKS_PER_SEC) + "s\n";
    make_log(msg_toLog);
}

/**
 * @brief 播放 VRAM 数据
 * @param FMT 视频分辨率
 * @param VRAM VRAM 数据
 */
void play_VRAM(enum PIXEL_FMT FMT, struct VRAM_t *VRAM)
{
    time_t start_time = clock();

    cv::Mat play_frame = cv::Mat(cv::Size(pixelFmt_size[FMT][0], pixelFmt_size[FMT][1]), CV_8UC3, (*VRAM).data, 0UL);

    cv::imshow(WINDOW_NAME, play_frame);

    int key = cv::waitKey(interval);
    if ('q' == key || 'Q' == key)
        userEnd = true;

    (*VRAM).is_empty = true; // 释放 VRAM

    time_t end_time = clock();
    time_t cost_time = end_time - start_time;
    msg_toLog = "[INFO] play_VRAM() 用时" + std::to_string((double)cost_time/CLOCKS_PER_SEC) + "s\n";
    make_log(msg_toLog);
}
