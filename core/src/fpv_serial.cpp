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