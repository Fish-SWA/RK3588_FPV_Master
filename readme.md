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
- 完成NPU调用， 加入YOLO5
  - 参考[rknpu2](https://github.com/rockchip-linux/rknpu2)
  - 先针对yolo5特化，写成一个YoloNPU类，后面再考虑通用性
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
// 暂无

## 改动记录

### 6.14~6.16 by Fish
- 加入了OpenCV
- 在vision.cpp中的双目相机类BinoCamera中实现了乒乓球的识别
  - 但是基于色彩过滤，比较依赖环境光照

### 5.31 by Fish
- 新建项目
- 非常基本的串口通讯