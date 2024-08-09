// opencv
#include "opencv2/opencv.hpp"
#include <opencv2/highgui/highgui.hpp>
// file
#include "nlohmann/json.hpp"
#include <fstream>
//thread
#include <thread>
//PCL
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>

using json = nlohmann::json;

void readCameraParameters(const std::string& filename, cv::Mat& cameraMatrix, cv::Mat& distCoeffs);
void readRTEF(const std::string& filename,
                                  cv::Mat& R, cv::Mat& T,
                                  cv::Mat& E, cv::Mat& F);

// 从JSON文件读取cameraMatrix和distCoeffs
void readCameraParameters(const std::string& filename, cv::Mat& cameraMatrix, cv::Mat& distCoeffs) {
    std::ifstream file(filename);
    json j;
    file >> j;

    // 创建相应大小的矩阵
    cameraMatrix = cv::Mat(3, 3, CV_64F);
    int distCoeffsSize = j["distCoeffs"].size() * j["distCoeffs"][0].size();
    distCoeffs = cv::Mat(distCoeffsSize, 1, CV_64F);

    // 读取cameraMatrix
    for (int i = 0; i < cameraMatrix.rows; ++i) {
        for (int k = 0; k < cameraMatrix.cols; ++k) {
            cameraMatrix.at<double>(i, k) = j["cameraMatrix"][i][k];
        }
    }

    // 读取distCoeffs
    for (int i = 0; i < distCoeffs.rows; ++i) {
        distCoeffs.at<double>(i) = j["distCoeffs"][0][i];
    }
}

void readRTEF(const std::string& filename,
                                  cv::Mat& R, cv::Mat& T,
                                  cv::Mat& E, cv::Mat& F) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

    json j;
    file >> j;

    // 读取旋转矩阵 R
    R = cv::Mat(3, 3, CV_64F);
    for (int i = 0; i < 3; ++i) {
        for (int k = 0; k < 3; ++k) {
            R.at<double>(i, k) = j["R"][i][k];
        }
    }

    // 读取平移向量 T
    T = cv::Mat(3, 1, CV_64F);
    for (int i = 0; i < 3; ++i) {
        T.at<double>(i) = j["T"][i][0];
    }

    // 读取本质矩阵 E
    E = cv::Mat(3, 3, CV_64F);
    for (int i = 0; i < 3; ++i) {
        for (int k = 0; k < 3; ++k) {
            E.at<double>(i, k) = j["E"][i][k];
        }
    }

    // 读取基本矩阵 F
    F = cv::Mat(3, 3, CV_64F);
    for (int i = 0; i < 3; ++i) {
        for (int k = 0; k < 3; ++k) {
            F.at<double>(i, k) = j["F"][i][k];
        }
    }
}

typedef struct
{
    cv::Mat mtx;
    cv::Mat dist;
    cv::VideoCapture cam;
    cv::Mat frame;
}camera_para_type;


typedef struct
{
    camera_para_type cam_l;
    camera_para_type cam_r;
    cv::Mat R;
    cv::Mat T;
    cv::Mat E;
    cv::Mat F;
    
}bi_camera_type;

bi_camera_type camreas;

