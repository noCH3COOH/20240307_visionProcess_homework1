#include "main.h"

//===================== 主函数 =====================

int main()
{
    int tem;
    int frame_rate;
    
    std::string src_file_path = "";

    ifstream info;

    cout << "输入 YUV 文件路径：" << endl;
    cin >> src_file_path;
    if(src_file_path.empty())
    {
        cout << "[ERROR] 文件路径为空" << endl;
        src_file_path = "./video";
        cout << "使用默认文件路径：" << src_file_path << endl;
    }
    
    src.open((src_file_path + std::string("/test.yuv")).c_str(), ios_base::in);
    if(!src.is_open())
    {
        cout << "[ERROR] 文件未打开" << endl;
        return 1;
    }
    
    info.open((src_file_path + std::string("/videoinfo.txt")).c_str(), ios_base::in);
    if(!info.is_open())
    {
        cout << "[ERROR] 未找到视频信息文件" << endl;
        
        cout << "选择分辨率：360p (0)、480p (1)、720p (2) 、1080p (3)" << endl;
        cin >> tem;
    
        cout << "输入帧率：" << endl;
        cin >> frame_rate;
    }
    else
    {
        tem = info.get();

        info.get();

        info >> frame_rate;
    }

    if('0' == tem)
        fmt = FMT_360P;
    else if('1' == tem)
        fmt = FMT_480P;
    else if('2' == tem)
        fmt = FMT_720P;
    else if('3' == tem)
        fmt = FMT_1080P;
    else
    {
        cout << "预置无该分辨率" << endl;
        return 1;
    }

    std::chrono::milliseconds interval(1000 / frame_rate);

    // 创建并启动定时器线程
    std::thread timer(thread3_playVideo, interval);
    
    std::thread t1(thread1_inputData);
    std::thread t2(thread2_transData);

    while(!terminateProgram) 
    {
        if (std::cin.peek() == 'q') 
        {
            char c;
            std::cin >> c; // 清除输入缓冲区中的 'q'

            std::unique_lock<std::mutex> lck(mtx);
            terminateProgram = true; // 设置终止标志 
            cv_condition_variable.notify_all(); // 通知所有线程终止
            cout << "用户终止程序" << endl;
            break; // 退出输入循环
        }
        
        if(fileEnd)
        {
            std::unique_lock<std::mutex> lck(mtx);
            cv_condition_variable.wait(lck, []{ return (bool)terminateProgram; }); // 等待程序结束
            cv_condition_variable.notify_all(); // 通知所有线程终止
            cout << "播放完成，程序结束" << endl;
            break; // 退出输入循环
        }
    }

    t1.join();
    t2.join();
    timer.detach();

    src.close();
    return 0;
}

//===================== 线程层 =====================

/**
 * @brief 输入数据线程
*/
void thread1_inputData() 
{
    while(!terminateProgram) 
    {
        struct VRAM_t* VRAM = nullptr;
        do{
            VRAM = VRAM_sw();
        }while(nullptr == VRAM);

        std::unique_lock<std::mutex> lck((*VRAM).mtx);
        std::cout << "[INFO] 线程一: 开始工作...\n";
        
        input_yuvData_1f(&src, fmt, VRAM);

        std::cout << "[INFO] 线程一: 完成工作.\n";
        threadOneDone = true;

        VRAM_needs_to_play = VRAM;    // 通知线程二处理 VRAM 数据
        lck.unlock();

        cv_condition_variable.notify_all();  // 通知其他线程

        cv_condition_variable.wait(lck, []{ return threadTwoDone; }); // 等待线程二完成
        threadTwoDone = false; // 重置标志位
    }
}

/**
 * @brief 数据转换线程
*/
void thread2_transData() 
{
    while(!terminateProgram) 
    {
        std::unique_lock<std::mutex> lck((*VRAM_needs_to_play).mtx);
        cv_condition_variable.wait(lck, []{ return threadOneDone; }); // 等待线程一完成
        
        std::cout << "[INFO] 线程二: 开始工作...\n";
        
        trans_yuv2rgb888_1f(fmt, VRAM_needs_to_play);

        std::cout << "[INFO] 线程二: 完成工作.\n";

        threadTwoDone = true;
        cv_condition_variable.notify_all();  // 通知其他线程
        lck.unlock();
        
        cv_condition_variable.wait(lck, []{ return threadOneDone; }); // 等待线程一再次完成
        threadOneDone = false; // 重置标志位
    }
}

/**
 * @brief 视频播放线程
*/
void thread3_playVideo(std::chrono::milliseconds interval) 
{
    while(!terminateProgram) 
    {
        std::this_thread::sleep_for(interval);    

        std::unique_lock<std::mutex> lck((*VRAM_needs_to_play).mtx);
        cv_condition_variable.wait(lck, []{ return threadTwoDone; }); // 等待线程二完成

        std::cout << "[INFO] 线程三: 开始工作...\n";

        play_VRAM(fmt, VRAM_needs_to_play);

        std::cout << "[INFO] 线程三: 完成工作.\n";

        cv_condition_variable.notify_all();  // 通知其他线程
        if(fileEnd)
            terminateProgram = true;    // 通知程序结束

        lck.unlock();
    }
}

