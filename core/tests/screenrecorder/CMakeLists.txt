cmake_minimum_required(VERSION 3.22)
project(screenrecorder_encoder_test LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(LingmoSDK REQUIRED)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil libswscale)
pkg_check_modules(LibVA REQUIRED IMPORTED_TARGET libva)
pkg_check_modules(Libdrm REQUIRED IMPORTED_TARGET libdrm)
pkg_check_modules(Gbm REQUIRED IMPORTED_TARGET gbm)
set(RECORDER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../screenrecorder)
# The encoder, fed made-up frames and sound, and DMA-BUFs where a render node can
# allocate them; the files it writes are read back with FFmpeg.
add_executable(screenrecorder_encoder_test screenrecorder_encoder_test.cpp
    ${RECORDER_DIR}/encoder.cpp ${RECORDER_DIR}/image.cpp)
target_include_directories(screenrecorder_encoder_test PRIVATE ${RECORDER_DIR})
target_link_libraries(screenrecorder_encoder_test PRIVATE Lingmo::SDKBase GTest::gtest_main
    PkgConfig::FFmpeg PkgConfig::LibVA PkgConfig::Libdrm PkgConfig::Gbm)
enable_testing()
include(GoogleTest)
gtest_discover_tests(screenrecorder_encoder_test)