int main() 
{
    //read parameters
    readCameraParameters("/home/fish/GKD/RK3588_FPV_Master/settings/cameraL_paramets.json", camreas.cam_l.mtx, camreas.cam_l.dist);
    readCameraParameters("/home/fish/GKD/RK3588_FPV_Master/settings/cameraR_paramets.json", camreas.cam_r.mtx, camreas.cam_r.dist);
    readRTEF("/home/fish/GKD/RK3588_FPV_Master/settings/cameraRL_RTEF.json", camreas.R, camreas.T, camreas.E, camreas.F);
    std::cout << camreas.T << "----T\n" << camreas.cam_l.mtx << std::endl; 

    //cam_l init
    camreas.cam_l.cam.open(0, cv::CAP_V4L2);
    camreas.cam_l.cam.set(cv::CAP_PROP_FRAME_WIDTH, 800);
    camreas.cam_l.cam.set(cv::CAP_PROP_FRAME_HEIGHT, 600);
    camreas.cam_l.cam.set(cv::CAP_PROP_FPS, 60);
    camreas.cam_l.cam.set(cv::CAP_PROP_TEMPERATURE, 5000);
    camreas.cam_l.cam.set(cv::CAP_PROP_EXPOSURE, 150);
    camreas.cam_l.cam.set(cv::CAP_PROP_AUTO_WB, 1);
    std::cout << camreas.cam_l.cam.get(cv::CAP_PROP_AUTO_WB) << std::endl;
    camreas.cam_l.cam.read(camreas.cam_l.frame);
    //cam_r init
    camreas.cam_r.cam.open(2, cv::CAP_V4L2);
    camreas.cam_r.cam.set(cv::CAP_PROP_FRAME_WIDTH, 800);
    camreas.cam_r.cam.set(cv::CAP_PROP_FRAME_HEIGHT, 600);
    camreas.cam_r.cam.set(cv::CAP_PROP_FPS, 60);
    camreas.cam_r.cam.set(cv::CAP_PROP_TEMPERATURE, 5000);
    camreas.cam_r.cam.set(cv::CAP_PROP_EXPOSURE, 150);
    camreas.cam_r.cam.read(camreas.cam_r.frame);

    // 立体校正
    cv::Mat R1, R2, P1, P2, Q;
    cv::stereoRectify(camreas.cam_l.mtx, camreas.cam_l.dist, camreas.cam_r.mtx, camreas.cam_r.dist, camreas.cam_l.frame.size(),
                    camreas.R, camreas.T, R1, R2, P1, P2, Q);

    // 获取映射
    cv::Mat map1L, map2L, map1R, map2R;
    cv::initUndistortRectifyMap(camreas.cam_l.mtx, camreas.cam_l.dist, R1, P1, camreas.cam_l.frame.size(), CV_16SC2, map1L, map2L);
    cv::initUndistortRectifyMap(camreas.cam_r.mtx, camreas.cam_r.dist, R2, P2, camreas.cam_l.frame.size(), CV_16SC2, map1R, map2R);

    // 3D映射
    cv::Mat imgL_rect, imgR_rect;
    cv::Mat depth;

    // 点云数据
    cv::Mat points3D;

    // 创建StereoSGBM对象
    int minDisparity = 16;              //视差搜索起点
    int numDisparities = 208 -16;       //视差搜索范围 必须是16的倍数
    int blockSize = 9;                  //blocksize，最好3~11
    cv::Ptr<cv::StereoSGBM> sgbm = cv::StereoSGBM::create(minDisparity, numDisparities, blockSize);

    // 配置StereoSGBM参数
    sgbm->setP1(8 * blockSize * blockSize);         //视差平滑参数P1
    sgbm->setP2(32 * blockSize * blockSize);        //视差平滑参数P2
    sgbm->setPreFilterCap(15);                      //截断图像像素强度的值（高->降噪&减细节）
    sgbm->setUniquenessRatio(10);                   //最差匹配视差比第二匹配视差好()%被启用
    sgbm->setSpeckleWindowSize(100);                //检测斑点窗口大小（噪点过滤）
    sgbm->setSpeckleRange(32);                      //视差变化允许范围（噪点过滤）
    sgbm->setDisp12MaxDiff(1);                      //左右一致性检查最大允许差异
    sgbm->setMode(cv::StereoSGBM::MODE_SGBM_3WAY);

    //PCL点云数据
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

    cv::Mat disparity;

    while (1)
    {
        camreas.cam_l.cam.read(camreas.cam_l.frame);
        camreas.cam_r.cam.read(camreas.cam_r.frame);

        cv::cvtColor(camreas.cam_l.frame, camreas.cam_l.frame, cv::COLOR_BGR2GRAY);
        cv::cvtColor(camreas.cam_r.frame, camreas.cam_r.frame, cv::COLOR_BGR2GRAY);

        cv::remap(camreas.cam_l.frame, imgL_rect, map1L, map2L, cv::INTER_LINEAR);
        cv::remap(camreas.cam_r.frame, imgR_rect, map1R, map2R, cv::INTER_LINEAR);

        // 计算视差
        sgbm->compute(imgL_rect, imgR_rect, disparity);

        // 转换为深度图
        cv::Mat depth;
        cv::reprojectImageTo3D(disparity, depth, Q, true);
        // 增加伪色
        cv::Mat falseColorsMap;
        cv::normalize(disparity, falseColorsMap, 0, 255, cv::NORM_MINMAX, CV_8UC1);
        cv::applyColorMap(falseColorsMap, falseColorsMap, cv::COLORMAP_JET);

        // 转换为点云
        cv::reprojectImageTo3D(disparity, points3D, Q, true);
        std::cout << points3D.size() << std::endl;
        //输出部分点云
        //输出区域
        int x_start = 300, y_start = 250;
        int width = 50, height = 50;

        //绘制输出区域到深度图上
        cv::rectangle(falseColorsMap, cv::Point(x_start, y_start), cv::Point(x_start + width, y_start + height), cv::Scalar(0, 255, 0), 2);

        // 输出部分点云
        for (int i = y_start; i < y_start + height; i++) {
            for (int j = x_start; j < x_start + width; j++) {
                cv::Point3f point = points3D.at<cv::Point3f>(i, j);
                if(std::isfinite(point.z) && point.z < 5000){  // 筛选有效Z值
                    std::cout << "3D Point(" << i << ", " << j << "): " << point << std::endl;
                }
            }
        }
        // 设置测距无效部分的伪色图为灰色
        cv::Scalar grayColor(255, 255, 255); // 灰色
        // 设置无效区域为灰色
        for (int i = 0; i < points3D.rows; i++) {
            for (int j = 0; j < points3D.cols; j++) {
                cv::Point3f point = points3D.at<cv::Point3f>(i, j);
                if (point.z > 5000) {
                    // falseColorsMap.at<cv::Vec3b>(i, j) 
                    falseColorsMap.data[i, j, 0] = grayColor[0];
                    falseColorsMap.data[i, j, 1] = grayColor[1];
                    falseColorsMap.data[i, j, 2] = grayColor[2];
                }
            }
        }
        printf("------------------------------------\n");

        /*****************点云可视化********************/
        // 填充点云数据
        for (int i = 0; i < points3D.rows; i++) {
            for (int j = 0; j < points3D.cols; j++) {
                cv::Vec3f point = points3D.at<cv::Vec3f>(i, j);
                // 确保点是有效的
                if (std::isfinite(point[2])) {
                    cloud->push_back(pcl::PointXYZ(point[0], point[1], point[2]));
                }
            }
        }
        // 可视化
        pcl::visualization::PCLVisualizer::Ptr viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
        viewer->setBackgroundColor(0, 0, 0);
        pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> single_color(cloud, 0, 255, 0);  // 绿色
        viewer->addPointCloud<pcl::PointXYZ>(cloud, single_color, "sample cloud");
        viewer->addCoordinateSystem(1.0);
        viewer->initCameraParameters();

        cv::imshow("camL", camreas.cam_l.frame);
        cv::imshow("camR", camreas.cam_r.frame);
        cv::imshow("depth", falseColorsMap);

        /*****暂时先这样******/
        while (!viewer->wasStopped()) {
            viewer->spinOnce(100);
        }

        if(cv::waitKey(1) == 'q') break;
    }
    


    return 0;
}


