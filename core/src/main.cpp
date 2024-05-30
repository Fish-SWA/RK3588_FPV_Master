#include <iostream>
#include "main.hpp"
#include "usart.hpp"
#include <thread>

using namespace std;
char readbuffer[10];

Usart usart0("/dev/ttyACM0", 115200);

int main()
{
    std::cout << usart0.device_name << std::endl;
    while(1) 
    {
        usart0.serialPrintf("naive!");
        //usart0.serialPrintf("%f\n", 114.514);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << usart0.serialDataReady() <<std::endl;
        if(usart0.serialDataReady() > 0){
            std::cout << usart0.serialGetChar() << std::endl;
        }
    }
    return 0;
}