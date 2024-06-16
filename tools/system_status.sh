# 用来查看GPU和NPU信息

# NPU
echo "NPU:"
cat /sys/kernel/debug/rknpu/load
# cat /sys/devices/platform/fdab0000.npu/devfreq/fdab0000.npu/load

# GPU
echo "GPU:"
cat /sys/devices/platform/fb000000.gpu/devfreq/fb000000.gpu/load