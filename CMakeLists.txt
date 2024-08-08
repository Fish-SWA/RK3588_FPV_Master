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


# rknn api
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/library/rknpu/librknn_api)
set(RKNN_RT_LIB ${RKNN_API_PATH}/aarch64/librknnrt.so)
include_directories(${RKNN_API_PATH}/include)

# yolo process
file(GLOB yolo_process ${CMAKE_SOURCE_DIR}/library/rknpu/yolo5_process/*.cc)
include_directories(library/rknpu/yolo5_process)

# rga
set(RGA_PATH ${CMAKE_SOURCE_DIR}/library/rknpu/librknn_api/rga)
set(RGA_LIB ${RGA_PATH}/lib/Linux/aarch64/librga.so)
include_directories(${RGA_PATH}/include)
include_directories(library/rknpu/librknn_api/rga/include)

# Serial
include_directories(${CMAKE_SOURCE_DIR}/library/src-serial/include/serial)

# json
include_directories(${CMAKE_SOURCE_DIR}/library)

# gst
find_package(PkgConfig)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)
include_directories(${GST_INCLUDE_DIRS})


# 编译源码生成目标
add_executable(Fpv_master
        ${src_file}
        ${yolo_process}
        )


# Link Opencv & RknnAPI & 
target_link_libraries(Fpv_master 
    ${OpenCV_LIBS}
    ${RKNN_RT_LIB}
    ${MPP_LIBS}
    ${CMAKE_SOURCE_DIR}/library/src-serial/libserial.a
    ${GST_LIBRARIES}
)

# Cam calibration tools
add_executable(Cam_calibrate 
        ${CMAKE_SOURCE_DIR}/tools/camera_cali.cpp
        )


target_link_libraries(Cam_calibrate 
    ${OpenCV_LIBS}
    ${RKNN_RT_LIB}
    ${MPP_LIBS}
    ${CMAKE_SOURCE_DIR}/library/src-serial/libserial.a
    ${GST_LIBRARIES}
)