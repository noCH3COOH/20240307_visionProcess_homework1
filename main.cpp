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
    if(src_file_path.empty())
    {
        msg_toLog = "[ERROR] 文件路径为空\n";
        make_log(msg_toLog);

        src_file_path = "./video";
    }

    msg_toLog = "[INFO] 使用文件路径：" + string(src_file_path) + '\n';
    make_log(msg_toLog);
    
    src_file = src_file_path + string("/test.yuv");
    src.open(src_file.c_str(), ios_base::in);
    if(!src.is_open())
    {
        msg_toLog =  "[ERROR] 文件未打开\n";
        make_log(msg_toLog);
        return 1;
    }
    msg_toLog = "[INFO] 文件已打开：" + src_file + '\n';
    make_log(msg_toLog);
    
    src_info = src_file_path + string("/videoinfo.txt"); 
    info.open(src_info.c_str(), ios_base::in);
    if(!info.is_open())
    {
        msg_toLog =  "[ERROR] 未找到视频信息文件\n";
        make_log(msg_toLog);
        
        msg_toLog =  "[INFO] 选择分辨率：360p (0)、480p (1)、720p (2) 、1080p (3)\n";
        make_log(msg_toLog);
        cin >> tem;
    
        msg_toLog =  "[INFO] 输入帧率：\n";
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
        msg_toLog =  "[ERROR] 预置无该分辨率\n";
        make_log(msg_toLog);
        return 1;
    }

    chrono::milliseconds interval(1000 / frame_rate);

    VRAM_toPlay = VRAM_sw();

    thread t1(thread1_inputData);
    std::unique_lock<std::mutex> lck(uni_mtx);
    cdn_v.wait(lck, []{return t1_start;});
    lck.unlock();

    thread t2(thread2_transData);
    thread timer(thread3_playVideo, interval);    // 创建并启动定时器线程

    while (!terminateProgram)
    {
        if(fileEnd)
        {
            std::unique_lock<std::mutex> lck(uni_mtx);
            cdn_v.wait(lck, []
                       { return (bool)terminateProgram; }); // 等待程序结束
            cdn_v.notify_all();                             // 通知所有线程终止
            msg_toLog = "[INFO] 播放完成，程序结束\n";
            make_log(msg_toLog);
            break; // 退出输入循环
        }
        else if(userEnd)
        {
            char c;
            cin >> c; // 清除输入缓冲区中的 'q'

            std::unique_lock<std::mutex> lck(uni_mtx);
            terminateProgram = true; // 设置终止标志
            cdn_v.notify_all();      // 通知所有线程终止
            msg_toLog = "[INFO] 用户终止程序\n";
            make_log(msg_toLog);
            break; // 退出输入循环
        }
    }

    t1.join();
    t2.join();
    timer.detach();

    src.close();
    info.close();
    uni_log.close();
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
        do{
            VRAM_toPlay = VRAM_sw();
        }while(nullptr == VRAM_toPlay);

        if(!t1_start)
        {
            t1_start = true;
            cdn_v.notify_one();
        }

        std::unique_lock<std::mutex> lck((*VRAM_toPlay).vram_mtx);
        msg_toLog =  "[INFO] 线程一: 开始工作\n";
        make_log(msg_toLog);
        
        input_yuvData_1f(fmt, VRAM_toPlay);

        msg_toLog =  "[INFO] 线程一: 工作结束\n";
        make_log(msg_toLog);

        lck.unlock();
        t1_done = true;

        cdn_v.notify_all();  // 通知其他线程

        cdn_v.wait(lck, []{ return t2_done; }); // 等待线程二完成
        t2_done = false; // 重置标志位
    }
}

/**
 * @brief 数据转换线程
*/
void thread2_transData() 
{
    while(!terminateProgram) 
    {
        std::unique_lock<std::mutex> lck((*VRAM_toPlay).vram_mtx);
        cdn_v.wait(lck, []{ return t1_done; }); // 等待线程一完成
        
        msg_toLog =  "[INFO] 线程二: 开始工作\n";
        make_log(msg_toLog);
        
        trans_yuv2rgb888_1f(fmt, VRAM_toPlay);

        msg_toLog =  "[INFO] 线程二: 结束工作\n";
        make_log(msg_toLog);

        cdn_v.notify_all();  // 通知其他线程
        lck.unlock();
        t2_done = true;
        
        cdn_v.wait(lck, []{ return t1_done; }); // 等待线程一再次完成
        t1_done = false; // 重置标志位
    }
}

/**
 * @brief 视频播放线程
*/
void thread3_playVideo(chrono::milliseconds interval) 
{
    while(!terminateProgram) 
    {
        this_thread::sleep_for(interval);    

        std::unique_lock<std::mutex> lck((*VRAM_toPlay).vram_mtx);
        cdn_v.wait(lck, []{ return t2_done; }); // 等待线程二完成

        msg_toLog =  "[INFO] 线程三: 开始工作\n";
        make_log(msg_toLog);

        play_VRAM(fmt, VRAM_toPlay);

        msg_toLog =  "[INFO] 线程三: 结束工作\n";
        make_log(msg_toLog);

        cdn_v.notify_all();  // 通知其他线程
        if(fileEnd || userEnd)
            terminateProgram = true;    // 通知程序结束

        lck.unlock();
    }
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
        msg_toLog =  "[ERROR] VRAM 已满\n";
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
void input_yuvData_1f(enum PIXEL_FMT FMT, struct VRAM_t* VRAM)
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

    if(!src.eof())
    {

        for(int i=0; i<pixel_max_len; i++)
            src.read((char*)&((*VRAM).data[3*i]), 1);    // 读取 Y 分量
        
        for(int i=0; i<pixel_max_len; i+=2)
        {
            src.read((char*)&((*VRAM).data[3*i + 1]), 1);    // 读取 U 分量
            (*VRAM).data[3*i + 1 + pixelFmt_size[FMT][0]] = (*VRAM).data[3*i + 1];    // 重复填充 U 分量
            (*VRAM).data[3*(i+1) + 1] = (*VRAM).data[3*i + 1];    // 重复填充 U 分量
            (*VRAM).data[3*(i+1) + 1 + pixelFmt_size[FMT][0]] = (*VRAM).data[3*i + 1];    // 重复填充 U 分量
        }
    
        for(int i=0; i<pixel_max_len; i+=2)
        {
            src.read((char*)&((*VRAM).data[3*i + 2]), 1);    // 读取 V 分量
            (*VRAM).data[3*i + 2 + pixelFmt_size[FMT][0]] = (*VRAM).data[3*i + 2];    // 重复填充 V 分量
            (*VRAM).data[3*(i+1) + 2] = (*VRAM).data[3*i + 2];    // 重复填充 V 分量
            (*VRAM).data[3*(i+1) + 2 + pixelFmt_size[FMT][0]] = (*VRAM).data[3*i + 2];    // 重复填充 V 分量
        }
    }
    else if(src.fail())
    {
        msg_toLog =  "[ERROR] 文件已读取完毕\n";
        make_log(msg_toLog);
        (*VRAM).is_empty = true;    // 释放 VRAM
        fileEnd = true;
    }
    else
    {
        msg_toLog =  "[ERROR] 文件读取异常\n";
        make_log(msg_toLog);
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

    cv::imshow("RGB 视频", play_frame);
    
    int key = cv::waitKey(1);
    if('q' == key || 'Q' == key)
        userEnd = true;

    (*VRAM).is_empty = true;    // 释放 VRAM
}

