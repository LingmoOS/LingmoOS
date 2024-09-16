
#!/bin/bash
BUILD_DIR=build
REPORT_DIR=report
EXTRACT_ARGS="*/src/window/modules/*"
cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
qmake ..
make
cd ../tests/
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
qmake ../ CONFIG+=debug
export ASAN_OPTIONS=halt_on_error=0
TESTARGS="--gtest_output=xml:deepin_test_report_defender.xml"  make check -j$(nproc)
lcov -d ./ -c -o coverage_all.info
lcov --extract coverage_all.info $EXTRACT_ARGS --output-file coverage.info
#lcov --remove coverage_all.info "*/tests/*" "*/usr/include*" "*build/src*" --output-file coverage.info
mv asan.log* asan_deepin_defender.log 
cd ..
genhtml -o $REPORT_DIR $BUILD_DIR/coverage.info
#rm -rf $BUILD_DIR
#rm -rf ../$BUILD_DIR

