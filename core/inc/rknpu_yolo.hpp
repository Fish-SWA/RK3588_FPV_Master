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


class RkNPU
{
private:
    /* data */



    double __get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }

public:
    unsigned char *load_data(FILE *fp, size_t ofst, size_t sz);
    unsigned char *load_model(char *filename, int *model_size);
    void dump_tensor_attr(rknn_tensor_attr *attr);
    int debug_main(); 


    // RkNPU(/* args */);
    // ~RkNPU();
};

// RkNPU::RkNPU(/* args */)
// {
// }

// RkNPU::~RkNPU()
// {
// }
