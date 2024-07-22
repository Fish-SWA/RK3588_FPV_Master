#include "rknpu_yolo.hpp"

using namespace cv;
using namespace std;

unsigned char *RkNPU::load_data(FILE *fp, size_t ofst, size_t sz)
{
  unsigned char *data;
  int ret;

  data = NULL;

  if (NULL == fp)
  {
    return NULL;
  }

  ret = fseek(fp, ofst, SEEK_SET);
  if (ret != 0)
  {
    printf("blob seek failure.\n");
    return NULL;
  }

  data = (unsigned char *)malloc(sz);
  if (data == NULL)
  {
    printf("buffer malloc failure.\n");
    return NULL;
  }
  ret = fread(data, 1, sz, fp);
  return data;
}

unsigned char *RkNPU::load_model(char *filename, int *model_size)
{
  FILE *fp;
  unsigned char *data;

  fp = fopen(filename, "rb");
  if (NULL == fp)
  {
    printf("Open file %s failed.\n", filename);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);

  data = load_data(fp, 0, size);

  fclose(fp);

  *model_size = size;
  return data;
}

void RkNPU::dump_tensor_attr(rknn_tensor_attr *attr)
{
  std::string shape_str = attr->n_dims < 1 ? "" : std::to_string(attr->dims[0]);
  for (int i = 1; i < attr->n_dims; ++i)
  {
    shape_str += ", " + std::to_string(attr->dims[i]);
  }

  printf("  index=%d, name=%s, n_dims=%d, dims=[%s], n_elems=%d, size=%d, w_stride = %d, size_with_stride=%d, fmt=%s, "
         "type=%s, qnt_type=%s, "
         "zp=%d, scale=%f\n",
         attr->index, attr->name, attr->n_dims, shape_str.c_str(), attr->n_elems, attr->size, attr->w_stride,
         attr->size_with_stride, get_format_string(attr->fmt), get_type_string(attr->type),
         get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}

int RkNPU::rknn_model_init()
{
  /*------载入模型-------*/
  int model_data_size = 0;
  model_data = load_model(model_path, &model_data_size);
  error_ret = rknn_init(&ctx, model_data, model_data_size, 0, NULL);
  if (error_ret < 0)  return error_ret;


  /*------查询信息------*/
  /*查询版本*/
  error_ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
  if (error_ret < 0)  return error_ret;

  /*输入&输出的维度*/
  error_ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
  if (error_ret < 0)  return error_ret;
  // printf("in:%d, ou:%d\n", io_num.n_input, io_num.n_output);

  /*输入向量*/
  memset(input_attrs, 0, sizeof(input_attrs));
  for (int i = 0; i < io_num.n_input; i++)
  {
    input_attrs[i].index = i;
    error_ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
    if (error_ret < 0)  return error_ret;
    // dump_tensor_attr(&(input_attrs[i]));
  }

  /*输出向量*/
  memset(output_attrs, 0, sizeof(output_attrs));
  for (int i = 0; i < io_num.n_output; i++)
  {
    output_attrs[i].index = i;
    error_ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
    if (error_ret < 0)  return error_ret;
    // dump_tensor_attr(&(output_attrs[i]));
  }

  if (input_attrs[0].fmt == RKNN_TENSOR_NCHW) //NCHW input fmt
  {
    channel = input_attrs[0].dims[1];
    height = input_attrs[0].dims[2];
    width = input_attrs[0].dims[3];
  }
  else    //HWC input fmt
  {
    height = input_attrs[0].dims[1];
    width = input_attrs[0].dims[2];
    channel = input_attrs[0].dims[3];
  }

  memset(inputs, 0, sizeof(inputs));
  inputs[0].index = 0;
  inputs[0].type = RKNN_TENSOR_UINT8;
  inputs[0].size = width * height * channel;
  inputs[0].fmt = RKNN_TENSOR_NHWC;
  inputs[0].pass_through = 0;

  return 0;
}

int RkNPU::rknn_img_inference(cv::Mat frame, _detect_result_group_t *results)
{
  /*-------预处理-------*/
  cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
  img_width = frame.cols;
  img_height = frame.rows;

  BOX_RECT pads;
  memset(&pads, 0, sizeof(BOX_RECT));
  cv::Size target_size(width, height);
  cv::Mat resized_img(target_size.height, target_size.width, CV_8UC3);
  //计算缩放比例
  float scale_w = (float)target_size.width / frame.cols;
  float scale_h = (float)target_size.height / frame.rows;

  if (img_width != width || img_height != height)
  {
      float min_scale = std::min(scale_w, scale_h);
      scale_w = min_scale;
      scale_h = min_scale;
      letterbox(frame, resized_img, pads, min_scale, target_size);
    inputs[0].buf = resized_img.data;
  }
  else
  {
    inputs[0].buf = frame.data;
  }

  /*--------输入数据&执行推理---------*/
  rknn_inputs_set(ctx, io_num.n_input, inputs);

  memset(outputs, 0, sizeof(outputs));
  for (int i = 0; i < io_num.n_output; i++)
  {
    outputs[i].want_float = 0;
  }

  error_ret = rknn_run(ctx, NULL);
  error_ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);

  /*--------后处理---------*/
  std::vector<float> out_scales;
  std::vector<int32_t> out_zps;
  for (int i = 0; i < io_num.n_output; ++i)
  {
    out_scales.push_back(output_attrs[i].scale);
    out_zps.push_back(output_attrs[i].zp);
  }
  post_process((int8_t *)outputs[0].buf, (int8_t *)outputs[1].buf, (int8_t *)outputs[2].buf, height, width,
                box_conf_threshold, nms_threshold, pads, scale_w, scale_h, out_zps, out_scales, results,
                    label_name_txt_path);

  error_ret = rknn_outputs_release(ctx, io_num.n_output, outputs);
  cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);

  return 0;
}

