#include <iostream>
#include "main.hpp"
#include "usart.hpp"

using namespace std;

Usart usart0;

int main()
{
    usart0.init("/dev/ACM0", 115200);
    std::cout << usart0.bud_rate << std::endl;
    return 0;
}