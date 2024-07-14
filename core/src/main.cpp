#include "main.hpp"
#include "vision.hpp"
#include "serial.h"
#include "stdint.h"
#include "stdio.h"
#include "tuple"
#include "thread"
#include "format"
#include "iostream"
#include "fpv_serial.hpp"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdlib>

using namespace cv;

void Opencl_init();
void Yolo_cam_task();
uint32_t get_time();

std::string getTimestampedFilename();
int detect_balls(cv::InputArray frame_in, cv::InputOutputArray frame_labled,
                        cv::OutputArray frame_binary, std::vector<cv::Point> &balls_detected, int debug_en);
// FPV_Serial Fpv_serial("/dev/ttyACM0", 230400);

// BinoCamera BinoPair(0, 2, 140); //双目摄像头对象


int main()
{    
    Opencl_init();  //初始化OpenCL

    Yolo_cam_task();
    
    return 0;
}

/*初始化OpenCL*/
void Opencl_init()
{
    cv::ocl::setUseOpenCL(false);
    std::cout << cv::ocl::haveOpenCL() <<std::endl;

    cv::ocl::Context context;
    if (!context.create(cv::ocl::Device::TYPE_ALL))
    {
        std::cout << "Failed creating the context..." << std::endl;
        //return;
    }

    std::cout << context.ndevices() << " GPU devices are detected." << std::endl; //This bit provides an overview of the OpenCL devices you have in your computer
    for (int i = 0; i < context.ndevices(); i++)
    {
        cv::ocl::Device device = context.device(i);
        std::cout << "name:              " << device.name() << std::endl;
        std::cout << "available:         " << device.available() << std::endl;
        std::cout << "imageSupport:      " << device.imageSupport() << std::endl;
        std::cout << "OpenCL_C_Version:  " << device.OpenCL_C_Version() << std::endl;
        std::cout << std::endl;
    }
}

void Yolo_cam_task()
{
    /*timing*/
    uint32_t prev = get_time();
    uint32_t curr = 0;
    static uint32_t frame_count;
    
    /*Debug en*/
    int debug_en = 0;

    cv::VideoCapture Cam;
    cv::Mat frame_cam;
    cv::Mat frame_baled;
    cv::Mat frame_binary;
    std::vector<cv::Point> balls_detected;

    Cam.open(0, cv::CAP_V4L2);

    Cam.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );
    Cam.set(cv::CAP_PROP_FPS, 120);
    Cam.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    Cam.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    Cam.set(cv::CAP_PROP_BRIGHTNESS, 10);
    Cam.set(cv::CAP_PROP_TEMPERATURE, 7000);
    std::cout << Cam.get(CAP_PROP_FPS) << std::endl;
 
    if(debug_en) cv::namedWindow("cam", cv::WINDOW_AUTOSIZE);


    prev = get_time();
    curr = get_time();
   
    while(1){
        prev = curr;
        curr = get_time();

        Cam.read(frame_cam);
        detect_balls(frame_cam, frame_baled, frame_binary, balls_detected, debug_en);
        if(debug_en) cv::imshow("cam", frame_baled);

        /*DEBUG INFO*/
        printf("---------------frame_begin-------------\n");

        for(int i=0; i<balls_detected.size(); i++){
            printf("center%d:\t%d,\t%d\n", i, balls_detected[i].x, balls_detected[i].y);
        }

        printf("  - time: %u\n", curr - prev);
        printf("  - frame: %u\n", frame_count++);
        printf("  - curr: %ld\n", balls_detected.size());
        printf("---------------frame_end-------------\n");

        if(debug_en) if(cv::waitKey(1) == 'q') break;
        }
}

uint32_t get_time()
{
std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>
      tp = std::chrono::time_point_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}


std::string getTimestampedFilename() {
    // 获取当前系统时间点
    auto now = std::chrono::system_clock::now();
    
    // 转换为time_t格式，便于转换为本地时间
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    // 使用put_time进行格式化，包括年月日时分秒
    std::tm bt = *std::localtime(&time_t_now);
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d_%H-%M-%S");

    // 获取自午夜以来的毫秒数
    auto since_epoch = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(since_epoch);
    since_epoch -= seconds;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);

    // 组合完整的文件名，包括毫秒
    oss << '-' << std::setfill('0') << std::setw(3) << milliseconds.count();

    return "/home/fish/GKD/Record/video/" + oss.str() + ".avi";  // 生成.avi视频文件名
}



int detect_balls(cv::InputArray frame_in, cv::InputOutputArray frame_labled,
                        cv::OutputArray frame_binary, std::vector<cv::Point> &balls_detected, int debug_en)
{
    /*----------魔法参数------------*/
    float circle_ratio = 4*3.14; //圆(周长平方/面积)的比值
    float circle_tolorance = 0.8; //判定圆允许的误差
 
    /*摄像机参数*/
    int CAM_FRAME_HEIGHT = 720;
    int CAM_FRAME_WIDTH = 1280;
    int CAM_FRAME_FPS = 120;
    int CAM_BRIGHTNESS = 5;


    std::vector<std::vector<cv::Point> > country;   //轮廓线
    std::vector<cv::Vec4i> hiera;
    cv::Rect bound_box;

    /*inRange() 使用UMat会异常，因此加上中间变量Mat*/
    cv::Mat binary_Mat;

    /*过滤&二值化*/
    frame_in.copyTo(frame_binary);
    frame_in.copyTo(frame_labled);
    cv::medianBlur(frame_binary, frame_binary, 1);
    cv::cvtColor(frame_binary, frame_binary, cv::COLOR_BGR2GRAY);
    cv::threshold(frame_binary, frame_binary, 220, 255, cv::THRESH_BINARY);

    /*寻找轮廓*/
    cv::findContours(frame_binary, country, hiera, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    /*筛选乒乓球的轮廓*/
    //用面积大小 & 边长和面积比值筛选圆

    balls_detected.clear();     //清空旧有的数据
    for( size_t i = 0; i< country.size(); i++ )
    {
        /*计算轮廓的边缘&面积*/
        float area, length; 
        area = cv::contourArea(country[i]);
        length = cv::arcLength(country[i], true);
        if(debug_en){
            drawContours(frame_labled, country, (int)i, 
                        cv::Scalar(127, 127, 127), 2, cv::LINE_8, hiera, 0);
        }
        /*当符合乒乓球轮廓的条件时*/
        if(area > 0.1 && abs(length*length/area - circle_ratio) < circle_ratio*circle_tolorance)
        {
            bound_box = cv::boundingRect(country[i]);
            if(debug_en){
            /*画出轮廓&标记框&圆心*/
                drawContours(frame_labled, country, (int)i, cv::Scalar(255, 0, 0), 2, cv::LINE_8, hiera, 0);
                cv::rectangle(frame_labled, bound_box.tl(), bound_box.br(), cv::Scalar(0, 255, 0), 2);
                cv::circle(frame_labled, cv::Point(bound_box.x+bound_box.width/2, 
                                                    bound_box.y+bound_box.height/2), 2, 
                                                    cv::Scalar(0, 0, 255), 2);
            }
            /*输出圆心数据*/
            balls_detected.push_back(cv::Point(bound_box.x+bound_box.width/2, 
                                        bound_box.y+bound_box.height/2));
        }
    }
    frame_labled.copyTo(frame_labled);
    return 0;
}