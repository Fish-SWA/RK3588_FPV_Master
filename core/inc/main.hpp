#include <iostream>
#include "usart.hpp"
#include <thread>
#include <chrono>

#define Sleep_ms(x) std::this_thread::sleep_for (std::chrono::milliseconds(x));