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

int BinoCamera::monitor_task()
{
    namedWindow("Cam_L", cv::WINDOW_AUTOSIZE);  //左摄像头窗口
    namedWindow("Cam_R", cv::WINDOW_AUTOSIZE);  //右摄像头窗口

    cv::Mat frame_binary;
    std::vector<std::vector<cv::Point> > country;   //轮廓线
    std::vector<cv::Vec4i> hiera;

    cv::Rect bound_box;

    while(1)
    {
        Cam_L.read(Frame_raw_L);
        Cam_R.read(Frame_raw_R);

        /*画面处理，暂时*/
        
        /*过滤颜色&二值化*/
        cv::cvtColor(Frame_raw_R, frame_binary, cv::COLOR_RGB2HSV);
        cv::circle(Frame_raw_R, cv::Point(CAM_FRAME_WIDTH/2, CAM_FRAME_HEIGHT/2), 10, cv::Scalar(255,0,0));
        std::cout << Frame_raw_R.at<cv::Vec3b>(CAM_FRAME_HEIGHT/2, CAM_FRAME_WIDTH/2) << std::endl;
        cv::inRange(frame_binary, cv::Scalar(85, 100, 210),
                                cv::Scalar(110, 180, 255), frame_binary);
        cv::medianBlur(frame_binary, frame_binary, 3);
        cv::threshold(frame_binary, frame_binary, 120, 255, cv::THRESH_BINARY);

        /*寻找轮廓*/
        cv::findContours(frame_binary, country, hiera, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        /*绘制边界*/
        //用面积大小 & 边长和面积比值筛选圆
        for( size_t i = 0; i< country.size(); i++ )
        {
            float area, length; //周长&面积
            area = cv::contourArea(country[i]);
            length = cv::arcLength(country[i], true);
            if(area > 100 && abs(length*length/area - circle_ratio) < circle_ratio*0.9)
            {
                // printf("id:%d, aera:%f, length:%f\n", i, area, length);
                cv::Scalar color = cv::Scalar(255, 0, 0);
                drawContours( Frame_raw_R, country, (int)i, color, 2, cv::LINE_8, hiera, 0);
                bound_box = cv::boundingRect(country[i]);
                cv::rectangle( Frame_raw_R, bound_box.tl(), bound_box.br(), cv::Scalar(0, 255, 0), 2);
                cv::circle(Frame_raw_R, cv::Point(bound_box.x+bound_box.width/2, 
                                                    bound_box.y+bound_box.height/2), 2, 
                                                    cv::Scalar(0, 0, 255), 2);
                printf("center:%d, %d\n", bound_box.x+bound_box.width/2, bound_box.y+bound_box.height/2);
            }
        }

        cv::imshow("Cam_L", Frame_raw_R);
        cv::imshow("Cam_R", frame_binary);
        if(cv::waitKey(1) == 'q') break;

    }
    return 0;
}

int BinoCamera::hough_circles(cv::Mat frame_input)
{
    cv::Mat frame;
    return 0;
}

int BinoCamera::monitor_window()
{
    std::thread (monitor_task);
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