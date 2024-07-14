# cmake 最低版本需求
cmake_minimum_required(VERSION 3.13)

# 工程名称
project(Fpv_master)

# 设置
set(CMAKE_CXX_STANDARD 11)
include_directories(${PROJECT_SOURCE_DIR}/core/inc)

# Opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# find src
file(GLOB src_file ${CMAKE_SOURCE_DIR}/core/src/*.cpp)

# Serial
include_directories(${CMAKE_SOURCE_DIR}/library/src-serial/include/serial)

# 编译源码生成目标
add_executable(Fpv_master
        ${src_file}
        ${yolo_process}
        )

# Link Opencv & RknnAPI & 
target_link_libraries(Fpv_master 
    ${OpenCV_LIBS}
    ${CMAKE_SOURCE_DIR}/library/src-serial/build/libserial.a
)