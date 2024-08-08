#include "vision.hpp"
#include "main.hpp"
#include <thread>



/*******************************双目品颜色小球的类*************************/
std::mutex img_lock; 
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
        printf("waitting camera....(L:%d, R:%d)\n",
                        Cam_L.isOpened(), Cam_R.isOpened());
        Sleep_ms(100);
    }
    /*设置相机参数*/
    Cam_L.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );
    Cam_L.set(cv::CAP_PROP_FPS, CAM_FRAME_FPS);
    Cam_L.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_FRAME_HEIGHT);
    Cam_L.set(cv::CAP_PROP_FRAME_WIDTH, CAM_FRAME_WIDTH);
    Cam_L.set(cv::CAP_PROP_BRIGHTNESS, CAM_BRIGHTNESS-5);
    // Cam_L.set(cv::CAP_TE<, CAM_BRIGHTNESS-5);
    Cam_L.set(cv::CAP_PROP_TEMPERATURE, 7000);

    Cam_R.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );
    Cam_R.set(cv::CAP_PROP_FPS, CAM_FRAME_FPS);
    Cam_R.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_FRAME_HEIGHT);
    Cam_R.set(cv::CAP_PROP_FRAME_WIDTH, CAM_FRAME_WIDTH);
    Cam_R.set(cv::CAP_PROP_BRIGHTNESS, CAM_BRIGHTNESS);
    Cam_R.set(cv::CAP_PROP_TEMPERATURE, 7000);
    
    return 0;
}

int BinoCamera::detect_balls(cv::InputArray frame_in, cv::InputOutputArray frame_labled,
                        cv::OutputArray frame_binary, std::vector<cv::Point> &balls_detected)
{
    std::vector<std::vector<cv::Point> > country;   //轮廓线
    std::vector<cv::Vec4i> hiera;
    cv::Rect bound_box;

    /*inRange() 使用UMat会异常，因此加上中间变量Mat*/
    cv::Mat binary_Mat;
    cv::UMat frame_processing;

    /*转换为HSV色彩*/
    frame_in.copyTo(frame_processing);
    cv::cvtColor(frame_in, binary_Mat, cv::COLOR_RGB2HSV);

    /*Debug: 输出中心点的HVS值, 用来调过滤器阈值*/
    if(Filter_Debug){
        cv::circle(frame_processing, cv::Point(CAM_FRAME_HEIGHT/2, CAM_FRAME_WIDTH/2), 
                                                        10, cv::Scalar(255,0,0));
        std::cout << binary_Mat.at<cv::Vec3b>(CAM_FRAME_WIDTH/2, CAM_FRAME_HEIGHT/2) 
                                                                    << std::endl;
    }
    
    /*过滤&二值化*/
    cv::inRange(binary_Mat, Filter_HSV_LOW,
                            Filter_HSV_HIGH, frame_binary);
    cv::medianBlur(frame_binary, frame_binary, 3);
    cv::threshold(frame_binary, frame_binary, 120, 255, cv::THRESH_BINARY);

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
        cv::Scalar color = cv::Scalar(127, 127, 127);
        drawContours(frame_processing, country, (int)i, color, 2, cv::LINE_8, hiera, 0);
        /*当符合乒乓球轮廓的条件时*/
        if(area > 20 && abs(length*length/area - circle_ratio) < circle_ratio*circle_tolorance)
        {
            color = cv::Scalar(255, 0, 0);
            /*画出轮廓&标记框&圆心*/
            drawContours(frame_processing, country, (int)i, color, 2, cv::LINE_8, hiera, 0);
            bound_box = cv::boundingRect(country[i]);
            cv::rectangle(frame_processing, bound_box.tl(), bound_box.br(), cv::Scalar(0, 255, 0), 2);
            cv::circle(frame_processing, cv::Point(bound_box.x+bound_box.width/2, 
                                                bound_box.y+bound_box.height/2), 2, 
                                                cv::Scalar(0, 0, 255), 2);
            /*输出圆心数据*/
            balls_detected.push_back(cv::Point(bound_box.x+bound_box.width/2, 
                                        bound_box.y+bound_box.height/2));
            // printf("center:%d, %d\n", bound_box.x+bound_box.width/2, bound_box.y+bound_box.height/2);
        }
    }
    frame_processing.copyTo(frame_labled);


    return 0;
}


