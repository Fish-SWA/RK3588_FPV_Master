#include "fpv_serial.hpp"
#include <thread>
#include <functional>

FPV_Serial::FPV_Serial(const std::string device, int budrate)
{
    serial::Timeout timeout = serial::Timeout::simpleTimeout(1000);

    fpv_serial.setBaudrate(budrate);
    fpv_serial.setPort(device);
    fpv_serial.setTimeout(timeout);
    fpv_serial.open();

    usart_listen_task = std::thread(std::bind(&FPV_Serial::Serial_handle_task, this));
}

void FPV_Serial::Serial_handle_task()
{
    uint8_t head;

    while(1){
        if(!fpv_serial.available()){
            continue;
        }

        if(fpv_serial.read(&head, 1) == 0 || head != fpv_data.head){
            continue;
        }
        fpv_data_mutex.lock();
        fpv_serial.read((uint8_t *)&fpv_data + sizeof(uint8_t), sizeof(fpv_data)-1);
        fpv_data_mutex.unlock();
        Sleep_ms(1);    //防止占过多CPU

        /*TEST*/        
        // printf("yaw:%f\tpitch:%f\troll:%f\n", fpv_data.IMU_yaw,
        //                         fpv_data.IMU_pitch, fpv_data.IMU_roll);

    }
}

int FPV_Serial::get_fpv_data(FpvPackType *FPV_data_recive)
{
    fpv_data_mutex.lock();
    memcpy(FPV_data_recive, &fpv_data, sizeof(FpvPackType));
    fpv_data_mutex.unlock();
    return 0;
}

int FPV_Serial::update_fpv_data()
{
    fpv_data_mutex.lock();
    memcpy(&last_data, &data, sizeof(FpvPackType));
    memcpy(&data, &fpv_data, sizeof(FpvPackType));
    fpv_data_mutex.unlock();
    return 0;
}

FPV_Serial::~FPV_Serial()
{
    if (usart_listen_task.joinable()) {
        usart_listen_task.join();
    }
}



/*****LogFile******/

int LogFile::write_frist_line()
{
    log_file << 
    "Time,IMU_yaw,IMU_pitch,IMU_roll,IMU_av_yaw,IMU_av_pitch,IMU_av_roll,\
    MTF01_roll,MTF01_pitch,MTF01_ToF_status,MTF01_distance,\
    MTF01_Px,MTF01_Py,MTF01_Vx,MTF01_Vy,\
    PWM1,PWM2,PWM3,PWM4,\
    Throttle_isLOCK,flight_mode,control_mode\
    CrsfChannels[0],\
    CrsfChannels[1],\
    CrsfChannels[2],\
    CrsfChannels[3],\
    CrsfChannels[4],\
    CrsfChannels[5],\
    CrsfChannels[6],\
    CrsfChannels[7],\
    CrsfChannels[8],\
    CrsfChannels[9],\
    CrsfChannels[10],\
    CrsfChannels[11],\
    CrsfChannels[12],\
    CrsfChannels[13],\
    CrsfChannels[14],\
    CrsfChannels[15]," << std::endl;
    return 0;
}

int LogFile::write_data_line(FpvPackType data)
{
    log_file <<
    get_time() << "," <<
    data.IMU_yaw << "," <<
    data.IMU_pitch << "," <<
    data.IMU_roll << "," <<
    data.IMU_av_yaw << "," <<
    data.IMU_av_pitch << "," <<
    data.IMU_av_roll << "," <<
    data.MTF01_roll << "," <<
    data.MTF01_pitch << "," <<
    data.MTF01_ToF_status << "," <<
    data.MTF01_distance << "," <<
    data.MTF01_Px << "," <<
    data.MTF01_Py << "," <<
    data.MTF01_Vx << "," <<
    data.MTF01_Vy << "," <<
    data.PWM1 << "," <<
    data.PWM2 << "," <<
    data.PWM3 << "," <<
    data.PWM4 << "," <<
    data.Throttle_isLOCK << "," <<
    data.flight_mode << "," <<
    data.control_mode << "," <<
    data.CrsfChannels[0] << "," <<
    data.CrsfChannels[1] << "," <<
    data.CrsfChannels[2] << "," <<
    data.CrsfChannels[3] << "," <<
    data.CrsfChannels[4] << "," <<
    data.CrsfChannels[5] << "," <<
    data.CrsfChannels[6] << "," <<
    data.CrsfChannels[7] << "," <<
    data.CrsfChannels[8] << "," <<
    data.CrsfChannels[9] << "," <<
    data.CrsfChannels[10] << "," <<
    data.CrsfChannels[11] << "," <<
    data.CrsfChannels[12] << "," <<
    data.CrsfChannels[13] << "," <<
    data.CrsfChannels[14] << "," <<
    data.CrsfChannels[15] << "," << std::endl;
    // for(int i=0; i<16; i++){
    //     log_file << data.CrsfChannels[i] << ",";
    // }

    // log_file << std::endl;
    return 0;
}

int LogFile::open(std::string input_file_name)
{
    file_name = input_file_name;
    log_file.open(log_path + file_name, std::ios::app);
    write_frist_line();
    return 0;
}

uint32_t LogFile::get_time()
{
std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>
      tp = std::chrono::time_point_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}

int LogFile::close()
{
    log_file.close();
    return 0;
}

LogFile::LogFile(std::string input_file_name)
{
    file_name = input_file_name;
    log_file.open(log_path + file_name, std::ios::app);
    write_frist_line();
}

LogFile::LogFile()
{

}

LogFile::~LogFile()
{
    log_file.close();
}