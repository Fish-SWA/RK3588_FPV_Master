#include "main.hpp"
#include "vision.hpp"

//OpenCV
#include "opencv2/opencv.hpp"

using namespace cv;

BinoCamera BinoPair(0, 2, 140); //双目摄像头对象

Mat frame;

int main()
{
    BinoPair.open();    //开启摄像头

    BinoPair.monitor_task();
    
    // while(1) 
    // {
    //     BinoPair.Cam_L.read(frame);
    //     cvtColor(frame, frame, COLOR_RGB2GRAY);
    //     threshold(frame, frame, 120, 255, THRESH_BINARY);
    //     imshow("cam_l", frame);

    //     if(waitKey(1) == 'q') break;
    // }
    return 0;
}