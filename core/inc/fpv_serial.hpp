#include "main.hpp"
#include "serial.h"
#include "stdint.h"
#include "stdio.h"
#include "tuple"
#include "thread"
#include "format"
#include "iostream"
#include <mutex>

/*文件写入*/
#include <fstream>

#define Sleep_ms(x) std::this_thread::sleep_for (std::chrono::milliseconds(x));

/*发送数据包的结构体*/
typedef struct
{
    int head = 0xEF;    //0xEF
    /*IMU信息*/
    float IMU_yaw;  //IMU角度
    float IMU_pitch;
    float IMU_roll;
    int IMU_av_yaw; //IMU加速度
    int IMU_av_pitch;
    int IMU_av_roll;
    /*光流信息*/
    float MTF01_roll;   //光流
    float MTF01_pitch;
    int MTF01_ToF_status;
    int MTF01_distance; //距离
    float MTF01_Px;     //位置&加速度
    float MTF01_Py;
    float MTF01_Vx;
    float MTF01_Vy;
    /*油门信息*/
    int PWM1;
    int PWM2;
    int PWM3;
    int PWM4;
    int Throttle_isLOCK;
    /*状态信息*/
    int flight_mode;
    int control_mode;
    /*遥控器信息*/
    int CrsfChannels[16];
    /*遥控器通道
    0 roll
    1 pitch
    2 throttle
    3 yaw    
    */

    int end;     //0x5A
}__attribute__((packed)) FpvPackType;

class FPV_Serial
{
private:
    serial::Serial fpv_serial;
    FpvPackType fpv_data;
    std::thread usart_listen_task;

public:
    std::mutex fpv_data_mutex;
    FpvPackType data;   //给外部读取的无人机数据
    FpvPackType last_data;

    void Serial_handle_task();
    int get_fpv_data(FpvPackType *FPV_data_recive);
    int update_fpv_data();
    FPV_Serial(const std::string device, int budrate);
    ~FPV_Serial();
};


/*------------飞控数据记录文件的类------------*/
class LogFile
{
private:
    std::ofstream log_file;
    std::string log_path =  "/home/fish/GKD/Record/log/";
public:
    std::string file_name;
    int open(std::string input_file_name);
    int close();
    int write_frist_line();
    int write_data_line(FpvPackType data);
    uint32_t get_time();
    LogFile(std::string input_file_name);
    LogFile();
    ~LogFile();
};
