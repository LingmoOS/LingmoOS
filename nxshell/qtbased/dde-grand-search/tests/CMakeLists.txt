# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置工程名字
project(test-dde-grand-search)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_DEBUG")

set(PROJECT_SOURCE_PATH "${CMAKE_SOURCE_DIR}/../src")
set(PROJECT_3RDPARTY_PATH "${CMAKE_SOURCE_DIR}/../3rdparty")
set(TESTS_3RDPARTY_PATH "${CMAKE_SOURCE_DIR}/3rdparty")

# 内存泄漏
if (NOT ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "mips"))
    ADD_DEFINITIONS(-DENABLE_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
endif()

# 用于测试覆盖率的编译条件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline -fno-access-control -O0 -fprofile-arcs -ftest-coverage -lgcov")

# UT依赖
find_package(GTest REQUIRED)
find_package(Qt5 COMPONENTS Test REQUIRED)

# 全局文件
FILE(GLOB GLOBAL_SRC "${PROJECT_SOURCE_PATH}/global/*.h" "${PROJECT_SOURCE_PATH}/global/*.cpp")

# 打桩工具
FILE(GLOB CPP_STUB_SRC "${TESTS_3RDPARTY_PATH}/cpp-stub/*.h"
    "${TESTS_3RDPARTY_PATH}/cpp-stub/*.hpp"
    "${TESTS_3RDPARTY_PATH}/stub-ext/*.h"
    "${TESTS_3RDPARTY_PATH}/stub-ext/*.cpp")

include_directories("${TESTS_3RDPARTY_PATH}/cpp-stub" "${TESTS_3RDPARTY_PATH}/stub-ext")
include_directories(${PROJECT_SOURCE_PATH})
include_directories("${CMAKE_SOURCE_DIR}/../3rdparty")

add_subdirectory(grand-search)
add_subdirectory(libgrand-search-daemon)
add_subdirectory(preview-plugin)
add_subdirectory(grand-search-dock-plugin)

