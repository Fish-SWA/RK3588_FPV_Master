#include "main.hpp"
#include "vision.hpp"
#include "rknpu_yolo.hpp"

using namespace cv;

void Opencl_init();

// BinoCamera BinoPair(0, 2, 140); //双目摄像头对象


RkNPU Rkyolo;


int main()
{
    Opencl_init();  //初始化OpenCL

    //模型路径
    Rkyolo.model_path = "/home/fish/GKD/RK3588_FPV_Master/model/yolov5s-640-640.rknn";
    //标签列表路径
    Rkyolo.label_name_txt_path = "/home/fish/GKD/RK3588_FPV_Master/model/coco_80_labels_list.txt";

    Rkyolo.rknn_model_init();  //初始化RKNN模型

    cv::VideoCapture Cam;
    cv::Mat frame_cam;

    Cam.open(0, cv::CAP_V4L2);

    Cam.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );
    Cam.set(cv::CAP_PROP_FPS, 60);
    Cam.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    Cam.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    Cam.set(cv::CAP_PROP_BRIGHTNESS, 10);
    Cam.set(cv::CAP_PROP_TEMPERATURE, 7000);

    cv::namedWindow("yolo_cam", cv::WINDOW_AUTOSIZE);

  while(1){
  
  Cam.read(frame_cam);
  
  _detect_result_group_t detect_result;

  Rkyolo.rknn_img_inference(frame_cam, &detect_result);
  
  Rkyolo.yolo_draw_results(frame_cam, frame_cam, &detect_result);

  Rkyolo.yolo_print_results(&detect_result);

  cv::imshow("yolo_cam", frame_cam);

  if(cv::waitKey(1) == 'q') break;

  }

    // BinoPair.open();    //开启摄像头

    //BinoPair.monitor_task();
    
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