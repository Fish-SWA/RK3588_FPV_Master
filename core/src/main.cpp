#include "main.hpp"
#include "vision.hpp"
#include "rknpu_yolo.hpp"
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


/*视频推流相关*/
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

/*文件写入*/
#include <fstream>

using namespace cv;

void Opencl_init();
void Yolo_cam_task();
std::string getTimestampedFilename();
std::string getFilename();
FPV_Serial Fpv_serial("/dev/ttyACM0", 230400);
ColoredBall balls_detector;
LogFile log_file;
int window_toggle = 1;

// BinoCamera BinoPair(0, 2, 140); //双目摄像头对象

RkNPU Rkyolo;

enum vision_state{
    YOLO_FULL = 0,
    YOLO_BALLS,
    YOLO_PHONE,
    YOLO_PERSION,
    FILTER_BALLS,    //传统算法识别球
    VISION_END
};
int VISION_MODE = YOLO_FULL;



int main()
{    
    Opencl_init();  //初始化OpenCL

    Yolo_cam_task();

    while(1){
        Fpv_serial.update_fpv_data();
        // printf("Yaw:%f\t\tPitch:%f\t\tRoll:%f\t\t\n", 
        //     Fpv_serial.data.IMU_yaw, Fpv_serial.data.IMU_pitch, Fpv_serial.data.IMU_roll);

        for(int i=0; i<16; i++){
            printf("%d\t", Fpv_serial.data.CrsfChannels[i]);
        }
        printf("\n");
        Sleep_ms(1);    
    }
    
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
    //模型路径
    Rkyolo.model_path = "/home/fish/GKD/RK3588_FPV_Master/model/yolov5s-640-640.rknn";
    //标签列表路径
    Rkyolo.label_name_txt_path = "/home/fish/GKD/RK3588_FPV_Master/model/coco_80_labels_list.txt";

    Rkyolo.rknn_model_init();  //初始化RKNN模型

    /*初始化摄像头Streaming*/
    cv::VideoWriter Cam_streaming;
    std::string gst_out = "appsrc ! videoconvert ! x264enc tune=zerolatency ! rtph264pay ! udpsink host=127.0.0.1 port=5000 ";
    Cam_streaming.open(gst_out, 0, (double)60, cv::Size(1280, 720), true);
    if(Cam_streaming.isOpened()){
        printf("cam streaming ok!\n");
    }else{
        printf("cam streaming fail!\n");
    }

    // printf("---------opencv build info -------");
    // std::cout << getBuildInformation() << std::endl;

    cv::VideoCapture Cam;
    cv::Mat frame_cam;

    Cam.open(0, cv::CAP_V4L2);

    Cam.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );
    Cam.set(cv::CAP_PROP_FPS, 60);
    Cam.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    Cam.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    Cam.set(cv::CAP_PROP_BRIGHTNESS, 10);
    Cam.set(cv::CAP_PROP_TEMPERATURE, 7000);

    //cv::namedWindow("yolo_cam", cv::WINDOW_AUTOSIZE);

    /*录像*/
    cv::VideoWriter yolo_video_record;

   
    while(1){
    Fpv_serial.update_fpv_data();
    Cam.read(frame_cam);

    for(int i=0; i<7; i++){
       printf("%d\t", Fpv_serial.data.CrsfChannels[i]);
    }
    // printf("\n");

     printf("Yaw:%f\t\tPitch:%f\t\tRoll:%f\t\t\n", 
         Fpv_serial.data.IMU_yaw, Fpv_serial.data.IMU_pitch, Fpv_serial.data.IMU_roll);

    //if(Fpv_serial.data.CrsfChannels[6] == 191){
        //system("halt");
    //}

    /*----------视频录制处理----------*/
    if(Fpv_serial.data.CrsfChannels[6] == 1792 && Fpv_serial.last_data.CrsfChannels[6] != 1792){
        printf("video_rec_on!\n");
        std::cout << getTimestampedFilename() <<std::endl;
        yolo_video_record.open(getTimestampedFilename() + ".avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                                30, cv::Size(640, 480));
        //logs
        log_file.open(getFilename() + ".csv");
        std::cout << getTimestampedFilename() <<std::endl;
    }

    if(Fpv_serial.data.CrsfChannels[6] != 1792 && Fpv_serial.last_data.CrsfChannels[6] == 1792){
        printf("video_rec_done!\n");
        yolo_video_record.release();
        //logs
        log_file.close();
    }

    /*----------视觉状态设定----------*/
    if(Fpv_serial.data.CrsfChannels[6] == 191 && Fpv_serial.last_data.CrsfChannels[6] != 191){
        //当检测到拨杆向下拨时
        VISION_MODE = VISION_MODE + 1;
        if(VISION_MODE >= VISION_END){
            VISION_MODE = 0;
        }
        printf("vision mode change to: %d\n", VISION_MODE);
    }

    /*相机错误处理*/
    if(frame_cam.empty()){
        printf("cam_fail!!\n");
        Sleep_ms(100);
	continue;
    }

    /*---------------视觉任务----------------*/
    if(VISION_MODE == YOLO_FULL){
        _detect_result_group_t detect_result;

        Rkyolo.rknn_img_inference(frame_cam, &detect_result);
        
        Rkyolo.yolo_draw_results(frame_cam, frame_cam, &detect_result);
    }

    if(VISION_MODE == FILTER_BALLS){
        cv::Mat frame_binary;
        std::vector<cv::Point> balls;
        balls_detector.detect(frame_cam, frame_cam, frame_binary, balls);
    }

    if(VISION_MODE == YOLO_PERSION){
        _detect_result_group_t detect_result;

        Rkyolo.rknn_img_inference(frame_cam, &detect_result);
        
        Rkyolo.yolo_draw_results_match(frame_cam, frame_cam, &detect_result, "person");
    }

    if(VISION_MODE == YOLO_PHONE){
        _detect_result_group_t detect_result;

        Rkyolo.rknn_img_inference(frame_cam, &detect_result);
        
        Rkyolo.yolo_draw_results_match(frame_cam, frame_cam, &detect_result, "cell_phone");
    }

    if(VISION_MODE == YOLO_BALLS){
        _detect_result_group_t detect_result;

        Rkyolo.rknn_img_inference(frame_cam, &detect_result);
        
        Rkyolo.yolo_draw_results_match(frame_cam, frame_cam, &detect_result, "sports_ball");
        Rkyolo.yolo_draw_results_match(frame_cam, frame_cam, &detect_result, "orange");
    }

    /*录像*/
    if(Fpv_serial.data.CrsfChannels[6] == 1792){
        yolo_video_record.write(frame_cam);
        log_file.write_data_line(Fpv_serial.data);
    }

    if(window_toggle) cv::imshow("yolo_cam", frame_cam);

    if(cv::waitKey(1) == 'q') break;
    }
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

    return "/home/fish/GKD/Record/video/" + oss.str();  // 生成.avi视频文件名
}

std::string getFilename() {
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

    return oss.str();  // 生成.avi视频文件名
}

