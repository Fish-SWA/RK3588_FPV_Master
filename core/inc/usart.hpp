/*
用于实现串口通讯打类：
 TODO
 串口监听线程-监听串口是否在线&中断实现
 串口工具
 serialWrite - 向串口发送数组
 serialPrintf



*/
#include "serial.h"
#include "stdint.h"
#include "stdio.h"
#include "tuple"
#include "thread"
#include "format"
#include "iostream"


class Usart
{
private:

public:
    const char* device_name; //串口设备名
    int bud_rate;   //波特率
    void Open(const char* name, int bud);
    Usart(const char* name, int bud);
    ~Usart();
};

