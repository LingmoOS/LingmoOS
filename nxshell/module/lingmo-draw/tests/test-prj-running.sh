QTEST_FUNCTION_TIMEOUT=500000
export QT_QPA_PLATFORM='offscreen'
cd ../
[ -d build-ut ] && rm -fr build-ut || mkdir build-ut
[ -d build ] && rm -fr build 
mkdir -p build
cp tests/collection-coverage.sh build/
cd build/

cmake -S ../ -B ./ -OCEANEPINDRAW_TEST=ON

make -j8

lcov --directory ./src/CMakeFiles/lingmoDrawBase.dir/ --zerocounters

# run test
ASAN_OPTIONS="fast_unwind_on_malloc=1" ./tests/lingmo-draw-test

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
cp ./build/asan_lingmo-draw.log* ./build-ut/asan_lingmo-draw.log
cd build-ut
mv ./report ./html
cd html
mv ./index.html ./cov_lingmo-draw.html
mkdir ../report
cp ../../build/report/report_lingmo-draw.xml ../report/
chmod 751 cov_lingmo-draw.html