//===================== 实用函数层 =====================

/**
 * @brief 双缓冲切换
 * @return 返回当前空闲的 VRAM
*/
struct VRAM_t* VRAM_sw(void)
{
    if(VRAM1.is_empty)
    {
        VRAM1.is_empty = false;
        return &VRAM1;
    }
    else if(VRAM2.is_empty)
    {
        VRAM2.is_empty = false;
        return &VRAM2;
    }
    else
    {
        cout << "[ERROR] VRAM 已满" << endl;
        return nullptr;
    }
}

/**
 * @brief 读取一帧 YUV 数据
 * @param src YUV 文件流
 * @param FMT 视频分辨率
 * @param VRAM VRAM 数据
*/
void input_yuvData_1f(ifstream* src, enum PIXEL_FMT FMT, struct VRAM_t* VRAM)
{
    int pixel_max_len = pixelFmt_size[FMT][0] * pixelFmt_size[FMT][1];
    
    (*VRAM).is_empty = false;    // VRAM 不为空

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

    if(!(*src).eof())
    {

        for(int i=0; i<pixel_max_len; i++)
            (*VRAM).data[3*i] = (*src).get();    // 读取 Y 分量
        
        for(int i=0; i<pixel_max_len; i+=2)
        {
            (*VRAM).data[3*i + 1] = (*src).get();    // 读取 U 分量
            (*VRAM).data[3*i + 1 + pixelFmt_size[FMT][0]] = (*VRAM).data[3*i + 1];    // 重复填充 U 分量
            (*VRAM).data[3*(i+1) + 1] = (*VRAM).data[3*i + 1];    // 重复填充 U 分量
            (*VRAM).data[3*(i+1) + 1 + pixelFmt_size[FMT][0]] = (*VRAM).data[3*i + 1];    // 重复填充 U 分量
        }
    
        for(int i=0; i<pixel_max_len; i+=2)
        {
            (*VRAM).data[3*i + 2] = (*src).get();    // 读取 V 分量
            (*VRAM).data[3*i + 2 + pixelFmt_size[FMT][0]] = (*VRAM).data[3*i + 2];    // 重复填充 V 分量
            (*VRAM).data[3*(i+1) + 2] = (*VRAM).data[3*i + 2];    // 重复填充 V 分量
            (*VRAM).data[3*(i+1) + 2 + pixelFmt_size[FMT][0]] = (*VRAM).data[3*i + 2];    // 重复填充 V 分量
        }
    }
    else if((*src).fail())
    {
        cout << "[ERROR] 文件已读取完毕" << endl;
        (*VRAM).is_empty = true;    // 释放 VRAM
        fileEnd = true;
    }
    else
    {
        cout << "[ERROR] 文件读取异常" << endl;
        (*VRAM).is_empty = true;    // 释放 VRAM
        fileEnd = true;
    
    }
}

/**
 * @brief YUV 转 RGB888 1 个像素点
 * @param src_pixel YUV 数据
*/
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

/**
 * @brief YUV 转 RGB888
 * @param FMT 视频分辨率
 * @param VRAM VRAM 数据
*/
void trans_yuv2rgb888_1f(enum PIXEL_FMT FMT, struct VRAM_t* VRAM)
{
    uint8_t tem[3] = {0};
    int pixel_max_len = pixelFmt_size[FMT][0] * pixelFmt_size[FMT][1];

    for(int i=0; i<pixel_max_len; i++)
    {
        // 将暂存至 RGB VRAM 的 YUV 数据读取出
        tem[0] = (*VRAM).data[3*i];
        tem[1] = (*VRAM).data[3*i + 1];
        tem[2] = (*VRAM).data[3*i + 2];

        trans_yuv2rgb888_1p(tem);

        (*VRAM).data[3*i] = tem[0];
        (*VRAM).data[3*i + 1] = tem[1];
        (*VRAM).data[3*i + 2] = tem[2];
    }
}

/**
 * @brief 播放 VRAM 数据
 * @param FMT 视频分辨率
 * @param VRAM VRAM 数据
*/
void play_VRAM(enum PIXEL_FMT FMT, struct VRAM_t* VRAM)
{
    cv::Mat play_frame = cv::Mat(cv::Size(pixelFmt_size[FMT][0], pixelFmt_size[FMT][1])
                                , CV_8UC3, (*VRAM).data, 0UL);

    cv::imshow("播放 RGB 视频", play_frame);
    cv::waitKey(0);

    (*VRAM).is_empty = true;    // 释放 VRAM
}

