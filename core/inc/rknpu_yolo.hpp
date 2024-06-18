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
    static unsigned char *load_data(FILE *fp, size_t ofst, size_t sz);
    static unsigned char *load_model(char *filename, int *model_size);


    RkNPU(/* args */);
    ~RkNPU();
};

RkNPU::RkNPU(/* args */)
{
}

RkNPU::~RkNPU()
{
}