int RkNPU::yolo_draw_results(cv::Mat frame_in, cv::Mat& frame_labbed,
                                      _detect_result_group_t *results)
{
  frame_in.copyTo(frame_labbed);
  char text[256];
  for (int i = 0; i < results->count; i++)
  {
    detect_result_t *det_result = &(results->results[i]);
    // if(compareStrings(det_result->name, "person") != 0){
    //   continue;
    // }
    
    sprintf(text, "%s %.1f%%", det_result->name, det_result->prop * 100);
    int x1 = det_result->box.left;
    int y1 = det_result->box.top;
    int x2 = det_result->box.right;
    int y2 = det_result->box.bottom;
    rectangle(frame_labbed, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 204, 102), 3);
    putText(frame_labbed, text, cv::Point(x1, y1 + 12), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255));
  }
  return 0;
}

int RkNPU::yolo_draw_results_match(cv::Mat frame_in, cv::Mat& frame_labbed,
                                      _detect_result_group_t *results, const char* target)
{
  frame_in.copyTo(frame_labbed);
  char text[256];
  for (int i = 0; i < results->count; i++)
  {
    detect_result_t *det_result = &(results->results[i]);
    if(compareStrings(det_result->name, target) != 0){
      continue;
    }
    
    sprintf(text, "%s %.1f%%", det_result->name, det_result->prop * 100);
    int x1 = det_result->box.left;
    int y1 = det_result->box.top;
    int x2 = det_result->box.right;
    int y2 = det_result->box.bottom;
    rectangle(frame_labbed, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 204, 102), 3);
    putText(frame_labbed, text, cv::Point(x1, y1 + 12), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255));
  }
  return 0;
}


std::string RkNPU::trim(const std::string& str) {
    std::string s = str;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

int RkNPU::compareStrings(const char* str1, const char* str2) {
    std::string trimmed1 = trim(std::string(str1));
    std::string trimmed2 = trim(std::string(str2));

    std::transform(trimmed1.begin(), trimmed1.end(), trimmed1.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    std::transform(trimmed2.begin(), trimmed2.end(), trimmed2.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (trimmed1 == trimmed2) {
        return 0;
    } else if (trimmed1 < trimmed2) {
        return -1;
    } else {
        return 1;
    }
}



int RkNPU::yolo_print_results(_detect_result_group_t *results)
{
  for (int i = 0; i < results->count; i++)
  {
    detect_result_t *det_result = &(results->results[i]);
    std::cout << det_result->name << std::endl;
    printf("%s @ (%d %d %d %d) %f\n", det_result->name, det_result->box.left, det_result->box.top,
           det_result->box.right, det_result->box.bottom, det_result->prop);
  }
  return 0;
}

RkNPU::RkNPU()
{

}

RkNPU::~RkNPU()
{
  /*释放模型*/
  deinitPostProcess();

  rknn_destroy(ctx);

  if (model_data)
  {
    free(model_data);
  }
}