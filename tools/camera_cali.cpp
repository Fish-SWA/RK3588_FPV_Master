// opencv
#include "opencv2/opencv.hpp"
#include <opencv2/highgui/highgui.hpp>
// file
#include "nlohmann/json.hpp"
#include <fstream>
//thread
#include <thread>


#define Sleep_ms(x) std::this_thread::sleep_for (std::chrono::milliseconds(x));

#define IMG_COUNT 60
#define SAMPLE_PERIOD 5

void writeRTEF(cv::Mat& R, cv::Mat& T,
                cv::Mat& E, cv::Mat& F,
                const std::string& filename);


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
        camrea.cam.set(cv::CAP_PROP_FRAME_WIDTH, 800);
        camrea.cam.set(cv::CAP_PROP_FRAME_HEIGHT, 600);
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

    void get(){
        camrea.cam.read(camrea.frame);
    }

    int check(){
        
       //get calibration corners
        cv::cvtColor(camrea.frame, camrea.frame_gray, cv::COLOR_BGR2GRAY);

        if(!cv::findChessboardCorners(camrea.frame_gray, boardSize, corners)){
            return -1;
        }
        //draw corners
        cv::drawChessboardCorners(camrea.frame, boardSize, corners, true);
        return 0;
    }

    int image_add(){
        /*Get sample images for calibration*/
        camrea.sample_period_count ++;
        if(camrea.sample_period_count >= SAMPLE_PERIOD){
            cv::cornerSubPix(camrea.frame_gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
            imagePoints.push_back(corners);
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

camrea_calibtation cam_l, cam_r;
int return_num_l = 0;
int return_num_r = 0;


int main()
{
    
    cam_l.camrea.cam_id = 0;
    cam_l.init();
    cam_r.camrea.cam_id = 2;
    cam_r.init();

    while(1)
    {
        cv::imshow("camL", cam_l.camrea.frame);
        cv::imshow("camR", cam_r.camrea.frame);
        
        if(cv::waitKey(1) == 'q') break;

        cam_l.get();
        cam_r.get();
        return_num_l = cam_l.check();
        return_num_r = cam_r.check();
        if(return_num_l == -1 || return_num_r == -1){
            continue;
        }
        
        return_num_l = cam_l.image_add();
        return_num_r = cam_r.image_add();
        if((return_num_l == 1) || (return_num_r == 1)){
            break;
        }

        // return_num = cam_l.check();
        // if(return_num == -1) continue;
        // else if(return_num == 1) break;

        // return_num = cam_r.check();
        // if(return_num == -1) continue;
        // else if(return_num == 1) break;


    }

    // // show frames
    // for(int i=0; i<IMG_COUNT; i++){
    //     cv::imshow("camL", cam_l.camrea.frame_calib[i]);
    //     cv::imshow("camR", cam_r.camrea.frame_calib[i]);
    //     Sleep_ms(1000);
    //     if(cv::waitKey(1) == 'q') break;

    // }

    // 执行相机标定
    cam_l.calculate();
    cam_r.calculate();


    // 输出相机参数
    cam_l.save("/home/fish/GKD/RK3588_FPV_Master/settings/cameraL_paramets.json");    
    cam_r.save("/home/fish/GKD/RK3588_FPV_Master/settings/cameraR_paramets.json");   

    // stereoRectify
    cv::Mat R, T, E, F;
    cv::stereoCalibrate(cam_l.objectPoints, cam_l.imagePoints, cam_r.imagePoints,
                    cam_l.mtx, cam_l.dist, cam_r.mtx, cam_r.dist,
                    cv::Size(cam_l.camrea.frame_gray.rows, cam_l.camrea.frame_gray.cols),
                    R, T, E, F);
    printf("-------------------------------\n");
    std::cout << R << "--R\n" << T << "--T\n" << E << "--E\n" << F << "--F\n" <<std::endl;
    writeRTEF(R, T, E, F, "/home/fish/GKD/RK3588_FPV_Master/settings/cameraRL_RTEF.json");
    return 0;
}

void writeRTEF(cv::Mat& R, cv::Mat& T,
                cv::Mat& E, cv::Mat& F,
                const std::string& filename) {
    using json = nlohmann::json;
    json j;

    // R
    for (int i = 0; i < R.rows; ++i) {
        for (int k = 0; k < R.cols; ++k) {
            j["R"][i][k] = R.at<double>(i, k);
        }
    }

    // T
    for (int i = 0; i < T.rows; ++i) {
        for (int k = 0; k < T.cols; ++k) {
            j["T"][i][k] = T.at<double>(i, k);
        }
    }

    // 保存本质矩阵
    for (int i = 0; i < E.rows; ++i) {
        for (int k = 0; k < E.cols; ++k) {
            j["E"][i][k] = E.at<double>(i, k);
        }
    }

    // 保存基本矩阵
    for (int i = 0; i < F.rows; ++i) {
        for (int k = 0; k < F.cols; ++k) {
            j["F"][i][k] = F.at<double>(i, k);
        }
    }

    // 写入到文件
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }
    file << j.dump(4);  // 格式化输出，缩进为4
    if (file.fail()) {
        std::cerr << "Error writing to file: " << filename << std::endl;
    }
    file.close();
}