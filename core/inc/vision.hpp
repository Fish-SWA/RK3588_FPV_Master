#include "opencv2/opencv.hpp"

class BinoCamera
{
private:
    int cam_dist = 0;   //两摄像头之间的距离，mm
    int cam_l_id = 0;  //左摄像头id
    int cam_r_id = 0;  //右摄像头id

    /*摄像机参数*/
    int CAM_FRAME_HEIGHT = 720;
    int CAM_FRAME_WIDTH = 1280;
    int CAM_FRAME_FPS = 60;
    int CAM_BRIGHTNESS = 10;
    
public:
    cv::VideoCapture Cam_L; //左摄像头对象
    cv::VideoCapture Cam_R; //右摄像头对象

    void init(int cam_l, int cam_r, int cam_dist);
    int open();     //打开相机
    BinoCamera(int cam_l_in, int cam_r_in, int cam_dist_in);
    ~BinoCamera();
};



