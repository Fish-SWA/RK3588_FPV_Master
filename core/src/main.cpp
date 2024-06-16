#include "main.hpp"
#include "vision.hpp"

using namespace cv;

void Opencl_init();

BinoCamera BinoPair(0, 2, 140); //双目摄像头对象

int main()
{
    Opencl_init();  //初始化OpenCL

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