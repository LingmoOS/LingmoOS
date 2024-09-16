QTEST_FUNCTION_TIMEOUT=500000
export QT_QPA_PLATFORM='offscreen'
cd ../
[ -d build-ut ] && rm -fr build-ut || mkdir build-ut
[ -d build ] && rm -fr build 
mkdir -p build
cp tests/collection-coverage.sh build/
cd build/

cmake -S ../ -B ./ -DDEEPINDRAW_TEST=ON

make -j8

lcov --directory ./src/CMakeFiles/deepinDrawBase.dir/ --zerocounters

# run test
ASAN_OPTIONS="fast_unwind_on_malloc=1" ./tests/deepin-draw-test

#first to collection (unknowed fail may happen).
./collection-coverage.sh

sleep 5

#unknowed failed may happen,so we collect again.
./collection-coverage.sh

cd ../
mkdir -p ./build-ut
#拷贝覆盖率到指定的路径
cp -r ./build/coverageResult/report ./build-ut
#拷贝内存泄漏检测信息到指定的路径
cp ./build/asan_deepin-draw.log* ./build-ut/asan_deepin-draw.log
cd build-ut
mv ./report ./html
cd html
mv ./index.html ./cov_deepin-draw.html
mkdir ../report
cp ../../build/report/report_deepin-draw.xml ../report/
chmod 751 cov_deepin-draw.html

