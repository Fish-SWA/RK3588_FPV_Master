#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <opencv4/opencv2/opencv.hpp>

#define _BASETSD_H

#include "RgaUtils.h"

#include "postprocess.h"

#include "rknn_api.h"
#include "preprocess.h"


#define INPUT_MAX_ATTR 1    //分配的输入张量数量
#define OUTPUT_MAX_ARRT 3   //分配的输出张量数量


class RkNPU
{
private:
    /*初始化参数*/
    int model_data_size = 0;
    unsigned char *model_data;
    rknn_sdk_version version;
    rknn_input_output_num io_num;
    rknn_tensor_attr input_attrs[1];
    rknn_tensor_attr output_attrs[3];
    rknn_input inputs[1];
    rknn_output outputs[OUTPUT_MAX_ARRT];
    int channel = 3;
    int width = 0;
    int height = 0;


    /* 内部参数 */
    rknn_context ctx;
    size_t actual_size = 0;
    int img_width = 0;
    int img_height = 0;
    int img_channel = 0;
    const float nms_threshold = NMS_THRESH;      // 默认的NMS阈值
    const float box_conf_threshold = BOX_THRESH; // 默认的置信度阈值
    struct timeval start_time, stop_time;

    /* 内部工具函数 */
    double __get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }

public:

    char *model_path;                //模型文件路径
    char *label_name_txt_path;       //标签文件路径
    int error_ret;                   //错误信息


    unsigned char *load_data(FILE *fp, size_t ofst, size_t sz);
    unsigned char *load_model(char *filename, int *model_size);
    void dump_tensor_attr(rknn_tensor_attr *attr);
    int rknn_model_init();       //载入&初始化模型
    int rknn_img_inference(cv::Mat frame, _detect_result_group_t *results);    //图像推理
    int yolo_draw_results(cv::Mat frame_in, cv::Mat& frame_labbed,
                                 _detect_result_group_t *results);     //画出识别结果
    int yolo_print_results(_detect_result_group_t *results);     //打印出识别结果

    RkNPU(/* args */);
    ~RkNPU();
};


