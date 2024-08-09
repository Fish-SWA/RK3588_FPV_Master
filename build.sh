mkdir build
cd build
cmake ..
make -j4

if [ "$1" == "-calib" ]; then
    echo "calib task"
    ./Cam_calibrate
elif [ "$1" == "-deep_map" ]; then
    echo "deep map"
    ./deep_map
else
    echo "main task"
    ./Fpv_master
fi
# ./Fpv_master