# cmake 最低版本需求
cmake_minimum_required(VERSION 3.13)

# 工程名称
project(Fpv_master)

# 设置
set(CMAKE_CXX_STANDARD 11)
include_directories(${PROJECT_SOURCE_DIR}/core/inc)

# 编译源码生成目标
add_executable(Fpv_master core/src/main.cpp core/src/usart.cpp)