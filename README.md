# 20240307_visionProcess_homework1
***
## 目录
- [项目简介](#项目简介)
- [依赖列表](#依赖列表)
- [安装过程](#安装过程)
- [使用方法](#使用方法)

***
## 项目简介

### 项目介绍

### 项目结构
``` shell
----20240307_visionProcess_homework1
    |----.gitignore   (32.000B)
    |----.vscode
    |    |----c_cpp_properties.json   (607.000B)
    |    |----launch.json   (1.024KB)
    |    |----settings.json   (1.797KB)
    |    |----tasks.json   (955.000B)
    |----log.txt   (0.000B)
    |----main.cpp   (7.390KB)
    |----main.exe   (9.248MB)
    |----main.h   (1.968KB)
    |----main.o   (9.248MB)
    |----README.md   (0.000B)
    |----requirements.txt   (0.000B)
    |----video
    |    |----test.mp4   (6.267MB)
    |    |----videoinfo.txt   (4.000B)
```
包含以下类型的文件：['无后缀', '.txt', '.cpp', '.exe', '.h', '.o', '.md', '.json', '.mp4']

***
## 功能列表
自己看代码

***
## 依赖
自己看代码

***
## 安装过程

1. 配置 OpenCV 的 C++ 接口；
2. （可选）配置 .vscode 文件以自动输入命令。

注：推荐在 linux 系统下配置 OpenCV ， Windows 下我试了好几次，都不行。

***
## 使用方法
``` shell
g++ -g -std=c++11 main.cpp -o main.o -I /usr/local/include -I /usr/local/include/opencv4 -I /usr/local/include/opencv4/opencv2 -L /usr/local/lib -l opencv_core -l opencv_imgproc -l opencv_imgcodecs -l opencv_video -l opencv_ml -l opencv_highgui -l opencv_objdetect -l opencv_flann -l opencv_imgcodecs -l opencv_photo -l opencv_videoio 
```

***
## 更新日志
自己看代码

***
