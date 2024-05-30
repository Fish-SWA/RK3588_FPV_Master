/*
用于实现串口通讯打类：
 TODO
 串口监听线程-监听串口是否在线&中断实现
 串口工具
 serialWrite - 向串口发送数组
 serialPrintf



*/



class Usart
{
private:
    int fd; //设备文件指针
    enum{
    SERIAL_OK = 0,
    SERIAL_FAIL
};
public:
    const char* device_name; //串口设备名
    int bud_rate;   //波特率
    int serial_state = SERIAL_OK;   //串口状态
    int serialOpen();
    int serialWrite(const char* data, int length);
    int serialDataReady();
    int serialRead(void *data_read, int length);
    int serialGetChar();
    void serialPrintf (const char *message, ...);
    Usart(const char* name, int bud);
    ~Usart();
};

