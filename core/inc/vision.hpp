#include "opencv2/opencv.hpp"

class BinoCamera
{
private:
    int cam_dist = 0;   //两摄像头之间的距离，mm
    int cam_l_id = 0;  //左摄像头id
    int cam_r_id = 0;  //右摄像头id

    /*魔法参数*/
    float circle_ratio = 4*3.14; //圆(周长平方/面积)的比值

    /*摄像机参数*/
    int CAM_FRAME_HEIGHT = 480;
    int CAM_FRAME_WIDTH = 640;
    int CAM_FRAME_FPS = 60;
    int CAM_BRIGHTNESS = 10;

    /*画面帧*/
    cv::Mat Frame_raw_L;
    cv::Mat Frame_raw_R;

    /*图像识别函数*/
    int hough_circles(cv::Mat frame_input);    //用霍夫变换识别圆
    
public:
    cv::VideoCapture Cam_L; //左摄像头对象
    cv::VideoCapture Cam_R; //右摄像头对象

    void init(int cam_l, int cam_r, int cam_dist);
    int open();     //打开相机
    int monitor_window();   //打开相机的监视窗口
    int monitor_task();     //相机监视窗口打维护线程
    BinoCamera(int cam_l_in, int cam_r_in, int cam_dist_in);
    ~BinoCamera();
};



