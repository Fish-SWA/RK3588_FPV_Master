#include "usart.hpp"

int Usart::init(const char* name, int bud)
{
    device_name = name;
    bud_rate = bud;
    return 0;
}