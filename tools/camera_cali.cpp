// opencv
#include "opencv2/opencv.hpp"
#include <opencv2/highgui/highgui.hpp>
// file
#include "nlohmann/json.hpp"
#include <fstream>

#define IMG_COUNT 10
#define SAMPLE_PERIOD 5

typedef struct
{
    cv::UMat frame;
    cv::UMat frame_gray;
    std::vector<cv::UMat> frame_calib;
    cv::VideoCapture cam;
    int cam_id = 0;
    int sample_period_count = 0;
}camera_cali_type;

class camrea_calibtation
{
private:
    // json
    using json = nlohmann::json;
public:
    camera_cali_type camrea;
    cv::Mat mtx, dist;
    std::vector<cv::Mat> rvecs, tvecs;

    /*************** calibration settings ****************/
    cv::Size boardSize; // 棋盘格的尺寸
    float squareSize = 20.0f; // 每格的大小，单位mm
    std::vector<std::vector<cv::Point3f>> objectPoints;
    std::vector<std::vector<cv::Point2f>> imagePoints;
    std::vector<cv::Point2f> corners;
    // 创建棋盘格的3D点
    std::vector<cv::Point3f> objP;
    
    void init(){
        /*************** cam settings ****************/
        camrea.cam.open(camrea.cam_id, cv::CAP_V4L2);
        camrea.cam.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        camrea.cam.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
        camrea.cam.set(cv::CAP_PROP_FPS, 60);
        camrea.cam.set(cv::CAP_PROP_TEMPERATURE, 5000);
        // std::cout << cam.get(cv::CAP_PROP_EXPOSURE) << std::endl;
        camrea.cam.set(cv::CAP_PROP_EXPOSURE, 100);
        camrea.cam.read(camrea.frame);
        boardSize.height = 9;
        boardSize.width = 7;

        // 创建棋盘格的3D点
        for (int i = 0; i < boardSize.height; ++i) {
            for (int j = 0; j < boardSize.width; ++j) {
                objP.push_back(cv::Point3f(j * squareSize, i * squareSize, 0));
            }
        }
    }

    int image_add(){
        camrea.cam.read(camrea.frame);
       //get calibration corners
        cv::cvtColor(camrea.frame, camrea.frame_gray, cv::COLOR_BGR2GRAY);

        if(!cv::findChessboardCorners(camrea.frame_gray, boardSize, corners)){
            return -1;
        }
        //draw corners
        cv::drawChessboardCorners(camrea.frame, boardSize, corners, true);

        /*Get sample images for calibration*/
        camrea.sample_period_count ++;
        if(camrea.sample_period_count >= SAMPLE_PERIOD){
            cv::cornerSubPix(camrea.frame_gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
            imagePoints.push_back(corners);
            std::cout << corners <<std::endl;
            objectPoints.push_back(objP);

            //get calibration imgs
            camrea.sample_period_count = 0;
            camrea.frame_calib.push_back(camrea.frame);
            printf("get image, idx = %ld / %d\n", camrea.frame_calib.size(), IMG_COUNT);
            if(camrea.frame_calib.size() >= (IMG_COUNT-1)){
                return 1;
            }
        }

        return 0;

    }

    void calculate(){
        // 执行相机标定
        for(int i=0; i<objectPoints.size(); i++){
            std::cout << objectPoints[i] << std::endl;
        }
        printf("%d, %d, %d\n", objectPoints.size(), imagePoints.size(), camrea.frame_gray.size());
        cv::calibrateCamera(objectPoints, imagePoints, cv::Size(camrea.frame_gray.rows, camrea.frame_gray.cols), 
                            mtx, dist, rvecs, tvecs);
    }

    void save(const std::string& filename){
        // 输出相机参数
        writeCameraParametersToJson(mtx, dist, filename);
        std::cout << "Camera Matrix:\n" << mtx << std::endl;
        std::cout << "Distortion Coefficients:\n" << dist << std::endl;
    }

    // 将cameraMatrix和distCoeffs写入到JSON文件
    void writeCameraParametersToJson(const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs, const std::string& filename) {
        json j;

        for (int i = 0; i < cameraMatrix.rows; ++i) {
            for (int k = 0; k < cameraMatrix.cols; ++k) {
                j["cameraMatrix"][i][k] = cameraMatrix.at<double>(i, k);
            }
        }

        for (int i = 0; i < distCoeffs.rows; ++i) {
            for (int k = 0; k < distCoeffs.cols; ++k) {
                j["distCoeffs"][i][k] = distCoeffs.at<double>(i, k);
            }
        }

        std::ofstream file(filename);
        if (!file) {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
            return;
        }
        file << j.dump(4);  // formatted with indent 4
        if (file.fail()) {
            std::cerr << "Error writing to file: " << filename << std::endl;
        }
        file.close();
    }


    // 从JSON文件读取cameraMatrix和distCoeffs
    void readCameraParametersFromJson(const std::string& filename, cv::Mat& cameraMatrix, cv::Mat& distCoeffs) {
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

    // camrea_calibtation(/* args */);
    // ~camrea_calibtation();
};

camrea_calibtation cam_l;
int return_num = 0;

int main()
{
    
    cam_l.camrea.cam_id = 0;
    cam_l.init();

    while(1)
    {
        cv::imshow("cam", cam_l.camrea.frame);
        
        if(cv::waitKey(1) == 'q') break;

        return_num = cam_l.image_add();
        if(return_num == -1) continue;
        else if(return_num == 1) break;

    }

    // 执行相机标定
    cam_l.calculate();

    // 输出相机参数
    cam_l.save("/home/fish/GKD/RK3588_FPV_Master/settings/camera_paramets.json");    

    return 0;
}