int BinoCamera::monitor_task()
{
    if(!Thread_monitor){
    namedWindow("Cam_L", cv::WINDOW_AUTOSIZE);  //左摄像头窗口
    namedWindow("Cam_R", cv::WINDOW_AUTOSIZE);  //右摄像头窗口
    }

    cv::UMat frame_binary[2];
    cv::UMat frame_labeled[2];
    cv::UMat frame_show[2];
    std::vector<cv::Point> balls[2];
    uint32_t time_p1, time_p2, time_p3, time_p4, time_p5;

    if(Filter_Debug) Filter_debug_task();   //Debug模式时进入调参任务

    /*读取帧*/
    Cam_L.read(frame_show[0]);
    Cam_R.read(frame_show[1]);


    /*启动监视窗口*/
    std::thread Cam_L_view(std::bind(&BinoCamera::monitor_frame, this, 
                            "Cam_L", frame_show[0]));
    std::thread Cam_R_view(std::bind(&BinoCamera::monitor_frame, this, 
                            "Cam_R", frame_show[1]));

    /*creat ORB detector*/ 
    cv::Ptr<cv::ORB> orb = cv::ORB::create(2000);
    std::vector<cv::KeyPoint> tracking_point_L;

    /*gray mat*/
    cv::UMat Frame_raw_L_gray;


    while(1)
    {
        /*读取帧*/
        Cam_L.read(Frame_raw_R);
        Cam_R.read(Frame_raw_L);

        /*识别*/
        cv::cvtColor(Frame_raw_L, Frame_raw_L_gray, cv::COLOR_RGB2GRAY);
        orb->detect(Frame_raw_L_gray, tracking_point_L);

        //draw all key points
        for(int i=0; i < tracking_point_L.size(); i++)
        {
            if(tracking_point_L[i].response > 0.0005){
                cv::circle(Frame_raw_L, tracking_point_L[i].pt, 2, cv::Scalar(0, 255, 0), 2);
            }else{
                cv::circle(Frame_raw_L, tracking_point_L[i].pt, 2, cv::Scalar(255, 0, 0), 2);
            }
        }

        std::cout << tracking_point_L.size() << std::endl;
        std::cout << tracking_point_L[0].response << std::endl;
        // std::cout << tracking_point_L[0] std::endl;
        // cv::drawKeypoints(Frame_raw_L, tracking_point_L, Frame_raw_L, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

        /*显示*/
        if(!Thread_monitor){
        cv::imshow("Cam_L", Frame_raw_L);
        cv::imshow("Cam_R", Frame_raw_R);
        }

        if(!Thread_monitor){
        if(cv::waitKey(1) == 'q') break;
        }


    }
    return 0;
}

uint32_t BinoCamera::get_time()
{
std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>
      tp = std::chrono::time_point_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}

void BinoCamera::monitor_frame(cv::String windows_name, cv::UMat& frame)
{
    if(!Thread_monitor) return; //不处于线程监看模式直接推出

    namedWindow(windows_name, cv::WINDOW_AUTOSIZE);

    while(1)
    {
        img_lock.lock();
        img_lock.unlock();
        
        cv::imshow(windows_name, frame);

        if(cv::waitKey(1) == 'q') break;
    }

}


// /*参考*/
// void fastLoopCode(Mat& frameCV) {

//         //mutex mtx;   //try it in here

//         //initialize your camera etc.

//         Frame *frame;      //direct data from camera...

//         while(1){
//                 frame = camera->GetLatestFrame();      

//                 if(frame){                                     

