## 版本说明
- 用于电赛&嵌塞无人机的视觉， 在RK3588上运行
- 正在边学边写ing

## 项目结构
//待完善

## 环境要求
- Opencv 4.10
- 完整的OpenCL环境，配置方法见[此处](https://clehaxze.tw/gemlog/2023/06-17-setting-up-opencl-on-rk3588-using-libmali.gmi) 
- clash & vscode (非必须, 方便调试)

## TODO 
- 更换成Opencv4
- 移植一个线程池来最大化NPU利用率
- 写完串口通讯
  - 串口发 √
  - 串口收 ×
  - 模拟串口中断 ×
- 实现与无人机飞控的串口通讯
  - 通过发结构体解决？
  - [0xEF] -- data -- [0xEE]
- 写一个自动配置环境的脚本


## 开源引用
- yolov5模型的数据预处理&后处理的库来自[rknpu2](https://github.com/rockchip-linux/rknpu2), 位于library/rknpu/yolo5_process

## 改动记录

### 6.18 by Fish
- 实现了NPU调用，现在可以用RK3588的NPU跑YOLO了（喜
  - 参考RKNPU的API手册实现
  - 调用接口在rknpu_yolo.cpp的RkNPU类中，使用方法如下
    - 初始化一个RkNPU类
    - 通过RkNPU::model_path和RkNPU::label_name_txt_path设置rknn模型文件&label_list文件的路径
    - 使用RkNPU::rknn_model_init初始化RKNN对象
    - 使用RlNPU::rknn_img_inference执行推理，输入是Mat对象，输出是_detect_result_group_t结构体
    - 可以选用RlNPU::yolo_draw_results在图像中画出判定框， 以及用RkNPU::yolo_print_results答应出识别结果
    - 全过程都可以通过RlNPU::error_ret来判断是否发生错误（error_ret=0为正常， 其他会对应错误码）
- 非常好进度，但是48小时之后的考试大概或许是要寄了（悲


### 6.14~6.16 by Fish
- 加入了OpenCV
- 在vision.cpp中的双目相机类BinoCamera中实现了乒乓球的识别
  - 但是基于色彩过滤，比较依赖环境光照

### 5.31 by Fish
- 新建项目
- 非常基本的串口通讯