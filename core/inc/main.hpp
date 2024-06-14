#include <iostream>
#include "usart.hpp"
#include <thread>

#define Sleep_ms(x) std::this_thread::sleep_for (std::chrono::milliseconds(x));