// //Ref
// int main() {
//     // 假设你已经加载了所有必要的参数和图像
//     cv::Mat imgL, imgR; // 左右图像
//     cv::Mat cameraMatrixL, distL, cameraMatrixR, distR; // 相机内参数和畸变参数
//     cv::Mat R, T, E, F; // 标定得到的外部参数

//     cv::Size imageSize = imgL.size(); // 图像尺寸

//     // 立体校正
//     cv::Mat R1, R2, P1, P2, Q;
//     cv::stereoRectify(cameraMatrixL, distL, cameraMatrixR, distR, imageSize, R, T, R1, R2, P1, P2, Q);

//     // 获取映射
//     cv::Mat map1L, map2L, map1R, map2R;
//     cv::initUndistortRectifyMap(cameraMatrixL, distL, R1, P1, imageSize, CV_16SC2, map1L, map2L);
//     cv::initUndistortRectifyMap(cameraMatrixR, distR, R2, P2, imageSize, CV_16SC2, map1R, map2R);

//     // 应用映射
//     cv::Mat imgL_rect, imgR_rect;
//     cv::remap(imgL, imgL_rect, map1L, map2L, cv::INTER_LINEAR);
//     cv::remap(imgR, imgR_rect, map1R, map2R, cv::INTER_LINEAR);

//     // 创建StereoBM对象
//     cv::Ptr<cv::StereoBM> stereoBM = cv::StereoBM::create(16, 9);
//     cv::Mat disparity;
//     stereoBM->compute(imgL_rect, imgR_rect, disparity);

//     // 转换为深度图（视情况使用）
//     cv::Mat depth;
//     cv::reprojectImageTo3D(disparity, depth, Q, true);

//     // 显示结果
//     cv::imshow("Disparity", disparity);
//     cv::waitKey(0);

//     return 0;
// }