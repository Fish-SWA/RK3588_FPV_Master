class Usart
{
private:
    /* data */
public:
    // usart(/* args */);
    // ~usart();
    const char* device_name; //串口设备名
    int bud_rate;   //波特率
    int init(const char* name, int bud);
};