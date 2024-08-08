#include "opencv2/opencv.hpp"
#include <opencv2/highgui/highgui.hpp>

//OpenCV
#include "opencv2/core/ocl.hpp"

class BinoCamera
{
private:
    int cam_dist = 0;   //两摄像头之间的距离，mm
    int cam_l_id = 0;  //左摄像头id
    int cam_r_id = 0;  //右摄像头id

    /*----------魔法参数------------*/
    float circle_ratio = 4*3.14; //圆(周长平方/面积)的比值
    float circle_tolorance = 0.8; //判定圆允许的误差
    /*过滤乒乓球颜色的阈值  (H, S, V)*/
    cv::Scalar Filter_HSV_LOW = cv::Scalar(85, 60, 245);    //(70, 100, 210);
    cv::Scalar Filter_HSV_HIGH = cv::Scalar(110, 255, 255);  //(170, 255, 255);

    int Filter_Debug = 0;
    int Thread_monitor = 0; //1监看放到独立线程（可能导致监看卡顿）， 0监看与识别在一个线程
    
 
    /*摄像机参数*/
    int CAM_FRAME_HEIGHT = 720;
    int CAM_FRAME_WIDTH = 1280;
    int CAM_FRAME_FPS = 60;
    int CAM_BRIGHTNESS = 5;

    /*画面帧*/
    cv::UMat Frame_raw_L;
    cv::UMat Frame_raw_R;

    /*工具函数*/
    void monitor_frame(cv::String windows_name, cv::UMat& frame);    //监控线程

    /*图像识别函数*/
    //识别乒乓球
    int detect_balls(cv::InputArray frame_in, cv::InputOutputArray frame_labled,
                        cv::OutputArray frame_binary, std::vector<cv::Point> &balls_detected);
    uint32_t get_time();        //获取毫秒时间 
public:
    cv::VideoCapture Cam_L; //左摄像头对象
    cv::VideoCapture Cam_R; //右摄像头对象
    void init(int cam_l, int cam_r, int cam_dist);
    int open();     //打开相机
    int monitor_window();   //打开相机的监视窗口
    int monitor_task();     //相机监视窗口的维护线程
    int Filter_debug_task();



    BinoCamera(int cam_l_in, int cam_r_in, int cam_dist_in);
    ~BinoCamera();
};

/*******************************识别颜色小球的类*************************/
class ColoredBall
{
private:
    int Filter_Debug = 0;
public:
    /*----------魔法参数------------*/
    float circle_ratio = 4*3.14; //圆(周长平方/面积)的比值
    float circle_tolorance = 0.8; //判定圆允许的误差
    float mini_balls_size = 50;
    /*过滤乒乓球颜色的阈值  (H, S, V)*/
    cv::Scalar Filter_HSV_LOW = cv::Scalar(85, 60, 245);    //(70, 100, 210);
    cv::Scalar Filter_HSV_HIGH = cv::Scalar(110, 255, 255);  //(170, 255, 255);
    /*摄像机参数*/
    int CAM_FRAME_HEIGHT = 480;
    int CAM_FRAME_WIDTH = 640;
    int CAM_FRAME_FPS = 60;
    int CAM_BRIGHTNESS = 5;


    int detect(cv::InputArray frame_in, cv::InputOutputArray frame_labled,
                        cv::OutputArray frame_binary, std::vector<cv::Point> &balls_detected);
    ColoredBall(/* args */);
    ~ColoredBall();
};





