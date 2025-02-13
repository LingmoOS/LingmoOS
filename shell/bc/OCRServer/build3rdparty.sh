#! bash

JOBS=`cat /proc/cpuinfo| grep "processor"|  wc -l`
if [ $JOBS -gt 8 ]
then JOBS=8
elif [ $JOBS -eq 0 ]
then JOBS=1
fi

#build opencv
if [ ! -f "3rdparty/opencv-4.5.4/build/install/lib/libopencv_world.a" ]; then
cd 3rdparty/opencv-4.5.4
patch -p1 --batch -i ../opencv-mobile/opencv-4.5.4-no-zlib.patch
truncate -s 0 cmake/OpenCVFindLibsGrfmt.cmake
rm -rf modules/gapi
rm -rf modules/highgui
cp -r ../opencv-mobile/highgui modules/
patch -p1 --batch -i ../opencv-mobile/opencv-4.5.4-no-rtti.patch
rm -rf build && mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Release `cat ../../opencv-mobile/opencv4_cmake_options.txt` -DBUILD_opencv_world=ON ..
make -j$JOBS && make install
cd ../../..
fi

#build ncnn
if [ ! -f "3rdparty/ncnn/build/install/lib/libncnn.a" ]; then
cd 3rdparty/ncnn
rm -rf build && mkdir build && cd build
cmake -DNCNN_C_API=OFF -DNCNN_BUILD_BENCHMARK=OFF -DNCNN_BUILD_TOOLS=OFF -DNCNN_BUILD_EXAMPLES=OFF -DNCNN_MSA=OFF -DNCNN_MMI=ON ..
make -j$JOBS && make install
fi