//                         mtx.lock();
//                         //whatever code required to turn the camera data into an openCV Mat
//                         mtx.unlock();

//                         frame->Release();
//                 }
//         }
//         camera->Release();
// }


int BinoCamera::Filter_debug_task()
{
    namedWindow("Cam_R", cv::WINDOW_AUTOSIZE);  //左摄像头窗口
    namedWindow("Cam_R_binary", cv::WINDOW_AUTOSIZE);  //右摄像头窗口

    cv::Mat frame_binary;
    cv::UMat frame_labeled;
    std::vector<cv::Point> balls;

    while(1)
    {
        Cam_L.read(Frame_raw_L);
        detect_balls(Frame_raw_L, frame_labeled, frame_binary, balls);

        cv::imshow("Cam_R", frame_labeled);
        cv::imshow("Cam_R_binary", frame_binary);

        if(cv::waitKey(1) == 'q') break;
    }
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


/*******************************识别颜色小球的类*************************/
int ColoredBall::detect(cv::InputArray frame_in, cv::InputOutputArray frame_labled,
                        cv::OutputArray frame_binary, std::vector<cv::Point> &balls_detected)
{
    std::vector<std::vector<cv::Point> > country;   //轮廓线
    std::vector<cv::Vec4i> hiera;
    cv::Rect bound_box;

    /*inRange() 使用UMat会异常，因此加上中间变量Mat*/
    cv::Mat binary_Mat;
    cv::UMat frame_processing;

    /*转换为HSV色彩*/
    frame_in.copyTo(frame_processing);
    cv::cvtColor(frame_in, binary_Mat, cv::COLOR_RGB2HSV);

    /*Debug: 输出中心点的HVS值, 用来调过滤器阈值*/
    if(Filter_Debug){
        cv::circle(frame_processing, cv::Point(CAM_FRAME_HEIGHT/2, CAM_FRAME_WIDTH/2), 
                                                        10, cv::Scalar(255,0,0));
        std::cout << binary_Mat.at<cv::Vec3b>(CAM_FRAME_WIDTH/2, CAM_FRAME_HEIGHT/2) 
                                                                    << std::endl;
    }
    
    /*过滤&二值化*/
    cv::inRange(binary_Mat, Filter_HSV_LOW,
                            Filter_HSV_HIGH, frame_binary);
    cv::medianBlur(frame_binary, frame_binary, 3);
    cv::threshold(frame_binary, frame_binary, 120, 255, cv::THRESH_BINARY);

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
        cv::Scalar color = cv::Scalar(127, 127, 127);
        drawContours(frame_processing, country, (int)i, color, 2, cv::LINE_8, hiera, 0);
        /*当符合乒乓球轮廓的条件时*/
        if(area > mini_balls_size && abs(length*length/area - circle_ratio) < circle_ratio*circle_tolorance)
        {
            color = cv::Scalar(255, 0, 0);
            /*画出轮廓&标记框&圆心*/
            drawContours(frame_processing, country, (int)i, color, 2, cv::LINE_8, hiera, 0);
            bound_box = cv::boundingRect(country[i]);
            cv::rectangle(frame_processing, bound_box.tl(), bound_box.br(), cv::Scalar(0, 255, 0), 2);
            cv::circle(frame_processing, cv::Point(bound_box.x+bound_box.width/2, 
                                                bound_box.y+bound_box.height/2), 2, 
                                                cv::Scalar(0, 0, 255), 2);
            /*输出圆心数据*/
            balls_detected.push_back(cv::Point(bound_box.x+bound_box.width/2, 
                                        bound_box.y+bound_box.height/2));
            // printf("center:%d, %d\n", bound_box.x+bound_box.width/2, bound_box.y+bound_box.height/2);
        }
    }
    frame_processing.copyTo(frame_labled);


    return 0;
}


ColoredBall::ColoredBall(/* args */)
{
}

ColoredBall::~ColoredBall()
{
}