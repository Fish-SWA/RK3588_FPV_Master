#include "vision.hpp"
#include "main.hpp"

void BinoCamera::init(int cam_l_in, int cam_r_in, int cam_dist_in)
{
    cam_dist = cam_dist_in;
    cam_l_id = cam_l_in;
    cam_r_id = cam_r_in;
}

int BinoCamera::open()
{
    /*打开摄像头*/
    Cam_L.open(cam_l_id, cv::CAP_V4L2);
    Cam_R.open(cam_r_id, cv::CAP_V4L2);
    /*等待摄像头就绪*/
    while((!Cam_L.isOpened()) | (!Cam_R.isOpened())){
        printf("waitting camera....\n");
        Sleep_ms(100);
    }
    /*设置相机参数*/
    Cam_L.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );
    Cam_L.set(cv::CAP_PROP_FPS, CAM_FRAME_FPS);
    Cam_L.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_FRAME_HEIGHT);
    Cam_L.set(cv::CAP_PROP_FRAME_WIDTH, CAM_FRAME_WIDTH);
    Cam_L.set(cv::CAP_PROP_BRIGHTNESS, CAM_BRIGHTNESS);

    Cam_R.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );
    Cam_R.set(cv::CAP_PROP_FPS, CAM_FRAME_FPS);
    Cam_R.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_FRAME_HEIGHT);
    Cam_R.set(cv::CAP_PROP_FRAME_WIDTH, CAM_FRAME_WIDTH);
    Cam_R.set(cv::CAP_PROP_BRIGHTNESS, CAM_BRIGHTNESS);
    
    return 0;
}

BinoCamera::BinoCamera(int cam_l_in, int cam_r_in, int cam_dist_in)
{
    cam_dist = cam_dist_in;
    cam_l_id = cam_l_in;
    cam_r_id = cam_r_in;
}

BinoCamera::~BinoCamera()
{
}