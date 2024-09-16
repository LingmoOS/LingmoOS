cmake_minimum_required(VERSION 3.1.0)

set(TEST_DAPPLOADER gts_dapploader)

# 指定 src 目录中生成的 lib 的名字和路径
add_definitions(-DLIB_NAME="${CMAKE_PROJECT_NAME}")
add_definitions(-DLIB_PATH="${CMAKE_BINARY_DIR}/src")

find_package(Qt5Test REQUIRED)

add_executable(${TEST_DAPPLOADER} gts_dapploader.cpp)

target_link_libraries(${TEST_DAPPLOADER}
    Qt5::Test
    ${DtkDeclarative_LIBRARIES}
    -lgtest
    -lpthread
    )

#------------------------------ 创建'make tests'指令---------------------------------------
include(GoogleTest)
enable_testing()

gtest_discover_tests(${TEST_DAPPLOADER} AUTO AUTO)

add_custom_target(tests ALL
    DEPENDS ${TEST_DAPPLOADER}
    )

add_custom_command(TARGET tests
    COMMAND echo " =================== TEST BEGIN ==================== "
    COMMAND ${CMAKE_BINARY_DIR}/tests/dapploader/${TEST_DAPPLOADER}
    COMMAND echo " =================== TEST END ==================== "
    )
##'make tests'命令依赖与我们的测试程序
add_dependencies(tests ${TEST_DAPPLOADER})

## 设置添加gocv相关信息的输出
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage -fno-inline")
